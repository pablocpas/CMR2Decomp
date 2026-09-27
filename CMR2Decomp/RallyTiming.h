#ifndef _RALLY_TIMING_H
#define _RALLY_TIMING_H

int RallyTiming_GetOverallPositionDriverID(int iPosition);
int RallyTiming_GetOverallTimeForPosition(int iPosition);
void RallyTiming_ResetOverallPlayerTimes(void);
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds);

extern int g_rallyOverallTimesRaw[16];

int RallyTiming_GetStageOrderDriverID(int iPosition);
int RallyTiming_GetStagePositionOfDriver(int iDriver);
int RallyTiming_GetStageTimeSeconds(int iDriver);
int RallyTiming_FUN_0040d3d0(int index);
int RallyTiming_GetOverallPositionOfDriver(int iDriver);
int RallyTiming_GetStagePenalty(int iDriver, int iUnused);
void RallyTiming_SortOverallOrder(void);
void RallyTiming_SortStageOrder(void);
int RallyTiming_GetPointsForPosition(int iPosition);
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise);
void RallyTiming_AddStageTimes(char *pcDriverIDs, char *pcTimeDriverIx, int *piTimesRaw);
void FUN_0040ccd0(char *pPositions, int *pPoints);

#endif
