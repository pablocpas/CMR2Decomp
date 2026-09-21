#include <windows.h>
#include "RallyData.h"
#include "GameInfo.h"

// GLOBAL: CMR2 0x0052f2a9
BYTE g_unk0x0052f2a9;

// GLOBAL: CMR2 0x0052f2ac
unsigned int g_selectedRallyData = 0;

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
