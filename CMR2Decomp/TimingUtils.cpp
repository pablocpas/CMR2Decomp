#include "TimingUtils.h"
#include "RallyTiming.h"
#include "FixedPoint.h"

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
