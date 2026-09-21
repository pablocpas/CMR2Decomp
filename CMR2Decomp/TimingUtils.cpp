#include "TimingUtils.h"
#include "RallyTiming.h"

#include <stdio.h>

// STRING: CMR2 0x00519f58
const char *g_minSecMSECFormatString = "%02d:%02d.%02d";

// FUNCTION: CMR2 0x004de170
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime)
{
	sprintf(pcFormattedTime, "%02d:%02d.%02d", iTime / 6000, (iTime % 6000) / 100, iTime % 100);
}

// FUNCTION: CMR2 0x0040d480
int ConvertRawTimeToCentiseconds(int iTime)
{
	// raw times are 16.16 fixed point seconds; 0x28f5c28 = 0.01 * 2^32.
	// The original used inline assembly for the 64-bit intermediate.
	__asm {
		mov eax, iTime
		add eax, 0x147
		mov iTime, eax
		mov eax, iTime
		mov ecx, 0x28f5c28
		cdq
		shld edx, eax, 16
		shl eax, 16
		idiv ecx
	}
}

// FUNCTION: CMR2 0x0040d3f0
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds)
{
	__asm {
		mov eax, iCentiseconds
		mov edx, 0x28f5c28
		imul edx
		shrd eax, edx, 16
		mov ecx, iDriver
		mov dword ptr [ecx*4 + g_rallyOverallTimesRaw], eax
	}
}
