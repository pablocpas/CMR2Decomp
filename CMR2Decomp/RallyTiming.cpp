#include "RallyTiming.h"
#include "FloatToInt.h"
#include "TimingUtils.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "LayoutChecks.h"
extern double g_minus65536;

// GLOBAL: CMR2 0x00533638
RallyOverallTables g_rallyOverallTables;

// GLOBAL: CMR2 0x00533538
RallyStageTables g_rallyStageTables;

#pragma pack(push, 1)
struct Unk0x005335d8 {
    short field_0x0;
    BYTE field_0x2;
};
#pragma pack(pop)
struct ChampionshipTables {
    int totals[8];
    Unk0x005335d8 rounds[8];
    int points[8];
    char order[8];
    char positions[8];
    char tieBreak[8];
    char wins[8];
};
CMR2_LAYOUT_CHECK(ChampionshipTablesSize, sizeof(ChampionshipTables) == 0x78);
// GLOBAL: CMR2 0x005335b8
ChampionshipTables g_championshipTables;

// FUNCTION: CMR2 0x0040cc60
void RallyTiming_ResetDriverSplitState(void)
{
    int i;
    Unk0x005335d8 *q;
    Unk0x005335d8 *r;

    i = 0;
    q = g_championshipTables.rounds;
    do {
        g_championshipTables.totals[i] = 0;
        g_championshipTables.points[i] = 0;
        g_championshipTables.wins[i] = 0;
        g_championshipTables.order[i] = i;
        r = q;
        g_championshipTables.positions[i] = i;
        g_championshipTables.tieBreak[i] = i;
        memset(r, 0, 3);
        i++;
        q++;
    } while (i < 8);
}

// FUNCTION: CMR2 0x0040ccb0
void RallyTiming_RebuildChampionshipPositionMap(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_championshipTables.tieBreak[g_championshipTables.order[i]] = i;
}

unsigned int RallyData_GetSelectionBits12To13(void);
int RallyTiming_GetPositionPoints(int position);

// match 89.7%: below the bar only because the pPoints loop bound compiles to the
// address after g_championshipTables.points, which reccmp renders as the next .bss global of
// our image (g_championshipTables.rounds) instead of the original's g_championshipTables.order.
// Adds the championship points of one finishing order to the per-car totals and
// resolves the ties of the overall classification.
// pPositions: finishing position of each of the eight cars.
// pPoints: points earned at each position.
// FUNCTION: CMR2 0x0040ccd0
void RallyTiming_AddChampionshipPoints(char *pPositions, int *pPoints)
{
    int i;
    int j;
    int car;
    int other;
    int score;
    char swap;

    for (i = 0; i < 8; i++) {
        car = pPositions[i];
        if (i == 0)
            g_championshipTables.wins[car]++;
        score = RallyTiming_GetPositionPoints(i);
        g_championshipTables.totals[car] += (int)FTOI(__int64, X87S_MUL((double)score, CGraphics::m_65536));
        *(char *)((char *)g_championshipTables.rounds + car * 3 + (RallyData_GetSelectionBits12To13() & 0xff)) =
            (char)score;
    }
    RallyTiming_SortOrder(g_championshipTables.totals, g_championshipTables.order, 0, 8, 0);
    for (i = 0; i < 8; i++)
        g_championshipTables.positions[g_championshipTables.order[i]] = (char)i;
    {
        // The original walks the accumulator table up to the global that follows
        // it (g_championshipTables.order); writing the bound with that symbol reproduces the
        // operand the original emits.
        int *pAcc;
        int *pPts;
        for (pAcc = g_championshipTables.points, pPts = pPoints; (INT_PTR)pAcc < (INT_PTR)(g_championshipTables.points + 8); pAcc++, pPts++)
            *pAcc += *pPts;
    }
    i = 0;
    do {
        for (j = i + 1; j < 8; j++) {
            car = g_championshipTables.order[i];
            other = g_championshipTables.order[j];
            if (g_championshipTables.totals[car] != g_championshipTables.totals[other])
                break;
            swap = 0;
            if ((char)g_championshipTables.wins[other] > (char)g_championshipTables.wins[car])
                swap = 1;
            if (g_championshipTables.wins[other] == g_championshipTables.wins[car] && other == 0)
                swap = 1;
            if (swap) {
                g_championshipTables.positions[car] = (char)j;
                g_championshipTables.positions[other] = (char)i;
                g_championshipTables.order[i] = (char)other;
                g_championshipTables.order[j] = (char)car;
            }
        }
        i++;
    } while (i < 8);
}

// FUNCTION: CMR2 0x0040ce30
int RallyTiming_GetChampionshipTieBreak(int index)
{
    return g_championshipTables.wins[index];
}

// Championship points for a finishing position.
// FUNCTION: CMR2 0x0040ce40
int RallyTiming_GetPositionPoints(int position)
{
    switch (position) {
    case 0:
        return 6;
    case 1:
        return 4;
    case 2:
        return 3;
    case 3:
        return 2;
    case 4:
        return 1;
    case 5:
        return 0;
    default:
        return 0;
    }
}

// FUNCTION: CMR2 0x0040cea0
int RallyTiming_GetChampionshipDriverAtPosition(int index)
{
    return g_championshipTables.order[index];
}

// FUNCTION: CMR2 0x0040ceb0
int RallyTiming_GetChampionshipPointsByPosition(int index)
{
    return g_championshipTables.positions[index];
}

// FUNCTION: CMR2 0x0040cec0
int RallyTiming_GetChampionshipDriverPosition(int index)
{
    return g_championshipTables.tieBreak[index];
}

// FUNCTION: CMR2 0x0040ced0
int RallyTiming_GetChampionshipTimeSeconds(int index)
{
    return g_championshipTables.totals[index] >> 16;
}

