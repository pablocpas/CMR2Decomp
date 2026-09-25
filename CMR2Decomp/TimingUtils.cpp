#include "TimingUtils.h"
#include "RallyTiming.h"
#include "FixedPoint.h"
#include <windows.h>
#include "Graphics.h"

#include <stdio.h>

// GLOBAL: CMR2 0x00519f58
char g_minSecMSECFormatString[] = "%02d:%02d.%02d";

// FUNCTION: CMR2 0x004de170
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime)
{
	sprintf(pcFormattedTime, g_minSecMSECFormatString, iTime / 6000, (iTime % 6000) / 100, iTime % 100);
}

// FUNCTION: CMR2 0x0040d480
int ConvertRawTimeToCentiseconds(int iTime)
{
	// raw times are 16.16 fixed point seconds; 0x28f5c28 = 0.01 * 2^32.
	return FixDiv(iTime + 0x147, 0x28f5c28);
}

// FUNCTION: CMR2 0x0040d3f0
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds)
{
	g_rallyOverallTimesRaw[iDriver] = FixMul(iCentiseconds, 0x28f5c28);
}

extern double g_minus65536;

// Converts a time in hundredths of a second into 16.16 seconds.
// TODO: CMR2 0x0040d4b0 (implemented, match 60%)
int FUN_0040d4b0(int hundredths)
{
    return FixDiv((int)(__int64)((hundredths % 100) * CGraphics::m_65536), 0x640000) -
           (int)(__int64)((hundredths / 100) * g_minus65536);
}

