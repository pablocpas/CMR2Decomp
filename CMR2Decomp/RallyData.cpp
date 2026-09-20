#include <windows.h>
#include "RallyData.h"

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
unsigned int RallyDataStageIndex(void)
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
