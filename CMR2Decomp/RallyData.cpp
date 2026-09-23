#include <string.h>
#include <windows.h>
#include "RallyData.h"
#include "RallyRoute.h"
#include "GameInfo.h"
#include "RallyTiming.h"
#include "main.h"
#include "Frontend.h"
#include "AIHelper.h"
#include "Font.h"
#include "Sprite.h"
#include "Graphics.h"

int FUN_00456c00(int index);

// GLOBAL: CMR2 0x0052f2a9
BYTE g_unk0x0052f2a9;

// GLOBAL: CMR2 0x0052f2a8
BYTE g_unk0x0052f2a8;
// Original 9 x 30 rally lookup table; the next data begins at 0x5166b4.
// GLOBAL: CMR2 0x0051627c
unsigned int g_unk0x0051627c[9][30] = {
    {
        0, 0, 0, 1, 1, 1, 1, 2, 2, 3,
        0, 0, 0, 1, 1, 1, 1, 2, 2, 3,
        0, 0, 1, 1, 1, 1, 2, 2, 2, 3
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 3,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 4
    },
    {
        0, 0, 1, 1, 2, 2, 6, 6, 7, 7,
        0, 1, 1, 2, 2, 2, 6, 7, 7, 8,
        1, 2, 2, 6, 7, 7, 7, 7, 8, 8
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 2,
        0, 0, 0, 0, 0, 1, 1, 1, 2, 3,
        0, 0, 0, 0, 0, 1, 1, 1, 2, 4
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 4,
        0, 0, 0, 0, 0, 1, 1, 2, 2, 4
    },
    {
        0, 0, 1, 1, 1, 1, 1, 2, 2, 3,
        1, 0, 1, 1, 1, 2, 2, 2, 3, 4,
        1, 0, 1, 1, 2, 2, 2, 3, 4, 5
    },
    {
        1, 1, 1, 2, 2, 2, 3, 3, 4, 4,
        1, 2, 2, 2, 2, 3, 3, 4, 4, 5,
        2, 2, 2, 2, 3, 3, 4, 4, 4, 5
    },
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    }
};
// GLOBAL: CMR2 0x0052f1a0
int g_unk0x0052f1a0[20];
// GLOBAL: CMR2 0x0052f1f0
int g_unk0x0052f1f0[20];
// GLOBAL: CMR2 0x0052f240
int g_unk0x0052f240[20];
// GLOBAL: CMR2 0x0052f294
BYTE g_unk0x0052f294[0x14];
// GLOBAL: CMR2 0x0052f2ac
unsigned int g_selectedRallyData = 0;

// GLOBAL: CMR2 0x0052f2b0
unsigned int g_unk0x0052f2b0;

// GLOBAL: CMR2 0x0052f2b4
unsigned int g_unk0x0052f2b4;
// GLOBAL: CMR2 0x0052f2b8
unsigned int g_unk0x0052f2b8;
// GLOBAL: CMR2 0x0052f2c4
unsigned int g_unk0x0052f2c4[6];
// GLOBAL: CMR2 0x0052f2dc
unsigned int g_unk0x0052f2dc[12];
// GLOBAL: CMR2 0x0052f30c
unsigned int g_unk0x0052f30c[48];

// FUNCTION: CMR2 0x00407820
unsigned int *RallyData_GetChampionshipState(void)
{
    return &g_unk0x0052f2b4;
}

// Decodes the two 5-bit driver indices of the current round; which table
// they come from depends on the championship stage (bits 3-5).
// FUNCTION: CMR2 0x00407830
void RallyData_GetRoundDrivers(unsigned int *pFirst, unsigned int *pSecond)
{
    switch ((g_unk0x0052f2b4 >> 3) & 7) {
    case 1:
        *pFirst = g_unk0x0052f30c[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] & 0x1f;
        *pSecond = (g_unk0x0052f30c[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] >> 5) & 0x1f;
        break;
    case 2:
        *pFirst = g_unk0x0052f2dc[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] & 0x1f;
        *pSecond = (g_unk0x0052f2dc[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] >> 5) & 0x1f;
        break;
    case 3:
        *pFirst = g_unk0x0052f2c4[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] & 0x1f;
        *pSecond = (g_unk0x0052f2c4[((g_unk0x0052f2b4 >> 0xc) & 0xf) * 3] >> 5) & 0x1f;
        break;
    case 4:
        *pFirst = g_unk0x0052f2b8 & 0x1f;
        *pSecond = (g_unk0x0052f2b8 >> 5) & 0x1f;
        break;
    }
}

