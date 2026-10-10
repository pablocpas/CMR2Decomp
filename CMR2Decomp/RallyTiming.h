#ifndef _RALLY_TIMING_H
#define _RALLY_TIMING_H
#if defined(_MSC_VER) && _MSC_VER <= 1200
#ifndef CMR2_LAYOUT_CHECK
#define CMR2_LAYOUT_CHECK(name, condition) typedef char name[condition ? 1 : -1]
#endif
#else
#include "LayoutChecks.h"
#endif
struct RallyStageTables {
    char order[16];
    char positions[16];
    int times[16];
    char penalties[16];
    char tieBreak[16];
};
CMR2_LAYOUT_CHECK(RallyStageTablesSize, sizeof(RallyStageTables) == 0x80);
extern RallyStageTables g_rallyStageTables;
#define g_stageOrderDriverID (g_rallyStageTables.order)
#define g_stagePositionOfDriver (g_rallyStageTables.positions)
#define g_stageTimesRaw (g_rallyStageTables.times)
#define g_stagePenalty (g_rallyStageTables.penalties)
#define g_stageTieBreak (g_rallyStageTables.tieBreak)

void RallyTiming_ResetOverallPlayerTimes(void);
int RallyTiming_GetOverallPositionDriverID(int iPosition);
int RallyTiming_GetOverallTimeForPosition(int iPosition);
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds);

struct RallyOverallTables {
    char order[16];
    char positions[16];
    int times[16];
    int count;
};
CMR2_LAYOUT_CHECK(RallyOverallTablesSize, sizeof(RallyOverallTables) == 0x64);
extern RallyOverallTables g_rallyOverallTables;
#define g_rallyOverallOrderDriverID (g_rallyOverallTables.order)
#define g_rallyOverallPositionOfDriver (g_rallyOverallTables.positions)
#define g_rallyOverallTimesRaw (g_rallyOverallTables.times)
#define g_unk0x00533698 (g_rallyOverallTables.count)

void RallyTiming_AddChampionshipPoints(char *pPositions, int *pPoints);
int RallyTiming_GetStageOrderDriverID(int iPosition);
int RallyTiming_GetStagePositionOfDriver(int iDriver);
int RallyTiming_GetStageTimeSeconds(int iDriver);
void RallyTiming_AddStageTimes(char *pcDriverIDs, char *pcTimeDriverIx, int *piTimesRaw);
void RallyTiming_SortOverallOrder(void);
void RallyTiming_SortStageOrder(void);
int RallyTiming_GetOverallPositionOfDriver(int iDriver);
int RallyTiming_GetOverallTimeCentiseconds(int index);
int RallyTiming_GetStagePenalty(int iDriver, int iUnused);
int RallyTiming_GetPointsForPosition(int iPosition);
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise);

#endif
