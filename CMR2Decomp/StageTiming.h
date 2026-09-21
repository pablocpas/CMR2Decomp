#ifndef _STAGE_TIMING_H
#define _STAGE_TIMING_H

#include <windows.h>

int StageTiming_GetDriverIDForPosition(int positionIx);
int StageTiming_GetTimeForPosition(int iPosition);

struct StageFile {
    void *buffer;
    unsigned int size;
    BOOL loaded;
    int field_0xc;
};

void StageTiming_FreeStageFile5(void);
void StageTiming_FreeStageFile2(void);
StageFile *StageTiming_GetStageFile0(void);
StageFile *StageTiming_GetStageFile1(void);
StageFile *StageTiming_GetStageFile2(void);
StageFile *StageTiming_GetStageFile3(void);
StageFile *StageTiming_GetStageFile4(void);
StageFile *StageTiming_GetStageFile5(void);
StageFile *StageTiming_GetStageFile6(void);
int StageTiming_FUN_00455460(void);
void StageTiming_FUN_00455610(void);
int StageTiming_FUN_00455ab0(int iSplit);
int StageTiming_FUN_00455ac0(int iSplit, int iIndex);
BYTE StageTiming_FUN_00455ae0(void);
int StageTiming_GetSplitDriverCount(int iSplit);
int StageTiming_GetSplitPositionOfDriver(int iDriver, int iSplit);
int StageTiming_GetCurrentSplitPositionOfDriver(int iDriver);
int StageTiming_GetSplitDriverIDForPosition(int iPosition, int iSplit);
int StageTiming_GetSplitTimeForPosition(int iPosition, int iSplit);
int StageTiming_GetCurrentSplitTimeForDriver(int iDriver);
int StageTiming_GetDriverSlot(int iDriver);
void StageTiming_Reset(void);
void StageTiming_AddToOverall(void);
void StageTiming_GetSplitTimesForPositions(int iPosition1, int iPosition2, int *piTime1, int *piTime2);
void StageTiming_RebuildSplitPositions(void);

int *FUN_00469680(int index);
BYTE *FUN_00456be0(int index);

#endif