// FUNCTION: CMR2 0x004070f0
int RallyData_FUN_004070f0(void)
{
    unsigned int drivers[2];

    if (CGameInfo::FUN_00405d80() == 4) {
        RallyData_GetChampionshipState();
        RallyData_GetRoundDrivers(&drivers[0], &drivers[1]);
        if (drivers[0] != drivers[1] && RallyData_FUN_00408500(drivers[0]) == -1 && RallyData_FUN_00408500(drivers[1]) == -1)
            return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x00411880
int RallyData_FUN_00411880(void)
{
    if (((BYTE)RallyDataState() > 1 && CGameInfo::FUN_00405da0() == 0 && CGameInfo::FUN_00405d80() != 4) || RallyData_FUN_004070f0() != 0)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x00406910
unsigned int RallyDataCountryIndex(void)
{
	return g_selectedRallyData & 0x1f;
}

// FUNCTION: CMR2 0x00406920
BYTE RallyData_FUN_00406920(void)
{
    return g_unk0x0052f2a8;
}

// FUNCTION: CMR2 0x00406930
unsigned char RallyDataStageIndex(void)
{
	return g_selectedRallyData >> 5 & 0x1f;
}

// FUNCTION: CMR2 0x004074f0
unsigned int RallyDataState(void)
{
	return g_selectedRallyData >> 0xe & 3;
}

// FUNCTION: CMR2 0x00407e50
unsigned int RallyData_GetFlag24(void)
{
	return g_selectedRallyData >> 0x18 & 1;
}

// FUNCTION: CMR2 0x00407e60
unsigned int RallyData_GetFlag25(void)
{
	return g_selectedRallyData >> 0x19 & 1;
}

// Index check compiled out of the release build; kept because every
// RallyData accessor still calls it.
// FUNCTION: CMR2 0x004083f0
void RallyData_ValidateIndex(int index)
{
}

// FUNCTION: CMR2 0x004239e0
void FUN_004239e0(int *p)
{
    if (p[1] == 3)
        RallyData_ValidateIndex((int)p);
}

// FUNCTION: CMR2 0x00406940
unsigned int RallyData_FUN_00406940(void)
{
	return g_selectedRallyData >> 10 & 3;
}

// FUNCTION: CMR2 0x00406950
unsigned int RallyData_FUN_00406950(void)
{
	return g_selectedRallyData >> 12 & 3;
}

// FUNCTION: CMR2 0x00406990
unsigned int RallyData_FUN_00406990(void)
{
	return g_selectedRallyData >> 16 & 0xf;
}

// FUNCTION: CMR2 0x004069b0
unsigned int RallyData_FUN_004069b0(void)
{
    return g_unk0x0052f2b4 & 7;
}

// FUNCTION: CMR2 0x004069a0
BYTE RallyData_FUN_004069a0(void)
{
	BYTE result = g_unk0x0052f2a9;
	result &= 0xf;
	return result;
}

// FUNCTION: CMR2 0x00408020
void RallyData_UpdateFlags(void)
{
	g_selectedRallyData &= ~0x2000000;
	if (RallyDataStageIndex() == 10)
		g_selectedRallyData |= 0x2000000;

	g_selectedRallyData &= ~0x10000000;
	if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 3)
		g_selectedRallyData |= 0x10000000;

	g_selectedRallyData &= ~0x8000000;
	if ((g_selectedRallyData & 0x2000000) && !(g_selectedRallyData & 0x10000000))
		g_selectedRallyData |= 0x8000000;

	g_selectedRallyData &= ~0x1000000;
	if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 || CGameInfo::FUN_00405d80() == 7 ||
	    CGameInfo::FUN_00405d80() == 11 || CGameInfo::FUN_00405d80() == 12)
		g_selectedRallyData |= 0x1000000;

	g_selectedRallyData &= ~0x4000000;
	if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6)
		g_selectedRallyData |= 0x4000000;

	g_selectedRallyData &= ~0x80000000;
	if (CGameInfo::FUN_00405d80() == 11 || CGameInfo::FUN_00405d80() == 12)
		g_selectedRallyData |= 0x80000000;

	g_selectedRallyData &= ~0x400000;
	if (g_selectedRallyData & 0xc000000)
		g_selectedRallyData |= 0x400000;

	g_selectedRallyData &= ~0x20000000;
	if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1 || CGameInfo::FUN_00405d80() == 2 ||
	    CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 || CGameInfo::FUN_00405d80() == 10)
		g_selectedRallyData |= 0x20000000;

	if (!(g_selectedRallyData & 0x1000000))
		g_selectedRallyData |= 0x40000000;

	g_selectedRallyData &= ~0x200000;
	if ((g_selectedRallyData & 0x20000000) || CGameInfo::FUN_00405d80() == 4)
		g_selectedRallyData |= 0x200000;

	g_selectedRallyData &= ~0x800000;
	if (g_selectedRallyData & 0x31000000)
		g_selectedRallyData |= 0x800000;
}

// FUNCTION: CMR2 0x0040d5c0
void RallyData_ResetSelection(void)
{
	g_selectedRallyData = (g_selectedRallyData & 0xfff3c000) | 0x30000;
	g_unk0x0052f2b4 = (g_unk0x0052f2b4 & 0xfff802cc) | 0x802cc;
	g_unk0x0052f2b0 &= 0xfffffff8;
}

// FUNCTION: CMR2 0x0040d600
void RallyData_FUN_0040d600(BYTE param1)
{
	g_selectedRallyData = ((param1 & 3) << 10) | (g_selectedRallyData & 0xfffff3ffU);
}

// FUNCTION: CMR2 0x0040d620
void RallyData_FUN_0040d620(BYTE param1)
{
	g_selectedRallyData = ((param1 & 0xf) << 16) | (g_selectedRallyData & 0xfff0ffffU);
}

// FUNCTION: CMR2 0x0040d640
void RallyData_FUN_0040d640(BYTE param1)
{
	g_unk0x0052f2a9 = g_unk0x0052f2a9 ^ (g_unk0x0052f2a9 ^ param1) & 0xf;
}

// FUNCTION: CMR2 0x0040d660
void RallyData_FUN_0040d660(BYTE param1)
{
	g_unk0x0052f2b4 = (param1 & 7) | (g_unk0x0052f2b4 & 0xfffffff8U);
}

