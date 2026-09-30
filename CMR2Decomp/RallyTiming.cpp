#include "RallyTiming.h"
#include "TimingUtils.h"
#include "GameInfo.h"
#include "Graphics.h"
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
typedef char ChampionshipTablesSize[sizeof(ChampionshipTables) == 0x78 ? 1 : -1];
// GLOBAL: CMR2 0x005335b8
ChampionshipTables g_championshipTables;
#define g_unk0x005335b8 (g_championshipTables.totals)
#define g_unk0x005335d8 (g_championshipTables.rounds)
#define g_unk0x005335f0 (g_championshipTables.points)
#define g_unk0x00533610 (g_championshipTables.order)
#define g_unk0x00533618 (g_championshipTables.positions)
#define g_unk0x00533620 (g_championshipTables.tieBreak)
#define g_unk0x00533628 (g_championshipTables.wins)

// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040cc60
void FUN_0040cc60(void)
{
    int i;
    int *p;
    Unk0x005335d8 *q;
    Unk0x005335d8 *r;

    i = 0;
    q = g_unk0x005335d8;
    p = g_unk0x005335f0;
    do {
        g_unk0x005335b8[i] = 0;
        *p = 0;
        g_unk0x00533628[i] = 0;
        g_unk0x00533610[i] = i;
        r = q;
        g_unk0x00533618[i] = i;
        g_unk0x00533620[i] = i;
        r->field_0x0 = 0;
        p++;
        i++;
        q++;
        r->field_0x2 = 0;
    } while ((int)p < (int)(g_unk0x005335f0 + 8));
}

// FUNCTION: CMR2 0x0040ccb0
void FUN_0040ccb0(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_unk0x00533620[g_unk0x00533610[i]] = i;
}

unsigned int RallyData_FUN_00406950(void);
int FUN_0040ce40(int position);

// match 89.7%: below the bar only because the pPoints loop bound compiles to the
// address after g_unk0x005335f0, which reccmp renders as the next .bss global of
// our image (g_unk0x005335d8) instead of the original's g_unk0x00533610.
// Adds the championship points of one finishing order to the per-car totals and
// resolves the ties of the overall classification.
// pPositions: finishing position of each of the eight cars.
// pPoints: points earned at each position.
// FUNCTION: CMR2 0x0040ccd0
void FUN_0040ccd0(char *pPositions, int *pPoints)
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
            g_unk0x00533628[car]++;
        score = FUN_0040ce40(i);
        g_unk0x005335b8[car] += (int)(__int64)((double)score * CGraphics::m_65536);
        *(char *)((char *)g_unk0x005335d8 + car * 3 + (RallyData_FUN_00406950() & 0xff)) =
            (char)score;
    }
    RallyTiming_SortOrder(g_unk0x005335b8, g_unk0x00533610, 0, 8, 0);
    for (i = 0; i < 8; i++)
        g_unk0x00533618[g_unk0x00533610[i]] = (char)i;
    {
        // The original walks the accumulator table up to the global that follows
        // it (g_unk0x00533610); writing the bound with that symbol reproduces the
        // operand the original emits.
        int *pAcc;
        int *pPts;
        for (pAcc = g_unk0x005335f0, pPts = pPoints; (int)pAcc < (int)g_unk0x00533610; pAcc++, pPts++)
            *pAcc += *pPts;
    }
    i = 0;
    do {
        for (j = i + 1; j < 8; j++) {
            car = g_unk0x00533610[i];
            other = g_unk0x00533610[j];
            if (g_unk0x005335b8[car] != g_unk0x005335b8[other])
                break;
            swap = 0;
            if ((char)g_unk0x00533628[other] > (char)g_unk0x00533628[car])
                swap = 1;
            if (g_unk0x00533628[other] == g_unk0x00533628[car] && other == 0)
                swap = 1;
            if (swap) {
                g_unk0x00533618[car] = (char)j;
                g_unk0x00533618[other] = (char)i;
                g_unk0x00533610[i] = (char)other;
                g_unk0x00533610[j] = (char)car;
            }
        }
        i = j;
    } while (i < 8);
}

