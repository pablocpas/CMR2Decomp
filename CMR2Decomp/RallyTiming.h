#ifndef _RALLY_TIMING_H
#define _RALLY_TIMING_H

struct RallyStageTables {
    char order[16];
    char positions[16];
    int times[16];
    char penalties[16];
    char tieBreak[16];
};
typedef char RallyStageTablesSize[sizeof(RallyStageTables) == 0x80 ? 1 : -1];
extern RallyStageTables g_rallyStageTables;
#define g_stageOrderDriverID (g_rallyStageTables.order)
#define g_stagePositionOfDriver (g_rallyStageTables.positions)
#define g_stageTimesRaw (g_rallyStageTables.times)
#define g_stagePenalty (g_rallyStageTables.penalties)
#define g_stageTieBreak (g_rallyStageTables.tieBreak)

int RallyTiming_GetOverallPositionDriverID(int iPosition);
int RallyTiming_GetOverallTimeForPosition(int iPosition);
void RallyTiming_ResetOverallPlayerTimes(void);
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds);

struct RallyOverallTables {
    char order[16];
    char positions[16];
    int times[16];
    int count;
};
typedef char RallyOverallTablesSize[sizeof(RallyOverallTables) == 0x64 ? 1 : -1];
extern RallyOverallTables g_rallyOverallTables;
#define g_rallyOverallOrderDriverID (g_rallyOverallTables.order)
#define g_rallyOverallPositionOfDriver (g_rallyOverallTables.positions)
#define g_rallyOverallTimesRaw (g_rallyOverallTables.times)
#define g_unk0x00533698 (g_rallyOverallTables.count)

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