// FUNCTION: CMR2 0x0040d680
void RallyData_FUN_0040d680(BYTE param1)
{
	g_unk0x0052f2b4 = ((param1 & 7) << 6) | (g_unk0x0052f2b4 & 0xfffffe3fU);
}

// FUNCTION: CMR2 0x0040d6a0
void RallyData_FUN_0040d6a0(BYTE param1)
{
	g_unk0x0052f2b4 = ((param1 & 0xf) << 16) | (g_unk0x0052f2b4 & 0xfff0ffffU);
}

// GLOBAL: CMR2 0x0051682c
BYTE g_unk0x0051682c[132];

// FUNCTION: CMR2 0x00406890
char *RallyData_FUN_00406890(void)
{
    unsigned char country;

    country = (unsigned char)RallyDataCountryIndex();
    return (char *)g_unk0x0051682c + country * 7;
}

// FUNCTION: CMR2 0x004068b0
void RallyData_FUN_004068b0(BYTE param1)
{
    g_selectedRallyData = (param1 & 0x1f) | (g_selectedRallyData & 0xffffffe0U);
}

// FUNCTION: CMR2 0x00406960
void RallyData_FUN_00406960(BYTE param1)
{
    g_selectedRallyData = (g_selectedRallyData & 0xffffcfffU) | ((param1 & 3) << 12);
    RallyData_UpdateFlags();
}

// FUNCTION: CMR2 0x004068d0
void RallyData_FUN_004068d0(BYTE param1)
{
    g_unk0x0052f2a8 = param1;
}

// FUNCTION: CMR2 0x004068e0
void RallyData_FUN_004068e0(BYTE param1)
{
    g_selectedRallyData = (g_selectedRallyData & 0xfffffc1fU) | ((param1 & 0x1f) << 5);
    RallyData_UpdateFlags();
}

// GLOBAL: CMR2 0x005200e8
BYTE g_unk0x005200e8[256];

// FUNCTION: CMR2 0x00494a40
char *RallyData_FUN_00494a40(void)
{
    return (char *)g_unk0x005200e8 +
        ((unsigned char)RallyData_FUN_00406940() * 3 +
         (unsigned char)RallyData_FUN_00406950()) * 7;
}

// GLOBAL: CMR2 0x0052ea68
BYTE g_unk0x0052ea68[11];
// GLOBAL: CMR2 0x0052ea74
BYTE g_unk0x0052ea74[0x24];
// GLOBAL: CMR2 0x0052ea98
int g_unk0x0052ea98[8];
// GLOBAL: CMR2 0x0052eab8
int g_unk0x0052eab8;

// FUNCTION: CMR2 0x00408300
BOOL RallyData_FUN_00408300(void)
{
    unsigned int i;

    for (i = 0; i < 0xb; i++) {
        if ((g_unk0x0052ea68[i] & 1) == 0)
            continue;
        if ((g_unk0x0052ea68[i] & 2) != 0 && !CGameInfo::FUN_00406410(0xd))
            continue;
        if ((g_unk0x0052ea68[i] & 4) == 0)
            break;
    }
    return ((g_selectedRallyData >> 5) & 0x1f) == i;
}

// FUNCTION: CMR2 0x00408390
void RallyData_FUN_00408390(void)
{
    unsigned int i;

    for (i = 0; i < 0xb; i++) {
        if ((g_unk0x0052ea68[i] & 1) == 0)
            continue;
        if ((g_unk0x0052ea68[i] & 2) != 0 && !CGameInfo::FUN_00406410(0xd))
            continue;
        if ((g_unk0x0052ea68[i] & 4) == 0) {
            RallyData_FUN_004068e0((BYTE)i);
            return;
        }
    }
}

// FUNCTION: CMR2 0x00408340
BOOL RallyData_FUN_00408340(void)
{
    unsigned int i;
    BYTE *p;

    i = ((g_selectedRallyData >> 5) & 0x1f) + 1;
    if (i >= 0xb)
        return TRUE;
    p = &g_unk0x0052ea68[i];
    do {
        if ((*p & 1) != 0 &&
            ((*p & 2) == 0 || CGameInfo::FUN_00406410(0xd)) &&
            (*p & 4) == 0)
            return FALSE;
        p++;
    } while (p < &g_unk0x0052ea68[0xb]);
    return TRUE;
}

// GLOBAL: CMR2 0x0052f3e8
BYTE g_unk0x0052f3e8[0x2000];
// GLOBAL: CMR2 0x00531350
BYTE g_unk0x00531350[0x1000];
extern BYTE g_unk0x0052fa5c[];

// FUNCTION: CMR2 0x004ebfd0
void FUN_004ebfd0(int index)
{
    unsigned int value = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    value = (value & 0xffffdfffU) | 0x3c0000;
    *(unsigned int *)(g_unk0x00531350 + index * 0x30) = value;
}

// FUNCTION: CMR2 0x004ec000
void FUN_004ec000(void)
{
    int i = 0;
    do {
        FUN_004ebfd0(i);
        i++;
    } while (i < 16);
}

// FUNCTION: CMR2 0x004075b0
int *RallyData_FUN_004075b0(int index)
{
    return &g_unk0x0052f1f0[index];
}

// FUNCTION: CMR2 0x004075c0
int *RallyData_FUN_004075c0(int index)
{
    return &g_unk0x0052f240[index];
}

