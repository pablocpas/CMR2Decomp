#include <windows.h>
#include "RallyData.h"
#include "GameInfo.h"
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

// TODO: CMR2 0x00494a70 (implemented, match 26%)
BYTE *FUN_00494a70(void)
{
    switch ((RallyDataCountryIndex() & 0xff) - 1) {
    case 0:
        return g_unk0x00520128 + 1 * 7;
    case 1:
        return g_unk0x00520128 + 2 * 7;
    case 2:
        return g_unk0x00520128 + 3 * 7;
    case 3:
        return g_unk0x00520128 + 4 * 7;
    case 4:
        return g_unk0x00520128 + 5 * 7;
    case 5:
        return g_unk0x00520128 + 6 * 7;
    case 6:
        return g_unk0x00520128 + 7 * 7;
    case 7:
        return g_unk0x00520128 + 8 * 7;
    }
    return g_unk0x00520128;
}