// FUNCTION: CMR2 0x0040cef0
int RallyTiming_GetChampionshipStagePoints(int index)
{
    return g_championshipTables.points[index];
}

// FUNCTION: CMR2 0x0040cf00
void RallyTiming_ResetStageResults(void)
{
    int i;
    int *p;

    i = 0;
    p = g_stageTimesRaw;
    do {
        g_stageOrderDriverID[i] = i;
        g_stagePositionOfDriver[i] = i;
        *p = 0;
        g_stagePenalty[i] = 0;
        g_stageTieBreak[i] = 0;
        p++;
        i++;
    } while ((INT_PTR)p < (INT_PTR)(g_rallyStageTables.times + 16));
}

unsigned char RallyDataCountryIndex(void);

// Awards the stage points (by position, ties sharing) to the rally totals and
// re-sorts the stage order.
// FUNCTION: CMR2 0x0040cf30
void RallyTiming_AwardStagePoints(void)
{
    int i;
    int position;
    int points;
    int driver;

    RallyDataCountryIndex();
    i = 0;
    memset(g_stagePenalty, 0, sizeof(g_stagePenalty));
    position = 0;
    do {
        driver = g_rallyOverallOrderDriverID[i];
        if (i > 0 && g_rallyOverallTimesRaw[driver] > g_rallyOverallTimesRaw[g_rallyOverallOrderDriverID[i - 1]])
            position = i;
        points = RallyTiming_GetPointsForPosition(position);
        if (position == 0)
            g_stageTieBreak[driver]++;
        g_stagePenalty[driver] = (char)points;
        g_stageTimesRaw[driver] += (int)FTOI(__int64, X87S_MUL(points, CGraphics::m_65536));
        i++;
    } while (position < 5);
    RallyTiming_SortOrder(g_stageTimesRaw, g_stageOrderDriverID, 0, 16, 0);
    RallyTiming_SortStageOrder();
}

// FUNCTION: CMR2 0x0040cfe0
int RallyTiming_GetStageTieBreak(int index)
{
    return g_stageTieBreak[index];
}

// FUNCTION: CMR2 0x0040cff0
void RallyTiming_SetStageTieBreak(int index, char value)
{
    g_stageTieBreak[index] = value;
}

// FUNCTION: CMR2 0x0040d390
int RallyTiming_GetOverallPositionDriverID(int iPosition)
{
	return g_rallyOverallOrderDriverID[iPosition];
}

// FUNCTION: CMR2 0x0040d3b0
int RallyTiming_GetOverallTimeForPosition(int iPosition)
{
	iPosition = g_rallyOverallOrderDriverID[iPosition];
	return ConvertRawTimeToCentiseconds(g_rallyOverallTimesRaw[iPosition]);
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
	} while ((INT_PTR)++ptr < (INT_PTR)(g_rallyOverallTables.times + 16)); // the original bound is the end of the 16 times
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
			if (16 - b <= CGameInfo::GetConfiguredPlayerCount() && 16 - a > CGameInfo::GetConfiguredPlayerCount())
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
			if (g_stageTieBreak[b] == g_stageTieBreak[a] && 16 - b <= CGameInfo::GetConfiguredPlayerCount() && 16 - a > CGameInfo::GetConfiguredPlayerCount())
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
	int remaining;
	INT_PTR offset;
	char *pCurrent;
	int a;
	int b;

	if (bInitialise)
	{
		for (i = 0; i < iCount; i++)
			pcOrder[i] = i;
	}

	if (iCount > 0)
	{
		pCurrent = pcOrder;
		// The start index is pCurrent - pcOrder + 1, but the original hoists
		// 1 - pcOrder as an integer address; pointer subtraction moves code.
		offset = 1 - (INT_PTR)pcOrder;
		for (remaining = iCount; remaining > 0; remaining--, pCurrent++)
		{
			for (i = (int)(offset + (INT_PTR)pCurrent); i < iCount; i++)
			{
				a = *pCurrent;
				b = pcOrder[i];
				switch (iDirection)
				{
				case 1:
					if (piTimes[a] > piTimes[b])
					{
						*pCurrent = b;
						pcOrder[i] = a;
					}
					break;
				case 0:
					if (piTimes[a] < piTimes[b])
					{
						*pCurrent = b;
						pcOrder[i] = a;
					}
					break;
				}
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

// FUNCTION: CMR2 0x0040d3d0
int RallyTiming_GetOverallTimeCentiseconds(int index)
{
    return ConvertRawTimeToCentiseconds(g_rallyOverallTimesRaw[index]);
}

// FUNCTION: CMR2 0x0040d0c0
void RallyTiming_SortStageAndOverallResults(void)
{
    RallyTiming_SortOrder(g_stageTimesRaw, g_stageOrderDriverID, 0, 16, 1);
    RallyTiming_SortStageOrder();
    RallyTiming_SortOrder(g_rallyOverallTimesRaw, g_rallyOverallOrderDriverID, 1, 16, 1);
    RallyTiming_SortOverallOrder();
}


// FUNCTION: CMR2 0x0040d090
void RallyTiming_SetStageTimeSeconds(int index, int seconds)
{
    g_stageTimesRaw[index] = (int)FTOI(__int64, X87S_MUL((double)seconds, CGraphics::m_65536));
}

// Adds each stage's penalty seconds to its raw time.
// FUNCTION: CMR2 0x0040d010
void RallyTiming_AddStagePenalties(void)
{
    int i;

    for (i = 0; i < 16; i++)
        g_stageTimesRaw[i] += (int)FTOI(__int64, X87S_MUL((double)g_stagePenalty[i], g_minus65536));
}