// FUNCTION: CMR2 0x004075d0
BYTE *RallyData_FUN_004075d0(int index)
{
    return &g_unk0x0052f294[index];
}

// FUNCTION: CMR2 0x004075e0
int *RallyData_FUN_004075e0(int index)
{
    return &g_unk0x0052f1a0[index];
}

// FUNCTION: CMR2 0x004075f0
unsigned int *RallyData_FUN_004075f0(void)
{
    return g_unk0x0051627c[g_selectedRallyData & 0x1f];
}

// GLOBAL: CMR2 0x0052f0e0
BYTE g_unk0x0052f0e0[4][7];

// FUNCTION: CMR2 0x00407630
BYTE *RallyData_FUN_00407630(int index)
{
    return g_unk0x0052f0e0[index];
}

// GLOBAL: CMR2 0x00516188
BYTE g_stageScoreScale[192] = {
    0x06, 0x09, 0x0c, 0x0c, 0x0e, 0x10, 0x10, 0x11, 0x08, 0x08, 0x0c, 0x0d, 0x0f, 0x10, 0x15, 0x15,
    0x08, 0x0a, 0x0c, 0x0d, 0x0f, 0x0f, 0x00, 0x00, 0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13,
    0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x12, 0x12, 0x07, 0x07, 0x09, 0x0b, 0x11, 0x11, 0x00, 0x00,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x11, 0x11, 0x08, 0x09, 0x0e, 0x0f, 0x0f, 0x0f, 0x13, 0x13,
    0x06, 0x09, 0x0f, 0x0f, 0x12, 0x12, 0x00, 0x00, 0x09, 0x09, 0x0d, 0x0d, 0x0e, 0x0f, 0x13, 0x13,
    0x0b, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x15, 0x15, 0x08, 0x09, 0x0c, 0x0d, 0x12, 0x12, 0x00, 0x00,
    0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x10, 0x11, 0x13,
    0x06, 0x09, 0x0d, 0x0d, 0x09, 0x09, 0x00, 0x00, 0x06, 0x09, 0x0c, 0x0c, 0x0e, 0x10, 0x10, 0x11,
    0x08, 0x08, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13, 0x08, 0x0a, 0x15, 0x15, 0x0a, 0x0a, 0x00, 0x00,
    0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x12, 0x12,
    0x07, 0x07, 0x09, 0x0b, 0x10, 0x10, 0x00, 0x00, 0x08, 0x0a, 0x0b, 0x0d, 0x10, 0x10, 0x12, 0x13,
    0x06, 0x08, 0x0c, 0x0c, 0x0d, 0x0d, 0x15, 0x15, 0x06, 0x07, 0x0c, 0x0d, 0x12, 0x12, 0x00, 0x00,
};

// FUNCTION: CMR2 0x004077d0
int RallyData_FUN_004077d0(int row, int column, int variant)
{
    return (unsigned int)g_stageScoreScale[(column + row * 12) * 2 + variant] * 100;
}

// FUNCTION: CMR2 0x00408860
BYTE *RallyData_FUN_00408860(int index)
{
    unsigned int category;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052f3e8 + category * 0x650 + 0x620;
    return NULL;
}

// FUNCTION: CMR2 0x00408c70
BYTE *RallyData_FUN_00408c70(int index)
{
    unsigned int category;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052fa5c + category * 0x650 + 0xc;
    return NULL;
}

// FUNCTION: CMR2 0x00408d60
BYTE *RallyData_FUN_00408d60(int index)
{
    return g_unk0x00531350 + index * 0x30;
}

// FUNCTION: CMR2 0x0040df30
void RallyData_FUN_0040df30(void)
{
    BYTE zero = 0;
    int i = 0;
    do {
        BYTE *p = RallyData_FUN_00407630(i);
        i++;
        p[4] = zero;
        p[5] = zero;
        p[1] = zero;
        p[3] = zero;
        p[2] = zero;
        p[0] = zero;
        p[6] = zero;
    } while (i < 4);
}

// FUNCTION: CMR2 0x004086b0
int RallyData_FUN_004086b0(unsigned int index)
{
    if (CGameInfo::FUN_00405d80() == 4)
        return *(BYTE *)((int *)g_unk0x0052f3e8 + (index & 0xff) * 49);
    return *(int *)((char *)g_unk0x00531350 + (index & 0xff) * 48) & 0x3f;
}

// FUNCTION: CMR2 0x004074a0
bool RallyData_FUN_004074a0(void)
{
    unsigned int v;

    if (CGameInfo::FUN_00405d80() == 5) {
        v = g_selectedRallyData;
        v ^= (((v & 0xfffff000) + 0x1000) ^ v) & 0x3000;
        g_selectedRallyData = v;
        if ((v & 0x3000) >= 0x3000)
            return false;
        if ((v & 0xc00) == 0x800)
            return false;
        return true;
    }
    return false;
}

// GLOBAL: CMR2 0x005167e0
int g_unk0x005167e0[16];

// FUNCTION: CMR2 0x00407f20
int *RallyData_FUN_00407f20(int index)
{
    int i;

    if (CGameInfo::FUN_00405d80() == 5 ||
        CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7) {
        i = index;
        if (i < 0 || i > 5)
            i = 0;
        i = CAIHelper::FUN_00407f80(i);
    } else {
        i = index;
        if (i < 0 || i > 0xf)
            i = 0;
    }
    return &g_unk0x005167e0[i];
}

