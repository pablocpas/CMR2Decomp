#include "RallyTiming.h"
#include "TimingUtils.h"
#include "GameInfo.h"

// GLOBAL: CMR2 0x00533638
char g_rallyOverallOrderDriverID[16];

// GLOBAL: CMR2 0x00533658
int g_rallyOverallTimesRaw[16];

// GLOBAL: CMR2 0x00533698
int g_unk0x00533698;

// GLOBAL: CMR2 0x00533648
char g_rallyOverallPositionOfDriver[16];

// GLOBAL: CMR2 0x00533538
char g_stageOrderDriverID[16];

// GLOBAL: CMR2 0x00533548
char g_stagePositionOfDriver[16];

// GLOBAL: CMR2 0x00533558
int g_stageTimesRaw[16];

// GLOBAL: CMR2 0x00533598
char g_stagePenalty[16];

// GLOBAL: CMR2 0x005335a8
char g_stageTieBreak[16];

// FUNCTION: CMR2 0x0040d390
int RallyTiming_GetOverallPositionDriverID(int iPosition)
{
	return g_rallyOverallOrderDriverID[iPosition];
}

// FUNCTION: CMR2 0x0040d3b0
int RallyTiming_GetOverallTimeForPosition(int iPosition)
{
	int iTime;

	iTime = g_rallyOverallTimesRaw[g_rallyOverallOrderDriverID[iPosition]];
	return ConvertRawTimeToCentiseconds(iTime);
}

// FUNCTION: CMR2 0x0040d100
void RallyTiming_ResetOverallPlayerTimes(void)
{
	int ix = 0;
	int *ptr = g_rallyOverallTimesRaw;

	do
	{
		*ptr = 0;
		g_rallyOverallOrderDriverID[ix] = ix;
		ix++;
	} while ((int)++ptr < (int)&g_unk0x00533698); // the original compares against the global that follows the array
}

// FUNCTION: CMR2 0x0040d050
int RallyTiming_GetStageOrderDriverID(int iPosition)
{
	return g_stageOrderDriverID[iPosition];
}

// FUNCTION: CMR2 0x0040d060
int RallyTiming_GetStagePositionOfDriver(int iDriver)
{
	return g_stagePositionOfDriver[iDriver];
}

// FUNCTION: CMR2 0x0040d070
int RallyTiming_GetStageTimeSeconds(int iDriver)
{
	return g_stageTimesRaw[iDriver] >> 16;
}

// FUNCTION: CMR2 0x0040d3a0
int RallyTiming_GetOverallPositionOfDriver(int iDriver)
{
	return g_rallyOverallPositionOfDriver[iDriver];
}

// FUNCTION: CMR2 0x0040d410
int RallyTiming_GetStagePenalty(int iDriver, int iUnused)
{
	return g_stagePenalty[iDriver];
}

// FUNCTION: CMR2 0x0040d180
void RallyTiming_SortOverallOrder(void)
{
	int i;
	int j;
	int a;
	int b;

	for (i = 0; i < 16; i++)
		g_rallyOverallPositionOfDriver[g_rallyOverallOrderDriverID[i]] = i;

	for (i = 0; i < 16; i++)
	{
		for (j = i + 1; j < 16; j++)
		{
			a = g_rallyOverallOrderDriverID[i];
			b = g_rallyOverallOrderDriverID[j];
			if (g_rallyOverallTimesRaw[a] != g_rallyOverallTimesRaw[b])
				break;
			if (16 - b <= CGameInfo::FUN_00405d70() && 16 - a > CGameInfo::FUN_00405d70())
			{
				g_rallyOverallPositionOfDriver[a] = j;
				g_rallyOverallPositionOfDriver[b] = i;
				g_rallyOverallOrderDriverID[i] = b;
				g_rallyOverallOrderDriverID[j] = a;
			}
		}
	}
}

// FUNCTION: CMR2 0x0040d270
void RallyTiming_SortStageOrder(void)
{
	int i;
	int j;
	int a;
	int b;
	char bSwap;

	for (i = 0; i < 16; i++)
		g_stagePositionOfDriver[g_stageOrderDriverID[i]] = i;

	for (i = 0; i < 16; i++)
	{
		for (j = i + 1; j < 16; j++)
		{
			a = g_stageOrderDriverID[i];
			b = g_stageOrderDriverID[j];
			if (g_stageTimesRaw[a] != g_stageTimesRaw[b])
				break;
			bSwap = 0;
			if (g_stageTieBreak[b] > g_stageTieBreak[a])
				bSwap = 1;
			if (g_stageTieBreak[b] == g_stageTieBreak[a] && 16 - b <= CGameInfo::FUN_00405d70() && 16 - a > CGameInfo::FUN_00405d70())
				bSwap = 1;
			if (bSwap)
			{
				g_stagePositionOfDriver[a] = j;
				g_stagePositionOfDriver[b] = i;
				g_stageOrderDriverID[i] = b;
				g_stageOrderDriverID[j] = a;
			}
		}
	}
}

// FUNCTION: CMR2 0x0040d420
int RallyTiming_GetPointsForPosition(int iPosition)
{
	switch (iPosition)
	{
	case 0:
		return 12;
	case 1:
		return 8;
	case 2:
		return 6;
	case 3:
		return 4;
	case 4:
		return 2;
	case 5:
		return 1;
	default:
		return 0;
	}
}

// FUNCTION: CMR2 0x0040d520
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise)
{
	int i;
	int j;
	char a;
	char b;

	if (bInitialise)
	{
		for (i = 0; i < iCount; i++)
			pcOrder[i] = i;
	}

	for (i = 0; i < iCount; i++)
	{
		for (j = i + 1; j < iCount; j++)
		{
			a = pcOrder[i];
			b = pcOrder[j];
			if (iDirection == 0)
			{
				if (piTimes[a] < piTimes[b])
				{
					pcOrder[i] = b;
					pcOrder[j] = a;
				}
			}
			else if (iDirection == 1 && piTimes[b] < piTimes[a])
			{
				pcOrder[i] = b;
				pcOrder[j] = a;
			}
		}
	}
}

// FUNCTION: CMR2 0x0040d120
void RallyTiming_AddStageTimes(char *pcDriverIDs, char *pcTimeDriverIx, int *piTimesRaw)
{
	int i;
	char d;
	char t;

	for (i = 0; i < 16; i++)
	{
		t = pcTimeDriverIx[i];
		d = pcDriverIDs[i];
		g_rallyOverallTimesRaw[d] += piTimesRaw[t];
	}
	RallyTiming_SortOrder(g_rallyOverallTimesRaw, g_rallyOverallOrderDriverID, 1, 16, 0);
	RallyTiming_SortOverallOrder();
}
