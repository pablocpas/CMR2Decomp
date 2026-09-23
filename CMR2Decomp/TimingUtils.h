#ifndef _TIMING_UTILS_H
#define _TIMING_UTILS_H

extern char g_minSecMSECFormatString[];

void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime);
int ConvertRawTimeToCentiseconds(int iTime);

#endif