// FUNCTION: CMR2 0x0040fe50
char *RallyData_FUN_0040fe50(void)
{
    int table[8];

    table[0] = 6;
    table[1] = 3;
    table[2] = 1;
    table[3] = 4;
    table[4] = 0;
    table[5] = 2;
    table[6] = 5;
    table[7] = 7;
    return CFrontend::GetTextString(table[(RallyData_FUN_00406940() & 0xff) * 3 +
                                          (RallyData_FUN_00406950() & 0xff)]);
}

// GLOBAL: CMR2 0x0058c938
BYTE *g_unk0x0058c938;
// GLOBAL: CMR2 0x0058c94c
BYTE *g_unk0x0058c94c;
// GLOBAL: CMR2 0x0058c958
int *g_unk0x0058c958;
// GLOBAL: CMR2 0x0058ca6c
unsigned int g_unk0x0058ca6c;

// Copies the 12-byte vector and, when the entry is not already flagged,
// raises the destination's Y component by 0x3e80000.
// TODO: CMR2 0x00471cc0 (implemented, match 49%)
void RallyData_FUN_00471cc0(int *pDest, void **pParam1)
{
    int *pSrc;
    int **pp;
    unsigned int index;

    pp = (int **)*pParam1;
    pSrc = *pp;
    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = pSrc[2];
    index = (unsigned int)((BYTE *)pParam1[0] - g_unk0x0058c94c) / 8;
    if (index >= g_unk0x0058ca6c)
        return;
    if (RallyDataState() > 1) {
        if (g_unk0x0058c958[index] != 0)
            return;
        pDest[1] = pDest[1] + 0x3e80000;
    } else {
        if ((g_unk0x0058c938[index] & 1) != 0)
            return;
        pDest[1] += 0x3e80000;
    }
}

// GLOBAL: CMR2 0x0052fa5c
BYTE g_unk0x0052fa5c[0x10 * 0x650];

// Bumps the 0x7f80-masked field of the entry selected by each 0x30-byte record.
// TODO: CMR2 0x004ec1a0 (implemented, match 62%)
void RallyData_FUN_004ec1a0(void)
{
    unsigned int *pEntry;
    unsigned int i;

    i = 0;
    if (CGameInfo::FUN_00405d70() == 0)
        return;
    do {
        pEntry = (unsigned int *)(g_unk0x0052fa5c +
                 (((*(unsigned int *)(g_unk0x00531350 + i * 0x30) >> 0x12) & 0xf) * 0x650));
        if ((*pEntry & 0x7f80) != 0x7f80)
            *pEntry = (*pEntry & 0xffffff80) |
                      (((*pEntry & 0x7f80) + 0x80) & 0x7f80);
        i++;
    } while (i < CGameInfo::FUN_00405d70());
}

// GLOBAL: CMR2 0x00520128
BYTE g_unk0x00520128[0x100];


// GLOBAL: CMR2 0x0082c698
int g_unk0x0082c698;
// GLOBAL: CMR2 0x0082c69c
int g_unk0x0082c69c;
// GLOBAL: CMR2 0x0082c6a0
int g_unk0x0082c6a0;
// GLOBAL: CMR2 0x0082c6a4
int g_unk0x0082c6a4;
// GLOBAL: CMR2 0x0082c6a8
int g_unk0x0082c6a8;
// GLOBAL: CMR2 0x0082c6ac
int g_unk0x0082c6ac;
// GLOBAL: CMR2 0x0082c6bc
int g_unk0x0082c6bc;

// FUNCTION: CMR2 0x00503e00
void FUN_00503e00(void)
{
    if ((unsigned char)RallyDataCountryIndex() == 3) {
        g_unk0x0082c698 = 0;
        g_unk0x0082c6bc = 6;
        g_unk0x0082c69c = 1;
        g_unk0x0082c6a0 = 2;
        g_unk0x0082c6a4 = 6;
        g_unk0x0082c6a8 = 7;
        g_unk0x0082c6ac = 8;
    } else {
        g_unk0x0082c6bc = 6;
        g_unk0x0082c698 = 0;
        g_unk0x0082c69c = 1;
        g_unk0x0082c6a0 = 2;
        g_unk0x0082c6a4 = 3;
        g_unk0x0082c6a8 = 4;
        g_unk0x0082c6ac = 5;
    }
}

// Copies the per-driver stage times into the 0x30-byte records, first for the
// used drivers (in reverse) and then for the unused ones.
// TODO: CMR2 0x00408d80 (implemented, match 56%)
void RallyData_FUN_00408d80(void)
{
    unsigned int *pRec;
    int count;
    int limit;
    int i;

    count = CGameInfo::FUN_00405d70() & 0xff;
    for (i = 0; i < count; i++) {
        pRec = (unsigned int *)(g_unk0x00531350 + i * 0x30);
        *pRec = (*pRec & 0xffffe03f) |
                ((RallyTiming_GetStageTimeSeconds(0xf - i) & 0x7f) << 6);
        *(int *)(g_unk0x00531350 + i * 0x30 + 4) = RallyTiming_FUN_0040d3d0(0xf - i);
    }
    limit = 0x10 - (CGameInfo::FUN_00405d70() & 0xff);
    for (i = 0; i < limit; i++) {
        pRec = (unsigned int *)(g_unk0x00531350 +
                                ((CGameInfo::FUN_00405d70() & 0xff) + i) * 0x30);
        *pRec = (*pRec & 0xffffe03f) |
                ((RallyTiming_GetStageTimeSeconds(i) & 0x7f) << 6);
        *(int *)(g_unk0x00531350 +
                 ((CGameInfo::FUN_00405d70() & 0xff) + i) * 0x30 + 4) =
            RallyTiming_FUN_0040d3d0(i);
    }
}

