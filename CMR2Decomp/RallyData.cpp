#include <windows.h>
#include "RallyData.h"
#include "GameInfo.h"
#include "RallyTiming.h"
#include "main.h"
#include "Frontend.h"
#include "AIHelper.h"

// GLOBAL: CMR2 0x0052f2a9
BYTE g_unk0x0052f2a9;

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
BYTE g_unk0x0051682c[140];

// FUNCTION: CMR2 0x00406890
char *RallyData_FUN_00406890(void)
{
    unsigned char country;

    country = (unsigned char)RallyDataCountryIndex();
    return (char *)g_unk0x0051682c + country * 7;
}

// FUNCTION: CMR2 0x00406960
void RallyData_FUN_00406960(BYTE param1)
{
    g_selectedRallyData = (g_selectedRallyData & 0xffffcfffU) | ((param1 & 3) << 12);
    RallyData_UpdateFlags();
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

// GLOBAL: CMR2 0x00538a84
int g_unk0x00538a84;
// GLOBAL: CMR2 0x00538a94
int g_unk0x00538a94;
// GLOBAL: CMR2 0x00538ab0
int g_unk0x00538ab0[16 * 6];
// GLOBAL: CMR2 0x00538c94
int g_unk0x00538c94;

// Scales the per-record value at p[0xb1a] to a 0..0x10000 ratio.
// TODO: CMR2 0x00421470 (implemented, match 63%)
int RallyData_FUN_00421470(BYTE *p)
{
    int value;
    int result;

    if (g_unk0x00538a84 == 0)
        return 0;
    value = g_unk0x00538ab0[(signed char)p[0xb1a] * 6];
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

// FUNCTION: CMR2 0x00411060
int RallyData_FUN_00411060(void)
{
    return g_unk0x00536be0;
}
