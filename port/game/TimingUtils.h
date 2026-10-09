#ifndef _TIMING_UTILS_H
#define _TIMING_UTILS_H

extern char g_minSecMSECFormatString[];

int ConvertRawTimeToCentiseconds(int iTime);
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime);

#endif