void RallyData_FUN_00420820(void);

// Per car progress along the route, 0x18 bytes
struct RaceRecord {
    int field_0x0;
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int field_0x10;
    short field_0x14;
    short field_0x16;
};

// GLOBAL: CMR2 0x00538a78
int g_unk0x00538a78;
// GLOBAL: CMR2 0x00538a7c
int g_unk0x00538a7c;
// GLOBAL: CMR2 0x00538a80
int g_unk0x00538a80;
// GLOBAL: CMR2 0x00538a84
int g_unk0x00538a84;
// GLOBAL: CMR2 0x00538a88
int g_unk0x00538a88;
// GLOBAL: CMR2 0x00538a94
int g_unk0x00538a94;
// GLOBAL: CMR2 0x00538a98
int g_unk0x00538a98;
// GLOBAL: CMR2 0x00538aa8
RaceRecord g_raceRecords[8];
// GLOBAL: CMR2 0x00538c8c
int g_unk0x00538c8c;
// GLOBAL: CMR2 0x00538c90
int g_unk0x00538c90;

// TODO: CMR2 0x004207f0 (implemented, match 45%)
void RallyData_FUN_004207f0(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        g_raceRecords[i].field_0x8 = 0;
        g_raceRecords[i].field_0x0 = g_raceRecords[i].field_0x4;
        g_raceRecords[i].field_0x14 = 0;
        g_raceRecords[i].field_0xc = 0;
    }
    RallyData_FUN_00420820();
}

// FUNCTION: CMR2 0x00420820
void RallyData_FUN_00420820(void)
{
    g_unk0x00538a80 = 0;
    g_unk0x00538a98 = 0;
    g_unk0x00538a78 = 0;
    g_unk0x00538c8c = 0;
    g_unk0x00538a7c = 0;
    g_unk0x00538c90 = 0;
}

// FUNCTION: CMR2 0x00421370
int RallyData_FUN_00421370(BYTE *p)
{
    if (g_unk0x00538a84 == 0)
        return 0;
    return g_raceRecords[(signed char)p[0xb1a]].field_0x0;
}

// FUNCTION: CMR2 0x004213a0
short RallyData_FUN_004213a0(BYTE *p)
{
    if (g_unk0x00538a84 == 0)
        return 0;
    return g_raceRecords[(signed char)p[0xb1a]].field_0x14;
}

// FUNCTION: CMR2 0x00421420
int RallyData_FUN_00421420(void)
{
    return g_unk0x00538a84;
}

// FUNCTION: CMR2 0x00421430
int RallyData_FUN_00421430(void)
{
    return g_unk0x00538a88;
}

// FUNCTION: CMR2 0x00421440
BYTE *RallyData_FUN_00421440(int index)
{
    if (g_unk0x00538a84 == 0)
        return NULL;
    return g_routeNodes + index * 0x2c;
}

// FUNCTION: CMR2 0x00421500
int RallyData_FUN_00421500(void)
{
    return g_unk0x00538a94;
}

// FUNCTION: CMR2 0x00421510
RaceRecord *RallyData_FUN_00421510(int index)
{
    return &g_raceRecords[index];
}

// FUNCTION: CMR2 0x00421530
void RallyData_FUN_00421530(int index, int *pOut)
{
    int x = *(int *)(g_routeNodes + index * 0x2c);
    pOut[1] = 0;
    pOut[0] = x;
    pOut[2] = *(int *)(g_routeNodes + index * 0x2c + 8);
}
// GLOBAL: CMR2 0x00538c94
int g_unk0x00538c94;

// FUNCTION: CMR2 0x004209d0
int RallyData_FUN_004209d0(BYTE *p)
{
    return g_raceRecords[(signed char)p[0xb1a]].field_0x10;
}

// Scales the per-record value at p[0xb1a] to a 0..0x10000 ratio.
// TODO: CMR2 0x00421470 (implemented, match 63%)
int RallyData_FUN_00421470(BYTE *p)
{
    int value;
    int result;

    if (g_unk0x00538a84 == 0)
        return 0;
    value = g_raceRecords[(signed char)p[0xb1a]].field_0x8;
    if (g_unk0x00538a94 != 0) {
        if (value >= (RallyData_FUN_00406990() & 0xff) * g_unk0x00538a84)
            return 0x10000;
    } else {
        if (value >= g_unk0x00538a84)
            return 0x10000;
    }
    result = (value << 16) / (g_unk0x00538c94 >> 16);
    if (result < 0)
        return 0;
    if (result > 0x10000)
        return 0x10000;
    return result;
}

// GLOBAL: CMR2 0x0052f3e0
BYTE g_unk0x0052f3e0[8 * 196];
// Per-category records of 0x650 bytes (runs up to the 0x531350 table).
// GLOBAL: CMR2 0x0052fa18
BYTE g_unk0x0052fa18[0x1938];