// FUNCTION: CMR2 0x0040ce30
int FUN_0040ce30(int index)
{
    return g_unk0x00533628[index];
}

// Championship points for a finishing position.
// FUNCTION: CMR2 0x0040ce40
int FUN_0040ce40(int position)
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
int FUN_0040cea0(int index)
{
    return g_unk0x00533610[index];
}

// FUNCTION: CMR2 0x0040ceb0
int FUN_0040ceb0(int index)
{
    return g_unk0x00533618[index];
}

// FUNCTION: CMR2 0x0040cec0
int FUN_0040cec0(int index)
{
    return g_unk0x00533620[index];
}

// FUNCTION: CMR2 0x0040ced0
int FUN_0040ced0(int index)
{
    return g_unk0x005335b8[index] >> 16;
}

// FUNCTION: CMR2 0x0040cef0
int FUN_0040cef0(int index)
{
    return g_unk0x005335f0[index];
}

// FUNCTION: CMR2 0x0040cf00
void FUN_0040cf00(void)
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
    } while ((int)p < (int)g_stagePenalty);
}

unsigned int RallyDataCountryIndex(void);

// Awards the stage points (by position, ties sharing) to the rally totals and
// re-sorts the stage order.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040cf30
void FUN_0040cf30(void)
{
    int i = 0;
    int position = 0;
    int points;
    char driver;

    RallyDataCountryIndex();
    memset(g_stagePenalty, 0, sizeof(g_stagePenalty));
    do {
        driver = g_rallyOverallOrderDriverID[i];
        if (i > 0 && g_rallyOverallTimesRaw[g_rallyOverallOrderDriverID[i - 1]] < g_rallyOverallTimesRaw[driver])
            position = i;
        points = RallyTiming_GetPointsForPosition(position);
        if (position == 0)
            g_stageTieBreak[driver]++;
        g_stagePenalty[driver] = (char)points;
        g_stageTimesRaw[driver] += (int)(__int64)(points * CGraphics::m_65536);
        i++;
    } while (position < 5);
    RallyTiming_SortOrder(g_stageTimesRaw, g_stageOrderDriverID, 0, 16, 0);
    RallyTiming_SortStageOrder();
}

// FUNCTION: CMR2 0x0040cfe0
int FUN_0040cfe0(int index)
{
    return g_stageTieBreak[index];
}

// FUNCTION: CMR2 0x0040cff0
void FUN_0040cff0(int index, char value)
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

// match 37%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d520
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise)
{
	int i;
	int remaining;
	char *pCurrent;
	char a;
	char b;

	if (bInitialise)
	{
		for (i = 0; i < iCount; i++)
			pcOrder[i] = i;
	}

	pCurrent = pcOrder;
	for (remaining = iCount; remaining > 0; remaining--, pCurrent++)
	{
		for (i = pCurrent - pcOrder + 1; i < iCount; i++)
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
int RallyTiming_FUN_0040d3d0(int index)
{
    return ConvertRawTimeToCentiseconds(g_rallyOverallTimesRaw[index]);
}

// FUNCTION: CMR2 0x0040d0c0
void RallyTiming_FUN_0040d0c0(void)
{
    RallyTiming_SortOrder(g_stageTimesRaw, g_stageOrderDriverID, 0, 16, 1);
    RallyTiming_SortStageOrder();
    RallyTiming_SortOrder(g_rallyOverallTimesRaw, g_rallyOverallOrderDriverID, 1, 16, 1);
    RallyTiming_SortOverallOrder();
}


// FUNCTION: CMR2 0x0040d090
void FUN_0040d090(int index, int seconds)
{
    g_stageTimesRaw[index] = (int)(__int64)((double)seconds * CGraphics::m_65536);
}

// Adds each stage's penalty seconds to its raw time.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d010
void FUN_0040d010(void)
{
    int *p;
    int i;

    i = 0;
    p = g_stageTimesRaw;
    do {
        *p += (int)(__int64)((double)g_stagePenalty[i] * g_minus65536);
        i++;
        p++;
    } while ((int)p < (int)&g_stageTimesRaw[16]);
}
