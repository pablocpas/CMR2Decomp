#ifndef _STAGE_TIMING_H
#define _STAGE_TIMING_H

#include <windows.h>

struct StageNodeTables {
    void *nodes[4];
    BYTE flags[4];
};
typedef char StageNodeTablesSize[sizeof(StageNodeTables) == 0x14 ? 1 : -1];
extern StageNodeTables g_stageNodeTables;
#define g_unk0x00590d7c (g_stageNodeTables.nodes)
#define g_unk0x00590d8c (g_stageNodeTables.flags)

struct ReplayLevelState {
    int levels[16];
    int bufferCount;
    int pending[8];
};
typedef char ReplayLevelSizeCheck[sizeof(ReplayLevelState) == 0x64 ? 1 : -1];
extern ReplayLevelState g_replayLevelState;
#define g_unk0x00588cd4 (g_replayLevelState.levels)
#define g_unk0x00588d14 (g_replayLevelState.bufferCount)
#define g_unk0x00588d18 (g_replayLevelState.pending)

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
int FUN_004692b0(unsigned int type, BYTE *pModel);
void StageDeform_ApplyRadialDent(void);
void StageDeform_ApplyPlanarDent(void);

// One deformable node of a stage record: position, spin rate, angle and scale
// (stride 0x24).
struct StageDeformNode {
    int x;          // 0x00
    int y;          // 0x04
    int z;          // 0x08
    int spin;       // 0x0c
    int field_0x10; // 0x10
    int angle;      // 0x14
    int scale;      // 0x18
    int field_0x1c; // 0x1c
    int wrapped;    // 0x20
};
extern StageDeformNode g_unk0x00543fb0[400];

#endif