// Record of the selected entry: the 196-byte entry in championship mode
// (mode 4), otherwise the 0x650-byte category record (NULL when the
// category nibble is 15).
// FUNCTION: CMR2 0x00408400
void *RallyData_GetRecord(unsigned int index)
{
    unsigned int category;

    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x0052f3e0 + (index & 0xff) * 196;
    RallyData_ValidateIndex(index & 0xff);
    category = (*(unsigned int *)(g_unk0x00531350 + (index & 0xff) * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052fa18 + category * 0x650;
    return NULL;
}

// Returns the 4-bit category of the record, or -1 when it is not usable.
// FUNCTION: CMR2 0x00408500
char RallyData_FUN_00408500(unsigned int param1)
{
    unsigned int index;

    if (CGameInfo::FUN_00405d80() == 4) {
        index = param1 & 0xff;
        if (strcmp((char *)(g_unk0x0052f3e0 + index * 196), CMain::m_logFileBlankLine) != 0 && (BYTE)param1 < 8)
            return -1;
        return (char)((*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0xe) & 0xf);
    }
    if ((*(unsigned int *)(g_unk0x00531350 + (param1 & 0xff) * 0x30) & 0x2000) != 0)
        return -1;
    return (char)((*(unsigned int *)(g_unk0x00531350 + (param1 & 0xff) * 0x30) >> 0xe) & 0xf);
}

// Returns bit 26 of the selected rally data (a per-rally flag).
// FUNCTION: CMR2 0x00407e70
unsigned int RallyData_FUN_00407e70(void)
{
    return (g_selectedRallyData >> 26) & 1;
}

// Returns the 0x650-byte record of the id, or NULL when its 0x3c0000 field is
// already full. The record base is 0x30 bytes into the 0x52fa5c table.
// FUNCTION: CMR2 0x00408cb0
BYTE *RallyData_FUN_00408cb0(int index)
{
    unsigned int value;

    RallyData_ValidateIndex(index);
    value = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    if ((value & 0x3c0000) != 0x3c0000)
        return g_unk0x0052fa5c + ((value >> 0x12) & 0xf) * 0x650 + 0x30;
    return NULL;
}

// FUNCTION: CMR2 0x00407e90
unsigned int RallyData_FUN_00407e90(void)
{
    return (g_selectedRallyData >> 27) & 1;
}

// FUNCTION: CMR2 0x00407ea0
unsigned int RallyData_FUN_00407ea0(void)
{
    return (g_selectedRallyData >> 28) & 1;
}

// Neutral type for now: nothing implemented reads it yet.
// GLOBAL: CMR2 0x00536be0
int g_unk0x00536be0;
// GLOBAL: CMR2 0x00536be4
int g_unk0x00536be4;

// FUNCTION: CMR2 0x0040eeb0
int RallyData_FUN_0040eeb0(void)
{
    return g_unk0x00536be4;
}

// FUNCTION: CMR2 0x00411060
int RallyData_FUN_00411060(void)
{
    return g_unk0x00536be0;
}

// FUNCTION: CMR2 0x004082e0
unsigned int RallyData_FUN_004082e0(void)
{
    return (g_unk0x0052f2b0 >> 0xb) & 1;
}

// FUNCTION: CMR2 0x004082b0
unsigned int RallyData_FUN_004082b0(void)
{
    return g_unk0x0052f2b0 & 7;
}

// Table of 0x148-byte records; count derived from the next known global
// (0x52f2a9), so it may cover further undeclared values.
struct Unk0x0052ebc0 {
    BYTE field_0x0[0x148];
};

// GLOBAL: CMR2 0x0052ebc0
Unk0x0052ebc0 g_unk0x0052ebc0[5];

// FUNCTION: CMR2 0x00407610
Unk0x0052ebc0 *RallyData_FUN_00407610(int index)
{
    return &g_unk0x0052ebc0[index];
}

// GLOBAL: CMR2 0x00516cd0
BYTE g_itemColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x00536bd8
short g_itemRect[4];

// Draws one item of a horizontal list and, unless it is the last one, the thin
// separator after it; returns the x the next item starts at.
// FUNCTION: CMR2 0x0040fd30
int RallyData_DrawListItem(int x, int y, char *pText, char last, BYTE alpha)
{
    BYTE colour[4];
    int width;

    colour[0] = g_itemColour[0];
    colour[1] = g_itemColour[1];
    colour[2] = g_itemColour[2];
    colour[3] = alpha;
    Font_DrawText(2, pText, x, y, (int *)colour, 0x11);
    if (last == 0) {
        width = Font_GetTextWidth(2, (BYTE *)pText);
        width = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
        g_itemRect[0] = (short)width;
        g_itemRect[2] = 2;
        g_itemRect[1] = (short)((int)(g_pGraphics->resY * 200) / 0x1e0);
        g_itemRect[3] = (short)((int)(g_pGraphics->resY * 0x3c) / 0x1e0);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_itemRect, colour, 1);
        return width + (int)(g_pGraphics->resX * 10) / 0x280;
    }
    return x;
}

// GLOBAL: CMR2 0x0052f0fc
int g_unk0x0052f0fc;
// GLOBAL: CMR2 0x0052f290
int g_unk0x0052f290;

// FUNCTION: CMR2 0x0040df60
void RallyData_FUN_0040df60(int param1, int param2)
{
    g_unk0x0052f290 = param1;
    g_unk0x0052f0fc = param2;
}

unsigned int RallyData_FUN_004070e0(void);

// FUNCTION: CMR2 0x004070c0
void RallyData_FUN_004070c0(void)
{
    RallyData_FUN_004068b0(RallyData_FUN_004070e0() & 0xff);
    RallyData_FUN_004068e0(10);
}

// FUNCTION: CMR2 0x004070e0
unsigned int RallyData_FUN_004070e0(void)
{
    return g_unk0x0052f2b4 >> 16 & 0xf;
}

// FUNCTION: CMR2 0x00407500
void RallyData_FUN_00407500(BYTE param1)
{
    g_selectedRallyData = ((param1 & 3) << 14) | (g_selectedRallyData & 0xffff3fffU);
}

// FUNCTION: CMR2 0x00407800
void RallyData_FUN_00407800(unsigned int param1)
{
    g_selectedRallyData = ((param1 & 1) << 20) | (g_selectedRallyData & 0xffefffffU);
}

// FUNCTION: CMR2 0x00407e20
unsigned int RallyData_GetFlag21(void)
{
    return g_selectedRallyData >> 21 & 1;
}

// FUNCTION: CMR2 0x00407e30
unsigned int RallyData_GetFlag22(void)
{
    return g_selectedRallyData >> 22 & 1;
}

// FUNCTION: CMR2 0x00407e40
unsigned int RallyData_GetFlag23(void)
{
    return g_selectedRallyData >> 23 & 1;
}

// FUNCTION: CMR2 0x00407e80
unsigned int RallyData_GetFlag31(void)
{
    return g_selectedRallyData >> 31;
}

// FUNCTION: CMR2 0x00407eb0
unsigned int RallyData_GetFlag30(void)
{
    return g_selectedRallyData >> 30 & 1;
}

// FUNCTION: CMR2 0x00408290
void RallyData_FUN_00408290(void)
{
    memset(g_unk0x0052ebc0, 0, 4 * sizeof(Unk0x0052ebc0));
}

// FUNCTION: CMR2 0x004082c0
unsigned int RallyData_FUN_004082c0(void)
{
    return g_unk0x0052f2b0 >> 7 & 0xf;
}

// FUNCTION: CMR2 0x004082d0
unsigned int RallyData_FUN_004082d0(void)
{
    return g_unk0x0052f2b0 >> 3 & 0xf;
}

// FUNCTION: CMR2 0x004082f0
BYTE *RallyData_FUN_004082f0(void)
{
    return g_unk0x0052ea68;
}

// FUNCTION: CMR2 0x004083d0
Unk0x0052ebc0 *RallyData_FUN_004083d0(void)
{
    return g_unk0x0052ebc0;
}

// FUNCTION: CMR2 0x004084c0
void RallyData_FUN_004084c0(BYTE index, BYTE param2)
{
    *(unsigned int *)&g_unk0x00531350[index * 0x30] =
        ((param2 & 0xf) << 14) | (*(unsigned int *)&g_unk0x00531350[index * 0x30] & 0xfffc0000U);
    *(unsigned int *)&g_unk0x00531350[index * 0x30 + 4] = 0;
}

// FUNCTION: CMR2 0x0040df80
void RallyData_FUN_0040df80(int index, int value)
{
    g_unk0x0052ea98[index] = value;
}

// FUNCTION: CMR2 0x0040e330
void RallyData_FUN_0040e330(char param1)
{
    if (param1 != 0) {
        g_unk0x0052f2b0 |= 0x800;
        return;
    }
    g_unk0x0052f2b0 &= ~0x800;
}

// FUNCTION: CMR2 0x0040e360
void RallyData_FUN_0040e360(unsigned int param1)
{
    g_unk0x0052f2b0 = g_unk0x0052f2b0 ^ (g_unk0x0052f2b0 ^ param1) & 7;
}

// FUNCTION: CMR2 0x0040e380
void RallyData_FUN_0040e380(unsigned int param1)
{
    g_unk0x0052f2b0 = ((param1 & 0xf) << 7) | (g_unk0x0052f2b0 & 0xfffff87fU);
}

// FUNCTION: CMR2 0x0040e3a0
void RallyData_FUN_0040e3a0(unsigned int param1)
{
    g_unk0x0052f2b0 = ((param1 & 0xf) << 3) | (g_unk0x0052f2b0 & 0xffffff87U);
}

// GLOBAL: CMR2 0x0051681c
BYTE g_unk0x0051681c[15] = { 0x00, 0x09, 0x0a, 0x08, 0x04, 0x04, 0x09, 0x00,
                             0x0a, 0x0b, 0x08, 0x0b, 0x04, 0x00, 0x0a };

// Row/slot of the driver select screen for one list index: the rally modes
// (5..7) only have six slots and ask the AI helper for the row, everything
// else uses a 15-entry lookup table.
// FUNCTION: CMR2 0x00407fc0
BYTE FUN_00407fc0(int param1)
{
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7) {
        if (param1 >= 0 && param1 < 6)
            return FUN_00456c00(param1);
        return 0;
    }
    if (param1 < 0)
        return 0;
    if (param1 >= 0xf)
        return 0;
    return g_unk0x0051681c[param1];
}

// FUNCTION: CMR2 0x00408010
int RallyData_FUN_00408010(int index)
{
    return g_unk0x0052ea98[index];
}

// FUNCTION: CMR2 0x00408270
BYTE *RallyData_FUN_00408270(void)
{
    return g_unk0x0052ea74;
}

// FUNCTION: CMR2 0x00408280
int RallyData_FUN_00408280(void)
{
    return g_unk0x0052eab8;
}
