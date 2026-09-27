#include <stdio.h>

void FUN_00417e70(char *text, int *pColour, int car, int param4, int x, int y);
#include <string.h>
#include <windows.h>
#include "RallyData.h"
#include "RallyRoute.h"
#include "Car.h"
#include "GameInfo.h"
#include "RallyTiming.h"
#include "main.h"
#include "Frontend.h"

// Championship save data, one contiguous block as in the original:
//   0x52f3e0  8 driver records of 0xc4 bytes
//   0x52fa00  per-category "name edited" flags
//   0x52fa10  4 category records of 0x650 bytes (counters at +0x10, name at +0x14)
//   0x531350  16 index records of 0x30 bytes (category in bits 18..21 of +0)
// GLOBAL: CMR2 0x0052f3e0
BYTE g_saveData[0x2270];
#define g_unk0x0052f3e0 (g_saveData)
#define g_unk0x0052f3e8 (g_saveData + 0x8)
#define g_unk0x0052f3ec (g_saveData + 0xc)
#define g_unk0x0052fa18 (g_saveData + 0x638)
#define g_unk0x0052fa24 (g_saveData + 0x644)
#define g_unk0x0052fa5c (g_saveData + 0x67c)
#define g_unk0x00531350 (g_saveData + 0x1f70)
#include "AIHelper.h"
#include "RegKey.h"
#include "Font.h"
#include "Sprite.h"
#include "Graphics.h"
#include "StageSplitData.h"
#include "FixedPoint.h"
#include <stdlib.h>
#include "StageTiming.h"
#include "StageUI.h"
#include "FileBuffer.h"
#include "Input.h"

int FUN_00456c00(int index);
int FUN_0041f3d0(BYTE index);
int FUN_00418570(void);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void RallyData_FUN_004084c0(BYTE index, BYTE param2);
int FUN_004582d0(int index);
bool FUN_00459390(void);
int FUN_00448c60(int index);
int FUN_004481c0(int car);
int FUN_0040a700(int index);
unsigned int FUN_0040a3d0(void);
char *FUN_0040a400(void);
unsigned int FUN_0040a410(int split);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082c0(void);
unsigned int RallyData_FUN_004082e0(void);
unsigned int RallyData_FUN_00407e90(void);
int FUN_0040e180(int exclude1, int exclude2);
int FUN_0040e210(int exclude1, int exclude2);

// GLOBAL: CMR2 0x0052f2a9
BYTE g_unk0x0052f2a9;

// GLOBAL: CMR2 0x0052f2a8
BYTE g_unk0x0052f2a8;
// Original 9 x 30 rally lookup table; the next data begins at 0x5166b4.
// GLOBAL: CMR2 0x0051627c
unsigned int g_unk0x0051627c[9][30] = {
    {
        0, 0, 0, 1, 1, 1, 1, 2, 2, 3,
        0, 0, 0, 1, 1, 1, 1, 2, 2, 3,
        0, 0, 1, 1, 1, 1, 2, 2, 2, 3
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 3,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 4
    },
    {
        0, 0, 1, 1, 2, 2, 6, 6, 7, 7,
        0, 1, 1, 2, 2, 2, 6, 7, 7, 8,
        1, 2, 2, 6, 7, 7, 7, 7, 8, 8
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 2,
        0, 0, 0, 0, 0, 1, 1, 1, 2, 3,
        0, 0, 0, 0, 0, 1, 1, 1, 2, 4
    },
    {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 1, 1, 1, 1, 2, 4,
        0, 0, 0, 0, 0, 1, 1, 2, 2, 4
    },
    {
        0, 0, 1, 1, 1, 1, 1, 2, 2, 3,
        1, 0, 1, 1, 1, 2, 2, 2, 3, 4,
        1, 0, 1, 1, 2, 2, 2, 3, 4, 5
    },
    {
        1, 1, 1, 2, 2, 2, 3, 3, 4, 4,
        1, 2, 2, 2, 2, 3, 3, 4, 4, 5,
        2, 2, 2, 2, 3, 3, 4, 4, 4, 5
    },
    {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    }
};
// GLOBAL: CMR2 0x0052f1a0
int g_unk0x0052f1a0[20];

// Picks the AI skill of the four opponents of a slot: for each opponent a
// random window around its rating (pRatings[1], [3], [5], [7]) is spread over
// the nine skill classes of pClasses (one per 10 points), and the most
// frequent class wins. Slot 2 only uses two opponents.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d6c0
void FUN_0040d6c0(int slot, int *pClasses, char *pRatings)
{
    unsigned int counts[9];
    int classes[4];
    int total;
    int opponent;
    int *pClass;
    int bestClass;
    int lo;
    int hi;
    int range;
    int best;
    int i;
    int j;
    int *pDest;

    pClass = classes;
    pRatings++;
    opponent = 4;
    do {
        memset(counts, 0, sizeof(counts));
        lo = *pRatings - rand() % 20 - 1;
        hi = rand() % 20 + *pRatings + 1;
        if (lo < 0)
            lo = 0;
        else if (hi > 100)
            hi = 100;
        range = 0;
        if (lo < hi) {
            range = hi - lo;
            for (j = lo; j < hi; j++)
                counts[pClasses[j / 10]]++;
        }
        best = -100;
        total = range << 16;
        bestClass = -1;
        for (i = 0; i < 9; i++) {
            counts[i] = FixMulShift32(0x640000, FixDiv(counts[i] << 16, total));
            if ((int)counts[i] > best) {
                best = counts[i];
                bestClass = i;
            }
        }
        *pClass++ = bestClass;
        pRatings += 2;
    } while (--opponent);
    pDest = &g_unk0x0052f1a0[slot * 4];
    for (i = 0; i < 4; i++) {
        if (slot == 2 && i > 1)
            pDest[i] = 0;
        else
            pDest[i] = classes[i];
    }
}

// GLOBAL: CMR2 0x0052f100
int g_unk0x0052f100[20][2];
// GLOBAL: CMR2 0x0052f1f0
int g_unk0x0052f1f0[20];
// GLOBAL: CMR2 0x0052f240
int g_unk0x0052f240[20];

void FUN_0040d9e0(int group);

// Restarts the driver pairing tables of a rally: both values of every pair, the
// AI skill array and the entry array take the rally key, the two flag arrays
// are cleared, and the three groups of the rally are rebuilt.
// MSVC folds the three int[20] tables into one base register with +-0x50
// displacements in the original (they are contiguous in its .bss); our tables
// are not adjacent, so the compiler picks another induction variable and the
// opcodes differ.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040dbe0
void FUN_0040dbe0(int value)
{
    int i;

    i = 20;
    do {
        i--;
        g_unk0x0052f100[i][0] = value;
        g_unk0x0052f100[i][1] = value;
        g_unk0x0052f1a0[i] = value;
        g_unk0x0052f1f0[i] = 0;
        g_unk0x0052f240[i] = 0;
    } while (i);
    FUN_0040d9e0(0);
    FUN_0040d9e0(1);
    FUN_0040d9e0(2);
}

// GLOBAL: CMR2 0x0052f294
BYTE g_unk0x0052f294[0x14];
// GLOBAL: CMR2 0x0052f2ac
unsigned int g_selectedRallyData = 0;

// GLOBAL: CMR2 0x0052f2b0
unsigned int g_unk0x0052f2b0;

// GLOBAL: CMR2 0x0052f2b4
KnockoutTable g_knockout;

// Seeds the knockout bracket, separates human drivers where possible and
// puts the lower human driver index first in each all-human pairing.
// match 11%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004069c0
void RallyData_InitKnockoutBracket(void)
{
    unsigned int state;
    unsigned int players;
    unsigned int flags;
    unsigned int first;
    unsigned int second;
    KnockoutMatch *round;
    int matchCount = 0;
    int participantCount;
    int roundIndex;
    int i;
    int j;
    int pick;
    BYTE aiIndex = 0;

    state = (g_knockout.state & 0xffff03ffU) | 0x200;
    switch (g_knockout.state & 7) {
    case 1: matchCount = 1; state = (g_knockout.state & 0xffff03e7U) | 0x220; break;
    case 2: matchCount = 2; state = (g_knockout.state & 0xffff03dfU) | 0x218; break;
    case 3: matchCount = 4; state = (g_knockout.state & 0xffff03d7U) | 0x210; break;
    case 4: matchCount = 8; state = (g_knockout.state & 0xffff03cfU) | 0x208; break;
    }
    g_knockout.state = state;
    players = CGameInfo::FUN_00405d70() & 0xff;

    for (i = 0; i < 8; i++) {
        g_knockout.round1[i].flags &= 0xfffffbffU;
        g_knockout.round1[i].time1 = 0;
        g_knockout.round1[i].time2 = 0;
        g_knockout.round1[i].flags = (g_knockout.round1[i].flags & 0xffffe7ffU) | 0x3ff;
    }
    for (i = 0; i < 4; i++) {
        g_knockout.quarters[i].flags &= 0xfffffbffU;
        g_knockout.quarters[i].time1 = 0;
        g_knockout.quarters[i].time2 = 0;
        g_knockout.quarters[i].flags = (g_knockout.quarters[i].flags & 0xffffe7ffU) | 0x3ff;
    }
    for (i = 0; i < 2; i++) {
        g_knockout.semis[i].flags &= 0xfffffbffU;
        g_knockout.semis[i].time1 = 0;
        g_knockout.semis[i].time2 = 0;
        g_knockout.semis[i].flags = (g_knockout.semis[i].flags & 0xffffe7ffU) | 0x3ff;
    }
    g_knockout.final.time1 = 0;
    g_knockout.final.time2 = 0;
    g_knockout.final.flags = (g_knockout.final.flags & 0xffffe3ffU) | 0x3ff;

    switch (g_knockout.state & 7) {
    case 1: round = &g_knockout.final; break;
    case 2: round = g_knockout.semis; break;
    case 3: round = g_knockout.quarters; break;
    case 4: round = g_knockout.round1; break;
    default: round = NULL; break;
    }
    if (round != NULL) {
        participantCount = CGameInfo::FUN_00405dd0() ? matchCount * 2 : players;
        for (i = 0; i < participantCount; i++) {
            if ((CGameInfo::FUN_00405d70() & 0xff) <= (unsigned int)i)
                RallyData_FUN_004084c0((BYTE)i, aiIndex++);
            for (;;) {
                pick = rand() & (matchCount * 2 - 1);
                flags = round[pick >> 1].flags;
                if (pick & 1) {
                    if ((flags & 0x3e0) != 0x3e0)
                        continue;
                    round[pick >> 1].flags = (flags & 0xfffffc1fU) | ((i & 0x1f) << 5);
                    break;
                }
                if ((flags & 0x1f) != 0x1f)
                    continue;
                round[pick >> 1].flags = (flags & 0xffffffe0U) | (i & 0x1f);
                break;
            }
        }
    }

    for (i = 0; i < matchCount; i++) {
        flags = g_knockout.round1[i].flags;
        if ((flags & 0x1f) < (CGameInfo::FUN_00405d70() & 0xff) &&
            ((flags >> 5) & 0x1f) < (CGameInfo::FUN_00405d70() & 0xff) && i + 1 < matchCount) {
            for (j = i + 1; j < matchCount; j++) {
                if ((CGameInfo::FUN_00405d70() & 0xff) <= (g_knockout.round1[j].flags & 0x1f)) {
                    flags = g_knockout.round1[j].flags;
                    if ((CGameInfo::FUN_00405d70() & 0xff) <= ((flags >> 5) & 0x1f)) {
                        second = (g_knockout.round1[i].flags >> 5) & 0x1f;
                        g_knockout.round1[j].flags = (flags & 0xffffffe0U) | second;
                        g_knockout.round1[i].flags =
                            (g_knockout.round1[i].flags & 0xfffffc1fU) | ((flags & 0x1f) << 5);
                    }
                }
            }
        }
    }

    for (roundIndex = 0; roundIndex < 4; roundIndex++) {
        if (roundIndex == 0) { round = g_knockout.round1; matchCount = 8; }
        else if (roundIndex == 1) { round = g_knockout.quarters; matchCount = 4; }
        else if (roundIndex == 2) { round = g_knockout.semis; matchCount = 2; }
        else { round = &g_knockout.final; matchCount = 1; }
        for (i = 0; i < matchCount; i++) {
            first = round[i].flags & 0x1f;
            if (first < (CGameInfo::FUN_00405d70() & 0xff)) {
                second = (round[i].flags >> 5) & 0x1f;
                if (second < (CGameInfo::FUN_00405d70() & 0xff) && second < first)
                    round[i].flags = (round[i].flags & 0xfffffc00U) | second | (first << 5);
            }
        }
    }
}

// FUNCTION: CMR2 0x00407820
unsigned int *RallyData_GetChampionshipState(void)
{
    return &g_knockout.state;
}

// Decodes the two 5-bit driver indices of the current round; which table
// they come from depends on the championship stage (bits 3-5).
// FUNCTION: CMR2 0x00407830
void RallyData_GetRoundDrivers(unsigned int *pFirst, unsigned int *pSecond)
{
    switch ((g_knockout.state >> 3) & 7) {
    case 1:
        *pFirst = g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags & 0x1f;
        *pSecond = (g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags >> 5) & 0x1f;
        break;
    case 2:
        *pFirst = g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags & 0x1f;
        *pSecond = (g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags >> 5) & 0x1f;
        break;
    case 3:
        *pFirst = g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags & 0x1f;
        *pSecond = (g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags >> 5) & 0x1f;
        break;
    case 4:
        *pFirst = g_knockout.final.flags & 0x1f;
        *pSecond = (g_knockout.final.flags >> 5) & 0x1f;
        break;
    }
}

// Stores the two times of the current round of the knockout table and marks
// the winner (bit 11: second driver faster, bit 12: first driver faster).
// FUNCTION: CMR2 0x00407940
void FUN_00407940(unsigned int first, unsigned int second)
{
    switch ((g_knockout.state >> 3) & 7) {
    case 1:
        g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].time1 = first;
        g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].time2 = second;
        if (second > first)
            g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags & 0xffffefff) | 0x800;
        else
            g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.round1[(g_knockout.state >> 0xc) & 0xf].flags & 0xfffff7ff) | 0x1000;
        break;
    case 2:
        g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].time1 = first;
        g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].time2 = second;
        if (second > first)
            g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags & 0xffffefff) | 0x800;
        else
            g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.quarters[(g_knockout.state >> 0xc) & 0xf].flags & 0xfffff7ff) | 0x1000;
        break;
    case 3:
        g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].time1 = first;
        g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].time2 = second;
        if (second > first)
            g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags & 0xffffefff) | 0x800;
        else
            g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags =
                (g_knockout.semis[(g_knockout.state >> 0xc) & 0xf].flags & 0xfffff7ff) | 0x1000;
        break;
    case 4:
        g_knockout.final.time1 = first;
        g_knockout.final.time2 = second;
        if (second > first)
            g_knockout.final.flags = (g_knockout.final.flags & 0xffffefff) | 0x800;
        else
            g_knockout.final.flags = (g_knockout.final.flags & 0xfffff7ff) | 0x1000;
        break;
    }
}

// FUNCTION: CMR2 0x004070f0
int RallyData_FUN_004070f0(void)
{
    unsigned int drivers[2];

    if (CGameInfo::FUN_00405d80() == 4) {
        RallyData_GetChampionshipState();
        RallyData_GetRoundDrivers(&drivers[0], &drivers[1]);
        if (drivers[0] != drivers[1] && RallyData_FUN_00408500(drivers[0]) == -1 && RallyData_FUN_00408500(drivers[1]) == -1)
            return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x00411880
int RallyData_FUN_00411880(void)
{
    if (((BYTE)RallyDataState() > 1 && CGameInfo::FUN_00405da0() == 0 && CGameInfo::FUN_00405d80() != 4) || RallyData_FUN_004070f0() != 0)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x00406910
unsigned int RallyDataCountryIndex(void)
{
	return g_selectedRallyData & 0x1f;
}

// FUNCTION: CMR2 0x00406920
BYTE RallyData_FUN_00406920(void)
{
    return g_unk0x0052f2a8;
}

// FUNCTION: CMR2 0x00406930
unsigned char RallyDataStageIndex(void)
{
	return g_selectedRallyData >> 5 & 0x1f;
}

// FUNCTION: CMR2 0x004074f0
unsigned int RallyDataState(void)
{
	return g_selectedRallyData >> 0xe & 3;
}

// FUNCTION: CMR2 0x00407e50
unsigned int RallyData_GetFlag24(void)
{
	return g_selectedRallyData >> 0x18 & 1;
}

// FUNCTION: CMR2 0x00407e60
unsigned int RallyData_GetFlag25(void)
{
	return g_selectedRallyData >> 0x19 & 1;
}

// Index check compiled out of the release build; kept because every
// RallyData accessor still calls it.
// FUNCTION: CMR2 0x004083f0
void RallyData_ValidateIndex(int index)
{
}


// Increments counter 8 of this entry's category unless the entry is unassigned.
// FUNCTION: CMR2 0x00408f20
void RallyData_FUN_00408f20(int index)
{
    unsigned int record;
    unsigned int category;

    RallyData_ValidateIndex(index);
    record = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    if ((record & 0x3c0000) != 0x3c0000) {
        category = (record >> 18) & 15;
        ++g_unk0x0052fa18[8 + category * 0x650];
    }
}

// Increments the use count of this entry's category unless the entry is unassigned.
// FUNCTION: CMR2 0x00408f70
void RallyData_IncrementCategoryUse(int index)
{
    unsigned int record;
    unsigned int category;

    RallyData_ValidateIndex(index);
    record = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    if ((record & 0x3c0000) != 0x3c0000) {
        category = (record >> 18) & 15;
        ++g_unk0x0052fa18[9 + category * 0x650];
    }
}

// Increments counter 10 of this entry's category unless the entry is unassigned.
// FUNCTION: CMR2 0x00408fc0
void RallyData_FUN_00408fc0(int index)
{
    unsigned int record;
    unsigned int category;

    RallyData_ValidateIndex(index);
    record = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    if ((record & 0x3c0000) != 0x3c0000) {
        category = (record >> 18) & 15;
        ++g_unk0x0052fa18[10 + category * 0x650];
    }
}

BYTE *FUN_0041b390(void);
int FUN_004232a0(int index, int mode);

// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423970
int FUN_00423970(unsigned int index, int mode)
{
    switch (mode) {
    case 1:
    case 3:
    case 4:
    case 5:
        break;
    case 2:
        if ((*(BYTE **)(FUN_0041b390() + 4))[(index & 0xff) * 8] == 10)
            return 0;
        break;
    default:
        return 0;
    }
    if (FUN_004232a0(index, mode) == 0)
        return 0;
    return 1;
}

// FUNCTION: CMR2 0x004239e0
void FUN_004239e0(int *p)
{
    if (p[1] == 3)
        RallyData_ValidateIndex((int)p);
}

// FUNCTION: CMR2 0x00406940
unsigned int RallyData_FUN_00406940(void)
{
	return g_selectedRallyData >> 10 & 3;
}

// FUNCTION: CMR2 0x00406950
unsigned int RallyData_FUN_00406950(void)
{
	return g_selectedRallyData >> 12 & 3;
}

// FUNCTION: CMR2 0x00406990
unsigned int RallyData_FUN_00406990(void)
{
	return g_selectedRallyData >> 16 & 0xf;
}

// FUNCTION: CMR2 0x004069b0
unsigned int RallyData_FUN_004069b0(void)
{
    return g_knockout.state & 7;
}

// FUNCTION: CMR2 0x004069a0
BYTE RallyData_FUN_004069a0(void)
{
	BYTE result = g_unk0x0052f2a9;
	result &= 0xf;
	return result;
}

// FUNCTION: CMR2 0x00408020
void RallyData_UpdateFlags(void)
{
	g_selectedRallyData &= ~0x2000000;
	if (RallyDataStageIndex() == 10)
		g_selectedRallyData |= 0x2000000;

	g_selectedRallyData &= ~0x10000000;
	if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 3)
		g_selectedRallyData |= 0x10000000;

	g_selectedRallyData &= ~0x8000000;
	if ((g_selectedRallyData & 0x2000000) && !(g_selectedRallyData & 0x10000000))
		g_selectedRallyData |= 0x8000000;

	g_selectedRallyData &= ~0x1000000;
	if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 || CGameInfo::FUN_00405d80() == 7 ||
	    CGameInfo::FUN_00405d80() == 11 || CGameInfo::FUN_00405d80() == 12)
		g_selectedRallyData |= 0x1000000;

	g_selectedRallyData &= ~0x4000000;
	if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6)
		g_selectedRallyData |= 0x4000000;

	g_selectedRallyData &= ~0x80000000;
	if (CGameInfo::FUN_00405d80() == 11 || CGameInfo::FUN_00405d80() == 12)
		g_selectedRallyData |= 0x80000000;

	g_selectedRallyData &= ~0x400000;
	if (g_selectedRallyData & 0xc000000)
		g_selectedRallyData |= 0x400000;

	g_selectedRallyData &= ~0x20000000;
	if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1 || CGameInfo::FUN_00405d80() == 2 ||
	    CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 || CGameInfo::FUN_00405d80() == 10)
		g_selectedRallyData |= 0x20000000;

	if (!(g_selectedRallyData & 0x1000000))
		g_selectedRallyData |= 0x40000000;

	g_selectedRallyData &= ~0x200000;
	if ((g_selectedRallyData & 0x20000000) || CGameInfo::FUN_00405d80() == 4)
		g_selectedRallyData |= 0x200000;

	g_selectedRallyData &= ~0x800000;
	if (g_selectedRallyData & 0x31000000)
		g_selectedRallyData |= 0x800000;
}

// FUNCTION: CMR2 0x0040d5c0
void RallyData_ResetSelection(void)
{
	g_selectedRallyData = (g_selectedRallyData & 0xfff3c000) | 0x30000;
	g_knockout.state = (g_knockout.state & 0xfff802cc) | 0x802cc;
	g_unk0x0052f2b0 &= 0xfffffff8;
}

// FUNCTION: CMR2 0x0040d600
void RallyData_FUN_0040d600(BYTE param1)
{
	g_selectedRallyData = ((param1 & 3) << 10) | (g_selectedRallyData & 0xfffff3ffU);
}

// FUNCTION: CMR2 0x0040d620
void RallyData_FUN_0040d620(BYTE param1)
{
	g_selectedRallyData = ((param1 & 0xf) << 16) | (g_selectedRallyData & 0xfff0ffffU);
}

// FUNCTION: CMR2 0x0040d640
void RallyData_FUN_0040d640(BYTE param1)
{
	g_unk0x0052f2a9 = g_unk0x0052f2a9 ^ (g_unk0x0052f2a9 ^ param1) & 0xf;
}

// FUNCTION: CMR2 0x0040d660
void RallyData_FUN_0040d660(BYTE param1)
{
	g_knockout.state = (param1 & 7) | (g_knockout.state & 0xfffffff8U);
}

// FUNCTION: CMR2 0x0040d680
void RallyData_FUN_0040d680(BYTE param1)
{
	g_knockout.state = ((param1 & 7) << 6) | (g_knockout.state & 0xfffffe3fU);
}

// FUNCTION: CMR2 0x0040d6a0
void RallyData_FUN_0040d6a0(BYTE param1)
{
	g_knockout.state = ((param1 & 0xf) << 16) | (g_knockout.state & 0xfff0ffffU);
}

// GLOBAL: CMR2 0x0051682c
BYTE g_unk0x0051682c[132] = {
    0x04, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00, 0x02,
    0x32, 0x32, 0x32, 0x32, 0x32, 0x06, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x02, 0x32, 0x32,
    0x32, 0x32, 0x32, 0x04, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00, 0x02, 0x32, 0x32, 0x32, 0x32,
    0x32, 0x02, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x20, 0x00, 0x00, 0x00, 0x53, 0x4e, 0x4f, 0x57, 0x00, 0x00, 0x00, 0x00,
    0x53, 0x54, 0x4f, 0x52, 0x4d, 0x00, 0x00, 0x00, 0x52, 0x41, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x00,
    0x44, 0x52, 0x49, 0x5a, 0x5a, 0x4c, 0x45, 0x00, 0x43, 0x4c, 0x4f, 0x55, 0x44, 0x59, 0x00, 0x00,
    0x42, 0x52, 0x49, 0x47, 0x48, 0x54, 0x00, 0x00, 0x42, 0x4c, 0x49, 0x5a, 0x5a, 0x41, 0x52, 0x44,
};

BYTE *RallyData_FUN_00407630(int index);
extern int g_unk0x0052f290;
extern int g_unk0x0052f0fc;

// Copies the country's default 7-byte settings into the four player records;
// in some championship stages the first value is bumped to the next odd one.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00406820
void FUN_00406820(void)
{
    int i;
    BYTE *p;
    BYTE *pRow;
    int row;
    char c;

    for (i = 0; i < 4; i++) {
        p = RallyData_FUN_00407630(i);
        row = (RallyDataCountryIndex() & 0xff) * 7;
        pRow = g_unk0x0051682c + row;
        *(int *)p = *(int *)pRow;
        *(short *)(p + 4) = *(short *)(pRow + 4);
        p[6] = pRow[6];
        if (g_unk0x0052f290 != 0 && g_unk0x0052f0fc > 2) {
            c = p[0];
            if (c % 2 == 0 && c != 6)
                p[0] = c + 1;
        }
    }
}

// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00406890
char *RallyData_FUN_00406890(void)
{
    return (char *)g_unk0x0051682c + (unsigned char)RallyDataCountryIndex() * 7;
}

// FUNCTION: CMR2 0x004068b0
void RallyData_FUN_004068b0(BYTE param1)
{
    g_selectedRallyData = (param1 & 0x1f) | (g_selectedRallyData & 0xffffffe0U);
}

// FUNCTION: CMR2 0x00406960
void RallyData_FUN_00406960(BYTE param1)
{
    g_selectedRallyData = (g_selectedRallyData & 0xffffcfffU) | ((param1 & 3) << 12);
    RallyData_UpdateFlags();
}

// FUNCTION: CMR2 0x004068d0
void RallyData_FUN_004068d0(char param1)
{
    g_unk0x0052f2a8 = param1;
}

// FUNCTION: CMR2 0x004068e0
void RallyData_FUN_004068e0(BYTE param1)
{
    g_selectedRallyData = (g_selectedRallyData & 0xfffffc1fU) | ((param1 & 0x1f) << 5);
    RallyData_UpdateFlags();
}

// GLOBAL: CMR2 0x005200e8
BYTE g_unk0x005200e8[0x40] = {
    0x00, 0x04, 0x64, 0x32, 0x32, 0x32, 0x32, 0x06, 0x04, 0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x04,
    0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x04, 0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x04, 0x32, 0x32,
    0x32, 0x32, 0x32, 0x00, 0x04, 0x64, 0x32, 0x32, 0x32, 0x32, 0x04, 0x04, 0x00, 0x32, 0x32, 0x32,
    0x32, 0x02, 0x04, 0x32, 0x32, 0x32, 0x32, 0x32, 0x00, 0x04, 0x64, 0x32, 0x32, 0x32, 0x32,
};

// FUNCTION: CMR2 0x00494a40
char *RallyData_FUN_00494a40(void)
{
    return (char *)g_unk0x005200e8 +
        ((unsigned char)RallyData_FUN_00406940() * 3 +
         (unsigned char)RallyData_FUN_00406950()) * 7;
}

// GLOBAL: CMR2 0x0052ea68
BYTE g_unk0x0052ea68[11];
// Sorted distinct values (9), the 8 per-slot values and the count: one block,
// because 0x4081d0 can append past the 9 sorted entries as the original does.
// GLOBAL: CMR2 0x0052ea74
int g_unk0x0052ea74Block[18];
#define g_unk0x0052ea74 ((BYTE *)g_unk0x0052ea74Block)
#define g_unk0x0052ea98 (g_unk0x0052ea74Block + 9)
#define g_unk0x0052eab8 (g_unk0x0052ea74Block[17])

// FUNCTION: CMR2 0x00408300
BYTE RallyData_FUN_00408300(void)
{
    unsigned int i;
    BYTE result;

    for (i = 0; i < 0xb; i++) {
        if ((g_unk0x0052ea68[i] & 1) == 0)
            continue;
        if ((g_unk0x0052ea68[i] & 2) != 0 && !CGameInfo::FUN_00406410(0xd))
            continue;
        if ((g_unk0x0052ea68[i] & 4) == 0)
            break;
    }
    result = ((g_selectedRallyData >> 5) & 0x1f) == i;
    return result;
}

// FUNCTION: CMR2 0x00408390
void RallyData_FUN_00408390(void)
{
    unsigned int i;

    for (i = 0; i < 0xb; i++) {
        if ((g_unk0x0052ea68[i] & 1) == 0)
            continue;
        if ((g_unk0x0052ea68[i] & 2) != 0 && !CGameInfo::FUN_00406410(0xd))
            continue;
        if ((g_unk0x0052ea68[i] & 4) == 0) {
            RallyData_FUN_004068e0((BYTE)i);
            return;
        }
    }
}

// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408340
BYTE RallyData_FUN_00408340(void)
{
    unsigned int i;
    BYTE *p;

    i = ((g_selectedRallyData >> 5) & 0x1f) + 1;
    if (i >= 0xb)
        return 1;
    p = &g_unk0x0052ea68[i];
    do {
        if ((*p & 1) != 0 &&
            ((*p & 2) == 0 || CGameInfo::FUN_00406410(0xd)) &&
            (*p & 4) == 0)
            return 0;
        p++;
    } while (p < &g_unk0x0052ea68[0xb]);
    return 1;
}





extern int g_unk0x00531650;
extern int g_unk0x00531654[4];
extern BYTE *g_unk0x00531764;
void FUN_004ec000(void);
void FUN_004ec260(int player);

// Resets the four players' records in the save block (all cleared, then the
// default flags, setup values and car position of each one).
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004eadb0
void FUN_004eadb0(void)
{
    int i;

    memset(g_saveData + 0x628, 0, 0x650 * 4);
    memset(g_unk0x00531350, 0, 0xc0 * 4);
    g_unk0x00531650 = 0;
    for (i = 0; i < 4; i++) {
        BYTE *pPlayer = g_saveData + 0x63c + i * 0x650;
        *(int *)(pPlayer + 0x44) = 4;
        g_saveData[0x620 + i] = 0;
        *(unsigned int *)pPlayer = (*(unsigned int *)pPlayer & 0xffe1f17e) | 0x1017e;
        g_unk0x00531654[i] = 0;
        *(unsigned int *)(pPlayer + 0x40) |= 0x20;
        *(int *)(pPlayer + 0x38) = 0xf11;
        FUN_004ec260(i);
    }
    FUN_004ec000();
    g_unk0x00531764 = NULL;
}


// Stores a driver (FUN_00405d80()==4) or category (otherwise) name into the
// championship save data, marking the record as edited and randomising the
// two packed counters of the category record.
// FUNCTION: CMR2 0x004eae90
void FUN_004eae90(unsigned int slot, char *pName)
{
    unsigned int category;

    slot &= 0xff;
    RallyData_ValidateIndex(slot);
    if (CGameInfo::FUN_00405d80() == 4) {
        strcpy((char *)(g_unk0x0052f3e0 + slot * 0xc4), pName);
        g_unk0x0052f3ec[slot * 0xc4] |= 2;
        return;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + slot * 0x30) >> 0x12) & 0xf;
    if (category == 0xf)
        return;
    strcpy((char *)(g_unk0x0052fa18 + category * 0x650), pName);
    g_saveData[0x620 + category] = 1;
    *(unsigned int *)(g_unk0x0052fa18 + category * 0x650 + 4) =
        (rand() & 0xf) << 0xc |
        (*(unsigned int *)(g_unk0x0052fa18 + category * 0x650 + 4) & 0xffff0fffU);
    *(unsigned int *)(g_unk0x0052fa18 + category * 0x650 + 4) =
        (rand() & 0x3f) << 0x16 |
        (*(unsigned int *)(g_unk0x0052fa18 + category * 0x650 + 4) & 0xf03fffffU);
}

// Copies the name into the category record of the given record index.
// FUNCTION: CMR2 0x004eaf90
void FUN_004eaf90(BYTE index, char *name)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    strcpy((char *)(g_unk0x0052fa24 + category * 0x650), name);
    // 0x52fa00: per-category "name edited" flags, inside the oversized g_unk0x0052f3e8.
    g_unk0x0052f3e8[0x618 + category] = 1;
}
void FUN_004eaae0(int *pValues, short *pOut1, int *pOut2);
int FUN_004eaca0(void);
void RallyData_FUN_004088a0(BYTE index, int *pValues);

// Resets the settings of record `index` to the defaults of the options.
// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ec210
void FUN_004ec210(int index)
{
    int values[5];

    RallyData_ValidateIndex(index);
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;
    values[4] = 0;
    FUN_004eaae0(values, (short *)&values[3], &values[4]);
    RallyData_FUN_004088a0(index, values);
}

// Marks record `index` (and its category's profile) as in use or not; when
// set, the record starts again from the defaults.
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004eb000
void FUN_004eb000(BYTE index, char set)
{
    unsigned int i;
    unsigned int record;
    unsigned int category;
    unsigned int bit;

    i = index;
    RallyData_ValidateIndex(i);
    record = *(unsigned int *)(g_unk0x00531350 + i * 0x30);
    category = record >> 0x12 & 0xf;
    if (category != 0xf) {
        bit = set & 1;
        record = (record & 0xfbffffff) | bit << 0x1a;
        *(unsigned int *)(g_saveData + 0x63c + category * 0x650) =
            (*(unsigned int *)(g_saveData + 0x63c + category * 0x650) & 0xffdfffff) | bit << 0x15;
        record |= 0x2000;
        *(unsigned int *)(g_unk0x00531350 + i * 0x30) = record;
        if (set != 0) {
            *(unsigned int *)(g_unk0x00531350 + i * 0x30) = record & 0xffffe000;
            *(unsigned int *)(g_unk0x00531350 + i * 0x30 + 4) = 0;
            FUN_004ec210(i);
            *(int *)(g_saveData + 0x680 + i * 0x650) = FUN_004eaca0();
            *(int *)(g_saveData + 0x680 + category * 0x650) = FUN_004eaca0();
        }
    }
}

// Which of the 4 profiles are used by the first 4 records.
#define RALLYDATA_USED_PROFILES(used)                                             \
    for (p = 0; p < 4; p++) {                                                     \
        used[p] = 0;                                                              \
        for (pRecord = g_unk0x00531350; pRecord < g_unk0x00531350 + 4 * 0x30; pRecord += 0x30) { \
            if ((*(unsigned int *)pRecord >> 0x12 & 0xf) == p) {                  \
                used[p] = 1;                                                      \
                break;                                                            \
            }                                                                     \
        }                                                                         \
    }

// Number of saved profiles (named, not hidden) no record is using.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ec020
int FUN_004ec020(void)
{
    char used[4];
    BYTE *pRecord;
    BYTE *pProfile;
    unsigned int p;
    int count;

    count = 0;
    RALLYDATA_USED_PROFILES(used)
    p = 0;
    for (pProfile = g_saveData + 0x63c; pProfile < g_saveData + 0x1f7c; pProfile += 0x650, p++) {
        if (used[p] == 0 && pProfile[-4] != 0 && (*(unsigned int *)pProfile & 0x200000) == 0)
            count++;
    }
    return count;
}

// Index of the n-th free saved profile, -1 if none.
// FUNCTION: CMR2 0x004ec090
int FUN_004ec090(int n)
{
    char used[4];
    BYTE *pRecord;
    BYTE *pProfile;
    unsigned int p;
    int count;
    int i;

    count = 0;
    RALLYDATA_USED_PROFILES(used)
    i = 0;
    for (pProfile = g_saveData + 0x63c; pProfile < g_saveData + 0x1f7c; pProfile += 0x650, i++) {
        if (used[i] == 0 && pProfile[-4] != 0 && (*(unsigned int *)pProfile & 0x200000) == 0) {
            if (count == n)
                return i;
            count++;
        }
    }
    return -1;
}

// Name of the n-th free saved profile, NULL if none.
// FUNCTION: CMR2 0x004ec110
BYTE *FUN_004ec110(int n)
{
    char used[4];
    BYTE *pRecord;
    BYTE *pProfile;
    unsigned int p;
    int count;
    int i;

    count = 0;
    RALLYDATA_USED_PROFILES(used)
    i = 0;
    for (pProfile = g_saveData + 0x63c; pProfile < g_saveData + 0x1f7c; pProfile += 0x650, i++) {
        if (used[i] == 0 && pProfile[-4] != 0 && (*(unsigned int *)pProfile & 0x200000) == 0) {
            if (count == n)
                return g_saveData + 0x638 + i * 0x650;
            count++;
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x004ebfd0
void FUN_004ebfd0(int index)
{
    unsigned int value = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    value = (value & 0xffffdfffU) | 0x3c0000;
    *(unsigned int *)(g_unk0x00531350 + index * 0x30) = value;
}

// FUNCTION: CMR2 0x004ec000
void FUN_004ec000(void)
{
    int i = 0;
    do {
        FUN_004ebfd0(i);
        i++;
    } while (i < 16);
}

// GLOBAL: CMR2 0x0052ea60
int g_unk0x0052ea60;
// GLOBAL: CMR2 0x0052ea64
int g_unk0x0052ea64;

unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00407e70(void);
unsigned int RallyData_FUN_00406950(void);

// Setting pair of a driver; in the rally modes it comes from the current
// championship position (both values the same).
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00407520
int *FUN_00407520(int index)
{
    if ((BYTE)RallyData_GetFlag24()) {
        if ((BYTE)RallyData_FUN_00407e70())
            g_unk0x0052ea64 = *(int *)((BYTE *)&g_knockout + 0xb8 +
                                       (((RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)) * 3 +
                                        (CGameInfo::FUN_00405d90() & 0xff)) * 4);
        else
            g_unk0x0052ea64 = *(int *)((BYTE *)&g_knockout + 0xb8 +
                                       ((RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)) * 0xc);
        g_unk0x0052ea60 = g_unk0x0052ea64;
        return &g_unk0x0052ea60;
    }
    return g_unk0x0052f100[index];
}

// FUNCTION: CMR2 0x004075b0
int *RallyData_FUN_004075b0(int index)
{
    return &g_unk0x0052f1f0[index];
}

// FUNCTION: CMR2 0x004075c0
int *RallyData_FUN_004075c0(int index)
{
    return &g_unk0x0052f240[index];
}

// FUNCTION: CMR2 0x004075d0
BYTE *RallyData_FUN_004075d0(int index)
{
    return &g_unk0x0052f294[index];
}

// FUNCTION: CMR2 0x004075e0
int *RallyData_FUN_004075e0(int index)
{
    return &g_unk0x0052f1a0[index];
}

// FUNCTION: CMR2 0x004075f0
unsigned int *RallyData_FUN_004075f0(void)
{
    return g_unk0x0051627c[g_selectedRallyData & 0x1f];
}

// GLOBAL: CMR2 0x0052f0e0
BYTE g_unk0x0052f0e0[4][7];

// FUNCTION: CMR2 0x00407630
BYTE *RallyData_FUN_00407630(int index)
{
    return g_unk0x0052f0e0[index];
}

// GLOBAL: CMR2 0x00516188
BYTE g_stageScoreScale[216] = {
    0x06, 0x09, 0x0c, 0x0c, 0x0e, 0x10, 0x10, 0x11, 0x08, 0x08, 0x0c, 0x0d, 0x0f, 0x10, 0x15, 0x15, 0x08, 0x0a, 0x0c, 0x0d, 0x0f, 0x0f, 0x00, 0x00,
    0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x12, 0x12, 0x07, 0x07, 0x09, 0x0b, 0x11, 0x11, 0x00, 0x00,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x11, 0x11, 0x08, 0x09, 0x0e, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x06, 0x09, 0x0f, 0x0f, 0x12, 0x12, 0x00, 0x00,
    0x09, 0x09, 0x0d, 0x0d, 0x0e, 0x0f, 0x13, 0x13, 0x0b, 0x0b, 0x0c, 0x0e, 0x0f, 0x10, 0x15, 0x15, 0x08, 0x09, 0x0c, 0x0d, 0x12, 0x12, 0x00, 0x00,
    0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x10, 0x11, 0x13, 0x06, 0x09, 0x0d, 0x0d, 0x09, 0x09, 0x00, 0x00,
    0x06, 0x09, 0x0c, 0x0c, 0x0e, 0x10, 0x10, 0x11, 0x08, 0x08, 0x0c, 0x0d, 0x0f, 0x10, 0x12, 0x13, 0x08, 0x0a, 0x15, 0x15, 0x0a, 0x0a, 0x00, 0x00,
    0x07, 0x09, 0x0d, 0x0f, 0x0f, 0x0f, 0x13, 0x13, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x12, 0x12, 0x07, 0x07, 0x09, 0x0b, 0x10, 0x10, 0x00, 0x00,
    0x08, 0x0a, 0x0b, 0x0d, 0x10, 0x10, 0x12, 0x13, 0x06, 0x08, 0x0c, 0x0c, 0x0d, 0x0d, 0x15, 0x15, 0x06, 0x07, 0x0c, 0x0d, 0x12, 0x12, 0x00, 0x00,
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
};
// Score scale of the rally modes by championship, rally and difficulty (3 x 3 x 3).
// GLOBAL: CMR2 0x00516260
BYTE g_rallyScoreScale[27] = {
    0x0a, 0x0a, 0x0a, 0x0c, 0x0c, 0x0c, 0x10, 0x10, 0x10,
    0x0a, 0x0a, 0x0a, 0x0c, 0x0c, 0x0c, 0x10, 0x10, 0x10,
    0x0a, 0x0a, 0x0a, 0x0c, 0x0c, 0x0c, 0x10, 0x10, 0x10,
};

unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00406950(void);
unsigned int RallyData_FUN_00407e70(void);
int FUN_00407270(void);

#define STAGE_SCORE_SCALE(variant)                                                                  \
    do {                                                                                            \
        unsigned int championship;                                                                  \
        unsigned int rally;                                                                         \
        unsigned int difficulty;                                                                    \
        if (!(BYTE)RallyData_GetFlag24()) {                                                         \
            if ((BYTE)FUN_00407270())                                                               \
                return 2000;                                                                        \
            return g_stageScoreScale[(((g_selectedRallyData >> 5) & 0x1f) + (g_selectedRallyData & 0x1f) * 12) * 2 + \
                                     (variant)] * 100;                                              \
        }                                                                                           \
        if ((BYTE)RallyData_FUN_00407e70()) {                                                       \
            championship = RallyData_FUN_00406940();                                                \
            rally = RallyData_FUN_00406950();                                                       \
            difficulty = CGameInfo::FUN_00405d90();                                                 \
            return g_rallyScoreScale[(difficulty & 0xff) + ((championship & 0xff) * 3 + (rally & 0xff)) * 3] * 100; \
        }                                                                                           \
        championship = RallyData_FUN_00406940();                                                    \
        rally = RallyData_FUN_00406950();                                                           \
        return g_rallyScoreScale[((championship & 0xff) * 3 + (rally & 0xff)) * 3] * 100;           \
    } while (0)

// Score scale of the current stage (first column).
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00407650
int FUN_00407650(void)
{
    STAGE_SCORE_SCALE(0);
}

// Score scale of the current stage (second column).
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00407710
int FUN_00407710(void)
{
    STAGE_SCORE_SCALE(1);
}

// FUNCTION: CMR2 0x004077d0
int RallyData_FUN_004077d0(int row, int column, int variant)
{
    return (unsigned int)g_stageScoreScale[(column + row * 12) * 2 + variant] * 100;
}

// Stores a driver's car (driver record, or category record when allowed).
// FUNCTION: CMR2 0x00408760
void RallyData_FUN_00408760(BYTE index, int value)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4) {
        CGameInfo::FUN_00405fd0(value);
        *(int *)(g_unk0x0052f3e0 + 4 + index * 0xc4) = value;
        g_unk0x0052f3e8[4 + index * 0xc4] |= 2;
        return;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf) {
        if ((*(unsigned int *)(g_unk0x0052f3e8 + 0x634 + category * 0x650) & 0x200000) != 0)
            CGameInfo::FUN_00405fd0(value);
        *(int *)(g_unk0x0052f3e8 + 0x678 + category * 0x650) = value;
        g_unk0x0052f3e8[0x618 + category] = 1;
    }
}

// Car of a driver: from the driver record in championship mode, else from its category.
// FUNCTION: CMR2 0x00408800
int RallyData_FUN_00408800(BYTE index)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4)
        return *(int *)(g_unk0x0052f3e0 + 4 + index * 0xc4);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return *(int *)(g_unk0x0052f3e8 + 0x678 + category * 0x650);
    return 0;
}

// FUNCTION: CMR2 0x00408860
BYTE *RallyData_FUN_00408860(int index)
{
    unsigned int category;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052f3e8 + category * 0x650 + 0x620;
    return NULL;
}

void RallyData_MarkTyresChanged(int index);

// Stores a driver's 20-byte tyre choice (driver record or category record).
// FUNCTION: CMR2 0x004088a0
void RallyData_FUN_004088a0(BYTE index, int *pValues)
{
    unsigned int category;

    if (CGameInfo::FUN_00405d80() == 4) {
        memcpy(g_unk0x0052f3e8 + 0xa8 + index * 0xc4, pValues, 5 * sizeof(int));
        g_unk0x0052f3e8[4 + index * 0xc4] |= 2;
        return;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf) {
        memcpy(g_unk0x0052f3e8 + 0xc54 + category * 0x650, pValues, 5 * sizeof(int));
        RallyData_MarkTyresChanged(index);
    }
}

// FUNCTION: CMR2 0x00408930
BYTE *RallyData_FUN_00408930(BYTE index)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x0052f3e8 + 0xa8 + index * 0xc4;
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052f3e8 + 0xc54 + category * 0x650;
    return NULL;
}

// Stores a driver's byte setting (driver record or category record).
// FUNCTION: CMR2 0x00408990
void RallyData_FUN_00408990(BYTE index, BYTE *pValue)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4) {
        g_unk0x0052f3e8[5 + index * 0xc4] = *pValue;
        return;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        g_unk0x0052f3e8[0xba8 + category * 0x650] = *pValue;
}

// Driver record of a car: its knockout entry, or its team's record.
// FUNCTION: CMR2 0x00408a00
BYTE *RallyData_FUN_00408a00(BYTE index)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x0052f3e8 + 5 + index * 0xc4;
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052f3e8 + 0xba8 + category * 0x650;
    return NULL;
}

// Tyre record of a driver (NULL for the ghost cars of the time trials).
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408a60
BYTE *RallyData_GetTyreRecord(BYTE index)
{
    unsigned int category;

    if ((CGameInfo::FUN_00405e00() != 0 || CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
         CGameInfo::FUN_00405d80() == 7 || RallyDataStageIndex() == 10) &&
        (int)index > (int)(CGameInfo::FUN_00405d70() - 1))
        return NULL;
    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x0052f3e8 + 8 + index * 0xc4;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category == 0xf)
        return NULL;
    return g_unk0x0052f3e8 + 0xbac + category * 0x650;
}

// Category colour of a driver's car: hue (5 bits), shade (4 bits) and value
// byte, or 0x45 each when the driver has no category.
// match 28%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408b10
void RallyData_FUN_00408b10(int index, unsigned int *pHue, unsigned int *pShade, unsigned int *pValue)
{
    unsigned int category;
    unsigned int colour;

    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category == 0xf) {
        if (pHue != NULL)
            *pHue = 0x45;
        if (pShade != NULL)
            *pShade = 0x45;
        if (pValue != NULL)
            *pValue = 0x45;
        return;
    }
    colour = *(unsigned int *)(g_unk0x0052f3e8 + 0x634 + category * 0x650);
    if (pHue != NULL)
        *pHue = (colour >> 16) & 0x1f;
    if (pShade != NULL)
        *pShade = (colour >> 8) & 0xf;
    if (pValue != NULL)
        *pValue = colour & 0xff;
}

// Stores a driver's position (x/z of pPos), heading and value.
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408bd0
void RallyData_FUN_00408bd0(int *pPos, short heading, int value, BYTE index)
{
    int record[5];

    RallyData_ValidateIndex(index);
    record[0] = 0;
    *(short *)&record[3] = heading;
    record[1] = pPos[1];
    record[2] = pPos[2];
    record[4] = value;
    RallyData_FUN_004088a0(index, record);
}

// Reads a driver's stored position, heading and value.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408c20
void RallyData_FUN_00408c20(int *pPos, short *pHeading, int *pValue, int index)
{
    int *p;

    RallyData_ValidateIndex(index);
    p = (int *)RallyData_FUN_00408930(index);
    pPos[0] = p[0];
    pPos[1] = p[1];
    pPos[2] = p[2];
    *pHeading = *(short *)(RallyData_FUN_00408930(index) + 0xc);
    *pValue = *(int *)(RallyData_FUN_00408930(index) + 0x10);
}

// FUNCTION: CMR2 0x00408c70
BYTE *RallyData_FUN_00408c70(int index)
{
    unsigned int category;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052fa5c + category * 0x650 + 0xc;
    return NULL;
}

// Marks the tyre record of a driver as changed.
// FUNCTION: CMR2 0x00408d00
void RallyData_MarkTyresChanged(int index)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4) {
        g_unk0x0052f3e8[4 + index * 0xc4] |= 2;
        return;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        g_unk0x0052f3e8[0x618 + category] = 1;
}

// FUNCTION: CMR2 0x00408d60
BYTE *RallyData_FUN_00408d60(int index)
{
    return g_unk0x00531350 + index * 0x30;
}

// Track condition tables of the stage setup (4 groups of 9 entries): the value
// of a group is interpolated between the dry and the wet entry of the current
// rally by the fade of its track index.
// GLOBAL: CMR2 0x00516974
int g_unk0x00516974[9] = {
    0x50000, 0xf0000, 0x70000, -0xa0000, 0xf0000, 0x140000, 0xa0000, 0x70000, 0x70000
};
// GLOBAL: CMR2 0x00516998
int g_unk0x00516998[9] = {
    0xf0000, 0x140000, 0x140000, 0xf0000, 0xf0000, 0x140000, 0xf0000, 0xa0000, 0x140000
};
// GLOBAL: CMR2 0x005169bc
int g_unk0x005169bc[9] = {
    -0x50000, 0x50000, 0x0, -0xf0000, 0x50000, 0xa0000, 0x0, -0x50000, 0x50000
};
// GLOBAL: CMR2 0x005169e0
int g_unk0x005169e0[9] = {
    0xa0000, 0xa0000, 0xa0000, 0xf0000, 0xa0000, 0xa0000, 0xa0000, 0xc0000, 0xa0000
};

extern float g_oneOverRandMax;

// Draws the wet/dry share of every stage of the current rally: the flag of a
// stage is set when its track value passes 80 and the share of the range above
// 80 is stored per stage.
// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d820
void FUN_0040d820(void)
{
    int index;
    int *pFlag;
    int *pOther;
    int *pPairs;
    int i;
    int value;
    int lo;
    int hi;
    int count;
    int above;
    int below;
    int total;
    unsigned int counts[2];
    unsigned int *pCount;
    unsigned short track;

    index = 0;
    pFlag = g_unk0x0052f1f0;
    pOther = g_unk0x0052f240;
    pPairs = &g_unk0x0052f100[0][1];
    do {
        *pFlag = 0;
        *pOther = 0;
        if (pPairs[-1] == 2 && pPairs[0] == 2) {
            if (((BYTE)RallyDataCountryIndex() == 6 &&
                 (pPairs == &g_unk0x0052f100[2][1] || pPairs == &g_unk0x0052f100[5][1] ||
                  pPairs == &g_unk0x0052f100[7][1])) ||
                ((BYTE)RallyDataCountryIndex() == 7 && (index / 4 == 0 || index / 4 == 2))) {
                track = (unsigned short)RallyData_FUN_004077d0(g_selectedRallyData & 0x1f, index, 0);
                if (track < 0x834) {
                    value = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
                    value = FixMulShift32(value, 0x640000);
                    if (value > 0x50)
                        *pFlag = 1;
                    lo = value - rand() % 0x14 - 1;
                    hi = rand() % 0x14 + value + 1;
                    if (lo < 0)
                        lo = 0;
                    else if (hi > 100)
                        hi = 100;
                    count = 0;
                    above = 0;
                    below = 0;
                    counts[1] = 0;
                    counts[0] = 0;
                    if (lo < hi) {
                        count = hi - lo;
                        for (i = lo; i < hi; i++) {
                            if (i > 0x50)
                                above++;
                            else
                                below++;
                        }
                        counts[1] = below;
                        counts[0] = above;
                    }
                    total = count << 16;
                    pCount = counts;
                    i = 2;
                    do {
                        *pCount = FixMulShift32(0x640000, FixDiv(*pCount << 16, total));
                        pCount++;
                    } while (--i);
                    *pOther = (counts[0] != 0);
                }
            }
        }
        pFlag++;
        pOther++;
        pPairs += 2;
        index++;
    } while ((int)pPairs < (int)&g_unk0x0052f100[11][1]);
}

// Recomputes the grip byte of the four stages of a rally group, interpolating
// between the dry and the wet value of the group by the rally's track fade.
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d9e0
void FUN_0040d9e0(int group)
{
    int index;
    int *pPairs;
    int k;
    int m;
    int n;
    int fade;
    int base;
    int track;

    index = group << 2;
    pPairs = &g_unk0x0052f100[index][1];
    k = 4;
    do {
        m = pPairs[-1];
        if (m >= 6)
            m += -3;
        if (pPairs[0] >= 6)
            m = m + -3 + pPairs[0];
        else
            m = m + pPairs[0];
        base = FixDiv(m << 16, 0xa0000);
        if (base < 0)
            base = 0;
        else if (base > 0x10000)
            base = 0x10000;
        fade = 0x10000 - base;
        m = g_unk0x00516974[(BYTE)RallyDataCountryIndex()] +
            FixMul(g_unk0x00516998[(BYTE)RallyDataCountryIndex()], fade);
        n = g_unk0x005169bc[(BYTE)RallyDataCountryIndex()] +
            FixMul(g_unk0x005169e0[(BYTE)RallyDataCountryIndex()], fade);
        track = (unsigned short)RallyData_FUN_004077d0((BYTE)RallyDataCountryIndex(), index, 0);
        if (track <= 0x1f4 || track >= 0x834) {
            g_unk0x0052f294[index] = (BYTE)(n >> 16);
        } else if (track < 0x4b0) {
            g_unk0x0052f294[index] =
                (BYTE)((FixMul(m - n, FixDiv((track / 100 - 5) << 16, 0x70000)) + n) >> 16);
        } else if (track > 0x640) {
            g_unk0x0052f294[index] =
                (BYTE)((FixMul(n - m, FixDiv((track / 100 - 0x10) << 16, 0x50000)) + m) >> 16);
        } else {
            g_unk0x0052f294[index] = (BYTE)(m >> 16);
        }
        index++;
        pPairs += 2;
    } while (--k);
}

// FUNCTION: CMR2 0x0040df30
void RallyData_FUN_0040df30(void)
{
    BYTE zero = 0;
    int i = 0;
    do {
        BYTE *p = RallyData_FUN_00407630(i);
        i++;
        p[4] = zero;
        p[5] = zero;
        p[1] = zero;
        p[3] = zero;
        p[2] = zero;
        p[0] = zero;
        p[6] = zero;
    } while (i < 4);
}

// Grid row of the driver for the given position flag.
// FUNCTION: CMR2 0x00407150
BYTE FUN_00407150(BYTE param1, char param2)
{
    if (param2 == 0)
        return (bool)(param1 % 2) + 4;
    if (param2 == 1)
        return (bool)(param1 % 2) + 8;
    return (bool)(param1 % 2) + 10;
}

BYTE FUN_00407150(BYTE param1, char param2);

// Stage group of a mode: in network games the last available one.
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004071c0
BYTE FUN_004071c0(BYTE flags, char mode)
{
    BYTE result = 0;
    int last = 0;
    int count;
    int i;

    if (CGameInfo::FUN_00405e00()) {
        count = FUN_00407150(flags, 2);
        for (i = 0; i < count; i++) {
            if ((g_unk0x0052ea68[i] & 1) != 0 && ((g_unk0x0052ea68[i] & 2) == 0 || CGameInfo::FUN_00406410(0xd)) &&
                (g_unk0x0052ea68[i] & 4) == 0)
                last = i;
            result = (BYTE)last;
        }
        return result;
    }
    if (flags & 1)
        return 10;
    if (mode == 0)
        return 3;
    if (mode == 1)
        return 7;
    if (mode == 2)
        return 9;
    return 0;
}

// Whether the championship has just finished its last rally (rally 8, stage 11).
// FUNCTION: CMR2 0x00407270
int FUN_00407270(void)
{
    BYTE mode;
    BYTE rallyClass;

    mode = CGameInfo::FUN_00405d80();
    rallyClass = CGameInfo::FUN_00405d90();
    FUN_00407150(g_selectedRallyData & 0x1f, rallyClass);
    if (mode == 0 && (g_selectedRallyData & 0x1f) == 8 && (g_selectedRallyData & 0x3e0) == 0x160)
        return 1;
    return 0;
}

void RallyData_UpdateFlags(void);

// Advances g_selectedRallyData to the next stage (bits 5-9) and, at the end of
// a rally, to the next rally (bits 0-4). Returns 0 when the championship is over.
// FUNCTION: CMR2 0x004072d0
BYTE FUN_004072d0(void)
{
    BYTE mode;
    BYTE count;
    unsigned int stage;
    int i;

    mode = CGameInfo::FUN_00405d80();
    count = CGameInfo::FUN_00405d90();
    if (CGameInfo::FUN_00405e00()) {
        count = FUN_00407150(g_selectedRallyData & 0x1f, 2);
        g_selectedRallyData ^= (((g_selectedRallyData & 0xffffffe0) + 0x20) ^ g_selectedRallyData) & 0x3e0;
        for (stage = (g_selectedRallyData >> 5) & 0x1f; stage < count; stage = (g_selectedRallyData >> 5) & 0x1f) {
            if ((g_unk0x0052ea68[stage] & 1) &&
                (!(g_unk0x0052ea68[stage] & 2) || CGameInfo::FUN_00406410(0xd)) &&
                !(g_unk0x0052ea68[(g_selectedRallyData >> 5) & 0x1f] & 4))
                goto done;
            g_selectedRallyData ^= (((g_selectedRallyData & 0xffffffe0) + 0x20) ^ g_selectedRallyData) & 0x3e0;
        }
        return 0;
    }
    count = FUN_00407150(g_selectedRallyData & 0x1f, count);
    if (mode != 0 && mode != 1)
        return 0;
    g_selectedRallyData ^= (((g_selectedRallyData & 0xffffffe0) + 0x20) ^ g_selectedRallyData) & 0x3e0;
    stage = (g_selectedRallyData >> 5) & 0x1f;
    if (stage != count && (g_selectedRallyData & 0x3e0) <= 0x140) {
        if (stage + 1 == count && (g_selectedRallyData & 1)) {
            g_selectedRallyData = (g_selectedRallyData & 0xfffffd5f) | 0x140;
            RallyData_UpdateFlags();
            return 1;
        }
        goto done;
    }
    if (mode == 1)
        return 0;
    g_selectedRallyData ^= ((g_selectedRallyData + 1) ^ g_selectedRallyData) & 0x1f;
    if ((g_selectedRallyData & 0x1f) == 8) {
        for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
            if (RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i)) < 3)
                goto done;
        }
        return 0;
    }
    if ((g_selectedRallyData & 0x1f) == 9)
        return 0;
    g_selectedRallyData &= 0xfffffc1f;
done:
    RallyData_UpdateFlags();
    return 1;
}


// Returns the record of the category that the index belongs to, or NULL when
// the record has no category.
// FUNCTION: CMR2 0x00408470
void *FUN_00408470(unsigned int param1)
{
    unsigned int category;

    category = (*(unsigned int *)(g_unk0x00531350 + (param1 & 0xff) * 0x30) >> 0x12) & 0xf;
    RallyData_ValidateIndex(param1 & 0xff);
    if (category != 0xf)
        return g_unk0x0052fa24 + category * 0x650;
    return NULL;
}

// Returns 1 when the record can be used (and 0 when it is full), unless the
// record is one of the "always usable" ones whose flag lacks the category.
// FUNCTION: CMR2 0x004085a0
BYTE FUN_004085a0(BYTE param1)
{
    RallyData_ValidateIndex(param1);
    if ((*(unsigned int *)(g_unk0x00531350 + param1 * 0x30) & 0x3c0000) != 0x3c0000) {
        if (CGameInfo::FUN_00405d80() == 4)
            return 1;
        return (*(unsigned int *)(g_unk0x00531350 + param1 * 0x30) >> 0x1a) & 1;
    }
    return 0;
}

// Stores a driver's 5/6-bit setting in the driver or category record.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408600
void RallyData_FUN_00408600(BYTE index, BYTE value)
{
    unsigned int record;
    unsigned int category;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4) {
        *(unsigned int *)(g_unk0x0052f3e8 + index * 0xc4) = value;
        g_unk0x0052f3e8[4 + index * 0xc4] |= 2;
        return;
    }
    record = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    category = (record >> 0x12) & 0xf;
    if (category != 0xf) {
        g_unk0x0052f3e8[0x618 + category] = 1;
        *(unsigned int *)(g_unk0x00531350 + index * 0x30) = ((value ^ record) & 0x3f) ^ record;
        *(unsigned int *)(g_unk0x0052f3e8 + 0x674 + category * 0x650) ^=
            (*(unsigned int *)(g_unk0x0052f3e8 + 0x674 + category * 0x650) ^ value) & 0x1f;
    }
}

// FUNCTION: CMR2 0x004086b0
BYTE RallyData_FUN_004086b0(BYTE index)
{
    if (CGameInfo::FUN_00405d80() == 4)
        return *(BYTE *)((int *)g_unk0x0052f3e8 + index * 49);
    return *(int *)((char *)g_unk0x00531350 + index * 48) & 0x3f;
}

// Returns bit 0 of the record flag, or bit 5 of the category flag when the
// record belongs to a category.
// FUNCTION: CMR2 0x004086f0
BYTE FUN_004086f0(BYTE param1)
{
    unsigned int category;
    BYTE flag;

    param1 = param1 & 0xff;
    RallyData_ValidateIndex(param1);
    if (CGameInfo::FUN_00405d80() == 4) {
        flag = *(BYTE *)(g_unk0x0052f3e8 + param1 * 196 + 4);
        flag &= 1;
        return flag;
    }
    category = (*(unsigned int *)(g_unk0x00531350 + param1 * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return (*(unsigned int *)(g_unk0x0052fa5c + category * 0x650) >> 5) & 1;
    return 1;
}

// GLOBAL: CMR2 0x00536ecc
int g_unk0x00536ecc;
// GLOBAL: CMR2 0x00537060
int g_unk0x00537060;

// Plays the looping sound of the stage that is not the one already playing.
// FUNCTION: CMR2 0x00411ab0
void FUN_00411ab0(BYTE param1, int param2)
{
    if (CGameInfo::FUN_00406320() != 0)
        return;
    if (FUN_0041f3d0(param1) != 0)
        return;
    if (param2 != 0) {
        FUN_004b7790(g_unk0x00536ecc, FUN_00418570() / 2, 0x5622, 0, 0, 0);
        return;
    }
    FUN_004b7790(g_unk0x00537060, FUN_00418570() / 2, 0x2b11, 0, 0, 0);
}

int FUN_004583a0(void);
int FUN_004583b0(int index);

// Finds the checkpoint before distance (whole units, plus percent/100) and
// the 16.16 fraction of the way to the next one.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00411e40
void FUN_00411e40(int *pOut, int distance, int percent)
{
    int found = -1;
    int i;
    int next;
    int current;

    for (i = FUN_004583a0() - 1; i >= 0; i--) {
        if ((FUN_004583b0(i) >> 16) <= distance) {
            found = i;
            break;
        }
    }
    if (found != -1 && found != FUN_004583a0() - 1) {
        next = FUN_004583b0(found + 1);
        current = FUN_004583b0(found);
        pOut[0] = found;
        pOut[1] = FixDiv(distance * 0x10000 - current + (percent << 16) / 100, next - current);
        return;
    }
    pOut[0] = FUN_004583a0() - 1;
    pOut[1] = 0;
}

// Copies the name of the driver with the given id (a human player's name, or
// the AI driver name) into CFrontend::m_stringDest.
// FUNCTION: CMR2 0x004125a0
void FUN_004125a0(int id)
{
    char *pName;
    int i;

    for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
        if (id == StageTiming_GetDriverSlot(i)) {
            pName = (char *)RallyData_GetRecord(i);
            goto done;
        }
    }
    pName = CAIHelper::GetNameForID(id);
done:
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, pName);
}

extern char g_stageNumberFormat[];
extern char g_nameSpaceFormat[];
extern char g_standingsRowFormat[];
extern double g_minus65536;
extern BYTE g_gapTextColour[4];

// GLOBAL: CMR2 0x005170bc
unsigned int g_stageHudPanelColour = 0x96dcbebc;
// GLOBAL: CMR2 0x005170c4
unsigned int g_stageHudTextColour = 0xffffffff;
// GLOBAL: CMR2 0x005170e8
unsigned int g_unk0x005170e8 = 0xffffffff;
// GLOBAL: CMR2 0x00517dc0
char g_positionFormat[] = "%d / %d  %d";
// GLOBAL: CMR2 0x00517dcc
char g_stageSlashFormat[] = " / ";

BYTE *FUN_00464b10(int view);
int FUN_004483b0(int index);
int FUN_00448110(void);
int RallyData_FUN_00421370(BYTE *p);
int RallyData_FUN_00421420(void);
extern int g_unk0x00536c88[2];
extern int g_unk0x00536c00[2];
extern int g_unk0x00536c28[2];
extern int g_unk0x00536e88[2];
void FUN_00415bc0(int, int);
BYTE FUN_004582b0(int index);
extern BYTE g_gapTextColour[4];
BYTE FUN_00448ca0(void);

// Draws the time-gap text of a car while the stage is running, for the two
// rally modes that the input mode selects.
// FUNCTION: CMR2 0x00412970
void FUN_00412970(int car, short *pRect)
{
    if (FUN_00448ca0() != 0) {
        switch (RallyData_FUN_004082b0()) {
        case 1:
            if (FUN_004483b0(car) == 0)
                FUN_00417e70(CFrontend::GetTextString(0xbe), (int *)g_gapTextColour, car, 0, -1,
                             -1);
            break;
        case 2:
            if (FUN_004483b0(car) == 0)
                FUN_00417e70(CFrontend::GetTextString(0xbe), (int *)g_gapTextColour, car, 0, -1,
                             -1);
            break;
        }
    }
}

// Temporary scaffolding for the RallyData helpers FUN_004125f0 dispatches to:
// every one is a real function of the reference exe (empty body, stdcall
// argument count from its ret N) so that its call sites are in place. Delete
// each entry as its implementation lands (0x415f50 belongs to StageUI.cpp and
// 0x417e70 to Race.cpp).
// STUB: CMR2 0x004137e0
void FUN_004137e0(int car, short *pRect) { }
// STUB: CMR2 0x00413fe0
void FUN_00413fe0(int car, short *pRect) { }
// STUB: CMR2 0x00414ed0
void FUN_00414ed0(int car, short *pRect) { }
// STUB: CMR2 0x00415480
void FUN_00415480(short *pRect) { }
// STUB: CMR2 0x00415f50
void FUN_00415f50(int car, short *pRect) { }

// Draws a car's on-stage HUD: the position/points line plus the per-car
// action icons, then dispatches the current input to the matching handler.
// FUNCTION: CMR2 0x004125f0
void FUN_004125f0(int car, short *pRect)
{
    BYTE colour[4] = { 0xff, 0xff, 0xff, 0xff };
    short rect[4];
    int *pView;

    if ((BYTE)CGameInfo::FUN_00405d60()) {
        Font_DrawText(0, CInput::FormatString(g_positionFormat,
                          RallyData_FUN_00421370((BYTE *)Car_Get(car)), RallyData_FUN_00421420(),
                          FUN_00448110() / 100),
                      pRect[0] + 10, pRect[3] + pRect[1] - 10, (int *)colour, 0x21);
    }
    pView = (int *)FUN_00464b10(car);
    *(int *)&rect[0] = pView[0];
    *(int *)&rect[2] = pView[1];
    if ((BYTE)RallyData_FUN_004082e0()) {
        FUN_00412970(car, pRect);
    } else if (RallyData_FUN_00411880()) {
        if (((BYTE)RallyData_GetFlag24() || (BYTE)RallyData_GetFlag25()) && FUN_004582b0(car) &&
            FUN_004483b0(car) == 0) {
            FUN_00417e70(CFrontend::GetTextString(0xbe), (int *)g_gapTextColour, car, 0, -1, -1);
        }
    }
    if (g_unk0x00536c88[car] != 0) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xbd));
        FUN_00417e70(CFrontend::m_stringDest, (int *)&g_unk0x005170e8, car, 1, -1, -1);
    }
    if (g_unk0x00536c00[car] != 0) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x30));
        FUN_00417e70(CFrontend::m_stringDest, (int *)&g_unk0x005170e8, car, 1, -1, -1);
    }
    if (g_unk0x00536e88[car] != 0) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x6a));
        FUN_00417e70(CFrontend::m_stringDest, (int *)&g_unk0x005170e8, car, 1, -1, -1);
    }
    if (g_unk0x00536c28[car] != 0) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x83));
        FUN_00417e70(CFrontend::m_stringDest, (int *)&g_unk0x005170e8, car, 1, -1, -1);
    }
    if ((*RallyData_FUN_00408a00(FUN_0041b370() + (char)car) & 4) != 0) {
        if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 5 ||
            CGameInfo::FUN_00405d80() == 6 || CGameInfo::FUN_00405d80() == 3 ||
            CGameInfo::FUN_00405d80() == 0xa || CGameInfo::FUN_00405d80() == 0xc) {
            if (RallyData_FUN_00411880() == 0 || CGameInfo::FUN_00405dc0() != 0) {
                FUN_004147f0(car, rect);
            }
        } else if ((BYTE)RallyData_FUN_00407e90() && !CGameInfo::FUN_00405e00()) {
            FUN_00415bc0(car, (int)rect);
        } else if (CGameInfo::FUN_00405d80() == 2 || CGameInfo::FUN_00405d80() == 0 ||
                   CGameInfo::FUN_00405d80() == 1) {
            FUN_00414ed0(car, rect);
        } else if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 ||
                   CGameInfo::FUN_00405d80() == 0xb) {
            FUN_00415480(rect);
        }
    }
    if ((*RallyData_FUN_00408a00(FUN_0041b370() + (char)car) & 8) != 0) {
        if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
            CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 0xb ||
            CGameInfo::FUN_00405d80() == 0xc || (BYTE)RallyData_GetFlag25()) {
            FUN_004129d0(car, rect);
        } else {
            FUN_00413fe0(car, rect);
        }
    }
    if ((*RallyData_FUN_00408a00(FUN_0041b370() + (char)car) & 0x20) != 0) {
        if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
            CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 0xb ||
            CGameInfo::FUN_00405d80() == 0xc || (BYTE)RallyData_GetFlag25()) {
            FUN_00415f50(car, rect);
        }
        FUN_004137e0(car, rect);
    }
}

// Draws a car's stage gap, stage number and position in the on-stage HUD.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004129d0
void FUN_004129d0(int car, short *pRect)
{
    short rect[4];
    int xShift = 0;
    int yShift = 0;
    int marginX = (int)g_pGraphics->resX * 3 / 640;
    int wideMarginX = (int)g_pGraphics->resX * 44 / 640;
    int topMarginY = (int)g_pGraphics->resY * 3 / 480;
    int mode;
    int stage;
    int count;
    int x;
    int y;
    int i;
    int pixelFixed;
    int dimensionFixed;

    if (RallyData_FUN_00411880()) {
        if (CGameInfo::FUN_00405dc0()) {
            if (car == 1)
                yShift = (int)g_pGraphics->resY / 2;
        } else if (car == 0) {
            xShift = -((int)g_pGraphics->resX / 2);
        }
    }
    rect[0] = (short)(((int)g_pGraphics->resX * 0xab9b >> 16) + xShift);
    rect[1] = (short)(((int)g_pGraphics->resY << 12 >> 16) + yShift);
    rect[2] = (short)(((int)g_pGraphics->resX * 0x4865 >> 16) + 1);
    rect[3] = (short)((int)g_pGraphics->resY * 53 / 480);

    if (FUN_00459390()) {
        if (CGameInfo::GetGameLanguage() == 1 || CGameInfo::GetGameLanguage() == 3 ||
            CGameInfo::GetGameLanguage() == 2)
            rect[3] += (short)((int)g_pGraphics->resY * 24 / 480 * 2);
        else
            rect[3] += (short)((int)g_pGraphics->resY * 24 / 480);
    }
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_stageHudPanelColour, 2);
    if (FUN_00459390()) {
        if (CGameInfo::GetGameLanguage() == 1 || CGameInfo::GetGameLanguage() == 3 ||
            CGameInfo::GetGameLanguage() == 2)
            rect[3] += (short)((int)g_pGraphics->resY * -24 / 480 * 2);
        else
            rect[3] += (short)((int)g_pGraphics->resY * -24 / 480);
    }

    pixelFixed = (int)(__int64)((double)(rect[3] + rect[1] - topMarginY + 1) * CGraphics::m_65536);
    dimensionFixed = (int)(__int64)((double)g_pGraphics->resY * CGraphics::m_65536);
    y = FixDiv(pixelFixed, dimensionFixed);
    pixelFixed = (int)(__int64)((double)(rect[0] + 16 + marginX) * CGraphics::m_65536);
    dimensionFixed = (int)(__int64)((double)g_pGraphics->resX * CGraphics::m_65536);
    x = FixDiv(pixelFixed, dimensionFixed);
    FormatGapToLeader(g_stageSplitData[car].targetTime, 4, 4, x, y,
                      &g_stageHudTextColour, 0x21, NULL, 0);

    stage = FUN_004582d0(car);
    count = RallyData_FUN_00406990() & 0xff;
    mode = 0;
    if (RallyData_FUN_004082e0()) {
        int uiState = RallyData_FUN_004082b0();
        if (uiState == 1)
            mode = 1;
        else if (uiState == 2)
            mode = 2;
    }

    if (mode != 2) {
        if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 12 || mode == 1) {
            if (stage < 0)
                stage = 0;
            sprintf(CFrontend::m_stringDest, g_nameSpaceFormat, CFrontend::GetTextString(0x2f));
            x = rect[0] + marginX;
            y = rect[1] + topMarginY - (int)g_pGraphics->resY * 2 / 480;
            Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 9);
            x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, (stage + 1) % 100);
            Font_DrawText(5, CFrontend::m_stringDest, x, rect[1] + topMarginY,
                          (int *)&g_stageHudTextColour, 9);
        } else if (!RallyData_FUN_00407e90()) {
            if (stage < 0)
                stage = 0;
            if (stage < count)
                stage++;
            sprintf(CFrontend::m_stringDest, g_nameSpaceFormat, CFrontend::GetTextString(0x2f), stage, count);
            x = rect[0] + marginX;
            y = rect[1] + topMarginY - (int)g_pGraphics->resY * 2 / 480;
            Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 9);
            x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, stage % 100);
            Font_DrawText(5, CFrontend::m_stringDest, x, rect[1] + topMarginY,
                          (int *)&g_stageHudTextColour, 9);
            x += Font_GetTextWidth(5, (BYTE *)CFrontend::m_stringDest);
            sprintf(CFrontend::m_stringDest, g_stageSlashFormat);
            Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 9);
            x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, count);
            Font_DrawText(5, CFrontend::m_stringDest, x, rect[1] + topMarginY,
                          (int *)&g_stageHudTextColour, 9);
        }
    } else {
        sprintf(CFrontend::m_stringDest, g_nameSpaceFormat, CFrontend::GetTextString(0xbc));
        x = rect[0] + marginX;
        y = rect[1] + topMarginY - (int)g_pGraphics->resY * 2 / 480;
        Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 9);
        x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_00448c60(car));
        Font_DrawText(5, CFrontend::m_stringDest, x, rect[1] + topMarginY,
                      (int *)&g_stageHudTextColour, 9);
        x += Font_GetTextWidth(5, (BYTE *)CFrontend::m_stringDest);
        sprintf(CFrontend::m_stringDest, g_stageSlashFormat);
        Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 9);
        x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, RallyData_FUN_004082c0());
        Font_DrawText(5, CFrontend::m_stringDest, x, rect[1] + topMarginY,
                      (int *)&g_stageHudTextColour, 9);
    }

    if (CGameInfo::FUN_00405d80() != 7 && CGameInfo::FUN_00405d80() != 12 &&
        (RallyDataStageIndex() != 10 || CGameInfo::FUN_00405d80() != 10)) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x99));
        x = rect[0] + rect[2] - wideMarginX;
        y = rect[1] + topMarginY - (int)g_pGraphics->resY * 2 / 480;
        Font_DrawText(0, CFrontend::m_stringDest, x, y, (int *)&g_stageHudTextColour, 12);
        if (CGameInfo::FUN_00405d80() == 11 ||
            (CGameInfo::FUN_00405e00() && RallyData_GetFlag25())) {
            for (i = 0; i < 8; i++) {
                if (FUN_0040a700(i) == -2)
                    sprintf(CFrontend::m_stringDest, g_stageNumberFormat, i + 1);
            }
        } else {
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_004481c0(car) + 1);
        }
        Font_DrawText(6, CFrontend::m_stringDest, rect[0] + rect[2] - marginX,
                      rect[1] + topMarginY, (int *)&g_stageHudTextColour, 12);
    }
}

BYTE FUN_00448ca0(void);
BYTE FUN_00448c90(void);
int FUN_00448c80(void);
int FUN_00448c70(void);
int FUN_00448c60(int index);
int FUN_004481c0(int car);
BYTE FUN_00458290(int index);
void Sound_SetPan(unsigned int handle, unsigned short pan);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082d0(void);

// GLOBAL: CMR2 0x00536c08
BYTE g_unk0x00536c08[4];
// GLOBAL: CMR2 0x00536c88
int g_unk0x00536c88[2];
// GLOBAL: CMR2 0x00536cac
BYTE g_unk0x00536cac[4];
// GLOBAL: CMR2 0x00536cb0
int g_unk0x00536cb0;
// GLOBAL: CMR2 0x00536fec
int g_unk0x00536fec;
// GLOBAL: CMR2 0x00537050
int g_unk0x00537050;

// Per-frame stage sounds: the start beep once the stage is over, and the
// countdown beeps (rising in pitch over the last five seconds) while racing.
// FUNCTION: CMR2 0x004118b0
void FUN_004118b0(int car)
{
    int elapsed;
    int remaining;
    int handle;

    if (FUN_00448ca0()) {
        if (g_unk0x00537050 == 0) {
            g_unk0x00537050 = 1;
            if (RallyData_FUN_004082b0() == 1)
                FUN_004b7790(g_unk0x00536cb0 + 2, FUN_00418570(), 0xac44, 0, 0, 0);
            else
                FUN_004b7790(g_unk0x00536cb0 + 1, FUN_00418570(), 0xac44, 0, 0, 0);
        }
        return;
    }
    switch (RallyData_FUN_004082b0()) {
    case 1:
        if (FUN_00458290(car)) {
            if (FUN_004481c0(car) == 0)
                FUN_004b7790(g_unk0x00536cb0 + 1, FUN_00418570(), 0xac44, 0, 0, 0);
            else
                FUN_00411ab0(car, 1);
            g_unk0x00536fec = RallyData_FUN_004082d0() - FUN_00448c70() / 100;
        }
        if (FUN_00448c90() && FUN_00448c80() != car) {
            elapsed = FUN_00448c70() / 100;
            remaining = RallyData_FUN_004082d0() - elapsed;
            if (remaining < g_unk0x00536fec) {
                g_unk0x00536fec = remaining;
                if (remaining > 5)
                    elapsed = 0xac44;
                else
                    elapsed = (6 - remaining) * 0xac44 / 0x30 + 0xac44;
                if (remaining != 0) {
                    handle = FUN_004b7790(g_unk0x00536cb0, FUN_00418570(), elapsed, 0, 0, 0);
                    Sound_SetPan(handle, elapsed);
                }
            }
        }
        break;
    case 2:
        if (FUN_00458290(car) && g_unk0x00536cac[car] != (BYTE)FUN_00448c60(car)) {
            g_unk0x00536cac[car] = FUN_00448c60(car);
            g_unk0x00536c88[car] = 1;
            g_unk0x00536c08[car] = 0x7d;
            FUN_004b7790(g_unk0x00536cb0 + 1, FUN_00418570(), 0xac44, 0, 0, 0);
        }
        break;
    }
}

BYTE FUN_00458250(int index);
BYTE FUN_00458270(int index);
int FUN_00458330(int index);
int FUN_00458350(int index);
int FUN_00458370(int index);
int FUN_004481f0(int car, int index);
void FUN_004279d0(int index, int value);
int FUN_0040a3c0(void);
unsigned int FUN_0040a440(void);

// GLOBAL: CMR2 0x00536bfc
int g_unk0x00536bfc;
// GLOBAL: CMR2 0x00536c0c
int g_unk0x00536c0c[2];
// GLOBAL: CMR2 0x00536c14
int g_unk0x00536c14;
// GLOBAL: CMR2 0x00536c20
BYTE g_unk0x00536c20[4];
// GLOBAL: CMR2 0x00536c40
int g_unk0x00536c40;
// Split time colours, 0x28 entries per car.
// GLOBAL: CMR2 0x00536d14
int g_unk0x00536d14[0x3a];
// Reference split times, g_unk0x00536e90[0] doubles as a "no reference" flag.
// GLOBAL: CMR2 0x00536e90
int g_unk0x00536e90[11];
// GLOBAL: CMR2 0x00537064
int g_unk0x00537064;
// GLOBAL: CMR2 0x00537068
int g_unk0x00537068;
// GLOBAL: CMR2 0x0051709c
unsigned int g_unk0x0051709c = 0xffa4fa2e;
// GLOBAL: CMR2 0x005170a0
unsigned int g_unk0x005170a0 = 0xff00008a;
// GLOBAL: CMR2 0x005170e0
unsigned int g_unk0x005170e0[2] = {0x00ffffff, 0x00ffffff};

// Records the time of the split the car just passed, colours it against the
// reference time and updates the car's position at that split.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00413330
void FUN_00413330(int car)
{
    int split;
    int time;
    int delta;
    int reference;

    g_unk0x00536c0c[car] = 0;
    if (!FUN_00458250(car))
        return;
    if ((BYTE)RallyData_GetFlag25())
        split = FUN_00458370(car);
    else
        split = FUN_00458350(car);
    if (FUN_00458290(car)) {
        split = 1;
        g_stageSplitData[car].times[split + 1] = FUN_004481f0(car, FUN_00458330(car));
        g_stageSplitData[car].targetTime = g_unk0x00536c40;
        g_stageSplitData[car].lastSplitTime = FUN_004481f0(car, FUN_00458330(car));
        g_unk0x00536bfc = 0;
    } else if (split != 0) {
        g_stageSplitData[car].times[split + 1] = g_stageSplitData[car].times[0];
        g_stageSplitData[car].targetTime = g_unk0x00536e90[split + 1];
        g_stageSplitData[car].lastSplitTime = g_stageSplitData[car].times[0];
    }
    if (FUN_00458270(car))
        g_unk0x00536bfc = 0;
    if (g_unk0x00536c40 != g_unk0x00537068) {
        time = g_stageSplitData[car].times[split + 1];
        if (time - g_stageSplitData[car].times[split] < g_unk0x00536e90[split + 1] - g_unk0x00536e90[split])
            g_unk0x00536d14[car * 0x28 + split] = g_unk0x0051709c;
        else
            g_unk0x00536d14[car * 0x28 + split] = g_unk0x005170a0;
        if (split != 0) {
            if (time < g_stageSplitData[car].targetTime)
                FUN_00411ab0(car, 1);
            else
                FUN_00411ab0(car, 0);
        }
    }
    g_stageSplitData[car].split = split;
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6 && split != 0) {
        ((BYTE *)&g_unk0x005170e0[car])[3] = 0xff;
        g_unk0x00536c20[car] = 0x4b;
    }
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6 &&
        CGameInfo::FUN_00405d80() != 7 && CGameInfo::FUN_00405d80() != 4) {
        if (StageTiming_FUN_00455ae0()) {
            g_stageSplitData[car].position =
                StageTiming_GetSplitPositionOfDriver(FUN_0041b370() + car, g_stageSplitData[car].split);
            return;
        }
        g_stageSplitData[car].position = 0xf;
    }
}

// Same as FUN_00413330 for the stage start and the time-trial ghost.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00413610
void FUN_00413610(int car)
{
    int time;

    g_unk0x00536e90[0] = 0;
    if (FUN_00458290(car)) {
        g_unk0x00537064 = FUN_00458330(car);
        if (CGameInfo::FUN_00405d80() == 0xc) {
            time = FUN_004481f0(0, 0);
            g_stageSplitData[car].times[0] = time;
            g_stageSplitData[car].times[g_unk0x00537064 + 1] = time;
            g_stageSplitData[car].lastSplitTime = g_stageSplitData[car].times[0];
            g_stageSplitData[car].split = g_unk0x00537064;
            FUN_004279d0(g_unk0x00537064, g_stageSplitData[car].times[0]);
            g_unk0x00536c14 = 1;
        } else {
            if (g_unk0x00537064 > 0 && g_unk0x00537064 < 10) {
                g_stageSplitData[car].times[g_unk0x00537064 + 1] = g_stageSplitData[car].times[0];
                g_stageSplitData[car].lastSplitTime = g_stageSplitData[car].times[0];
                g_stageSplitData[car].split = g_unk0x00537064;
                FUN_004279d0(g_unk0x00537064, g_stageSplitData[car].times[0]);
            }
            g_unk0x00536c14 = 1;
        }
    } else if (g_unk0x00536c14 == 0) {
        return;
    }
    if (CGameInfo::FUN_00405d80() == 0xc) {
        CGameInfo::FUN_0040a420(g_unk0x00537064);
        if (FUN_0040a3c0())
            time = FUN_0040a440();
        else
            time = CGameInfo::FUN_0040a420(g_unk0x00537064);
        g_stageSplitData[car].targetTime = time;
        if (g_stageSplitData[car].lastSplitTime < time) {
            FUN_00411ab0(car, 1);
            g_unk0x00536c14 = 0;
        } else if (time == 0) {
            g_unk0x00536e90[0] = 1;
        } else {
            FUN_00411ab0(car, 0);
            g_unk0x00536c14 = 0;
        }
        if (g_unk0x00537064 == 0 && CGameInfo::FUN_00405d80() != 0xc)
            return;
    } else {
        time = CGameInfo::FUN_0040a420(g_unk0x00537064);
        g_stageSplitData[car].targetTime = time;
        if (g_stageSplitData[car].lastSplitTime < time) {
            FUN_00411ab0(car, 1);
            g_unk0x00536c14 = 0;
        } else if (time == 0) {
            g_unk0x00536e90[0] = 1;
        } else {
            FUN_00411ab0(car, 0);
            g_unk0x00536c14 = 0;
        }
        if (g_unk0x00537064 == 0 && CGameInfo::FUN_00405d80() != 0xc)
            return;
    }
    ((BYTE *)&g_unk0x005170e0[car])[3] = 0xff;
    g_unk0x00536c20[car] = 0x4b;
}

BYTE FUN_00448cd0(int car);
BYTE FUN_004085a0(BYTE param1);
extern BYTE g_itemColour[4];
extern char g_classRowHeaderFormat[];
int RallyData_DrawListItem(int x, int y, char *pText, char last, BYTE alpha);
BYTE *RallyData_FUN_00408cb0(int index);
unsigned int RallyData_FUN_00407e70(void);
unsigned int RallyData_FUN_004082e0(void);

// GLOBAL: CMR2 0x00536c00
int g_unk0x00536c00[2];
// GLOBAL: CMR2 0x00536c28
int g_unk0x00536c28[2];
// GLOBAL: CMR2 0x00536c3c
int g_unk0x00536c3c;
// GLOBAL: CMR2 0x00536e88
int g_unk0x00536e88[2];
// GLOBAL: CMR2 0x00536ec4
int g_unk0x00536ec4[2];
// Name of the stage leader (8 bytes in the original; longer names run into the
// split values at +8/+0xc as there), kept as one block up to 0x536fe0.
// GLOBAL: CMR2 0x00536ed0
BYTE g_unk0x00536ed0Block[0x110];
#define g_unk0x00536ed0 ((char *)g_unk0x00536ed0Block)
#define g_unk0x00536ed8 (*(int *)(g_unk0x00536ed0Block + 8))
#define g_unk0x00536edc (*(int *)(g_unk0x00536ed0Block + 0xc))
// GLOBAL: CMR2 0x0053706c
int g_unk0x0053706c;

// GLOBAL: CMR2 0x005170d0
unsigned int g_stageResultTextColour = 0xb4ffffff;
// GLOBAL: CMR2 0x005170d4
unsigned int g_stageResultPanelColour = 0x96dcbebc;

// Renders each stage result row, including the player's, record and network
// result variants. A qualifying player has no second row.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004147f0
void FUN_004147f0(int car, short *position)
{
    BYTE *record;
    BYTE *gameInfo;
    unsigned int carTime;
    unsigned int recordTime;
    unsigned int time;
    char recordName[8];
    short rect[4];
    int baseHeight;
    int rows;
    int row;
    int shownRow;
    int fade;
    int highlight = 0;
    int offset;
    int shiftedOffset;
    int currentOffset;
    int textY;
    int pixelFixed;
    int dimensionFixed;
    unsigned int panelColour;
    int index;

    if (CGameInfo::FUN_00405d80() == 3) {
        record = RallyData_FUN_00408cb0((FUN_0041b370() & 0xff) + car);
        index = (RallyDataCountryIndex() & 0xff) * 12 + (RallyDataStageIndex() & 0xff);
        carTime = *(unsigned int *)(record + 0x154 + index * 8);
        index = (RallyDataCountryIndex() & 0xff) * 11 + (RallyDataStageIndex() & 0xff);
        gameInfo = (BYTE *)CGameInfo::FUN_00405fe0();
        recordTime = (*(unsigned int *)(gameInfo + 0x658 + index * 8) >> 7) & 0xffff;
        index = (RallyDataCountryIndex() & 0xff) * 11 + (RallyDataStageIndex() & 0xff);
        gameInfo = (BYTE *)CGameInfo::FUN_00405fe0();
        sprintf(recordName, (char *)(gameInfo + 0x654 + index * 8));
    } else {
        record = RallyData_FUN_00408cb0((FUN_0041b370() & 0xff) + car);
        index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
        carTime = *(unsigned int *)(record + 0x4c0 + index * 8);
        index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
        gameInfo = (BYTE *)CGameInfo::FUN_00405fe0();
        recordTime = (*(unsigned int *)(gameInfo + 0x1214 + index * 8) >> 7) & 0xffff;
        index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
        gameInfo = (BYTE *)CGameInfo::FUN_00405fe0();
        sprintf(recordName, (char *)(gameInfo + 0x1210 + index * 8));
    }

    rect[1] = (short)(((int)g_pGraphics->resY << 12 >> 16) + position[1]);
    baseHeight = (int)g_pGraphics->resY * 0xccc >> 16;
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7) {
        rows = CGameInfo::FUN_00406440() ? 2 : 4;
    } else {
        rows = 3;
    }
    if (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12)
        rows = 2;

    offset = 0;
    shiftedOffset = -baseHeight;
    for (row = 0; row < rows; row++, offset += baseHeight, shiftedOffset += baseHeight) {
        currentOffset = offset;
        shownRow = row;
        rect[0] = (short)(((int)g_pGraphics->resX * 0xc00 >> 16) + position[0]);
        rect[2] = (short)((int)g_pGraphics->resX * 0x47ae >> 16);
        rect[3] = (short)((int)g_pGraphics->resY * 0xccc >> 16);
        fade = (signed char)g_unk0x00536c08[car] - 0x77;
        if (fade < 0)
            fade = 0;

        if (CGameInfo::FUN_00405d80() != 10 && CGameInfo::FUN_00405d80() != 12) {
            if (CGameInfo::FUN_00406440()) {
                shownRow = row + 2;
            } else if (FUN_004085a0((BYTE)(FUN_0041b370() + car))) {
                if (row == 1)
                    continue;
                if (row > 1)
                    currentOffset = shiftedOffset;
            }
        }

        if (shownRow == 0) {
            if (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12) {
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, RallyData_GetRecord(0));
                time = FUN_0040a3d0();
            } else {
                sprintf(CFrontend::m_stringDest, g_standingsRowFormat,
                        CFrontend::GetTextString(0x69), recordName);
                time = recordTime;
            }
            highlight = g_unk0x00536c00[car] != 0 && fade != 0;
        } else if (shownRow == 1) {
            if (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12) {
                sprintf(CFrontend::m_stringDest, g_standingsRowFormat,
                        CFrontend::GetTextString(0x84), FUN_0040a400());
                if (CGameInfo::FUN_00405d80() == 10) {
                    time = RallyData_GetFlag25() ? FUN_0040a410(2) : FUN_0040a410(8);
                    // The original keeps the previous row's highlight here.
                } else {
                    time = CGameInfo::FUN_0040a420(0);
                    highlight = g_unk0x00536c28[car] != 0 && fade != 0;
                }
            } else {
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                        RallyData_GetRecord((BYTE)(FUN_0041b370() + car)));
                time = carTime;
                highlight = g_unk0x00536e88[car] != 0 && fade != 0;
            }
        } else if (shownRow == 2) {
            sprintf(CFrontend::m_stringDest, g_standingsRowFormat,
                    CFrontend::GetTextString(0x84), g_unk0x00536ed0);
            time = g_unk0x0053706c;
            highlight = g_unk0x00536c28[car] != 0 && fade != 0;
        } else {
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(0xa5));
            time = FUN_004481f0(car, FUN_00458330(car));
            highlight = g_unk0x00536ec4[car] != 0 && fade != 0;
        }

        panelColour = g_stageResultPanelColour;
        if (highlight) {
            if (fade >= 4) {
                panelColour = (g_gapTextColour[3] << 24) | 0x00ffffff;
            } else {
                BYTE *from = (BYTE *)&g_stageResultPanelColour;
                BYTE *to = (BYTE *)&panelColour;
                for (int channel = 0; channel < 4; channel++) {
                    int target = channel == 3 ? g_gapTextColour[3] : 0xff;
                    to[channel] = (BYTE)(from[channel] + ((target - from[channel]) * fade) / 3);
                }
            }
        }
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&panelColour, 2);
        textY = position[1] + currentOffset + 1 + ((int)g_pGraphics->resY * 0x1b32 >> 16);
        Font_DrawText(0, CFrontend::m_stringDest,
                      (short)((int)g_pGraphics->resX * 0xf95 >> 16) + position[0],
                      textY, (int *)&g_stageResultTextColour, 0x21);
        pixelFixed = (int)(__int64)((double)(textY - 1) * CGraphics::m_65536);
        dimensionFixed = (int)(__int64)((double)g_pGraphics->resY * CGraphics::m_65536);
        FormatGapToLeader(time == 0 ? -1 : time, 5, 5,
                          0x51e4 - (int)(__int64)((double)position[0] * g_minus65536),
                          FixDiv(pixelFixed, dimensionFixed), &g_stageResultTextColour, 0x24, NULL, 0);
        rect[1] += (short)baseHeight;
    }
}

// Called when the car crosses the finish line: raises the finish messages
// (stage record, qualification, best time) for the stage end screen.
// FUNCTION: CMR2 0x00415a60
void FUN_00415a60(int car)
{
    int time;

    if (!FUN_00458290(car))
        return;
    time = FUN_004481f0(car, FUN_00458330(car));
    g_unk0x00536c00[car] = 0;
    g_unk0x00536c88[car] = 0;
    g_unk0x00536c28[car] = 0;
    g_unk0x00536e88[car] = 0;
    g_unk0x00536ec4[car] = 0;
    if ((!(BYTE)CGameInfo::FUN_00406440() || !(BYTE)RallyData_GetFlag24()) && !CGameInfo::FUN_00405e00()) {
        if (FUN_00448cd0(car)) {
            g_unk0x00536c00[car] = 1;
            g_unk0x00536c08[car] = 0x7d;
        }
        if ((g_unk0x00536c3c == 0 || g_unk0x00536c3c > time) &&
            !FUN_004085a0((BYTE)(FUN_0041b370() + car))) {
            g_unk0x00536e88[car] = 1;
            g_unk0x00536c08[car] = 0x7d;
        }
    }
    if (CGameInfo::FUN_00405e00()) {
        if (CGameInfo::FUN_00405d80() != 0xc || !FUN_0040a3c0())
            goto done;
        g_unk0x0053706c = CGameInfo::FUN_0040a420(0);
        sprintf(g_unk0x00536ed0, (char *)RallyData_GetRecord(0));
        g_unk0x00536c28[car] = 1;
        g_unk0x00536c08[car] = 0x7d;
    } else {
        if (g_unk0x0053706c != 0 && g_unk0x0053706c <= time)
            goto done;
        g_unk0x0053706c = time;
        sprintf(g_unk0x00536ed0, (char *)RallyData_GetRecord(FUN_0041b370() + car));
        g_unk0x00536c28[car] = 1;
        g_unk0x00536c08[car] = 0x7d;
    }
done:
    if ((BYTE)RallyData_FUN_00407e70())
        RallyData_FUN_004082e0();
    g_unk0x00536ec4[car] = 1;
    g_unk0x00536c08[car] = 0x7d;
}

// FUNCTION: CMR2 0x004074a0
bool RallyData_FUN_004074a0(void)
{
    unsigned int v;

    if (CGameInfo::FUN_00405d80() == 5) {
        v = g_selectedRallyData;
        v ^= (((v & 0xfffff000) + 0x1000) ^ v) & 0x3000;
        g_selectedRallyData = v;
        if ((v & 0x3000) >= 0x3000)
            return false;
        if ((v & 0xc00) != 0x800)
            return true;
    }
    return false;
}

// GLOBAL: CMR2 0x005167e0
int g_unk0x005167e0[16] = {
    0x00726d63, 0x006b7265, 0x00677564, 0x00636162, 0x00747a74, 0x006b6961,
    0x00627568, 0x00676e69, 0x006e6f6a, 0x00757274, 0x006e6f6a, 0x0073616b,
    0x006e6163, 0x00706f63, 0x007a7475, 0x080a0900,
};   // 3-letter country codes

// FUNCTION: CMR2 0x00407f20
int *RallyData_FUN_00407f20(int index)
{
    int i;

    if (CGameInfo::FUN_00405d80() == 5 ||
        CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7) {
        i = index;
        if (i < 0 || i > 5)
            i = 0;
        i = CAIHelper::FUN_00407f80(i);
    } else {
        i = index;
        if (i < 0 || i > 0xf)
            i = 0;
    }
    return &g_unk0x005167e0[i];
}

// FUNCTION: CMR2 0x0040fe50
char *RallyData_FUN_0040fe50(void)
{
    int table[8];

    table[0] = 6;
    table[1] = 3;
    table[2] = 1;
    table[3] = 4;
    table[4] = 0;
    table[5] = 2;
    table[6] = 5;
    table[7] = 7;
    return CFrontend::GetTextString(table[(RallyData_FUN_00406940() & 0xff) * 3 +
                                          (RallyData_FUN_00406950() & 0xff)]);
}

extern char g_noTimeText[];
BYTE FUN_004085a0(BYTE param1);
extern BYTE g_itemColour[4];
extern char g_classRowHeaderFormat[];
int RallyData_DrawListItem(int x, int y, char *pText, char last, BYTE alpha);
BYTE *RallyData_FUN_00408cb0(int index);

// GLOBAL: CMR2 0x00516e3c
char g_loadRecordTimeFormat[] = "%.2d:%.2d.%.2d";

#define LOAD_TIME_TEXT(t) sprintf(CFrontend::m_stringDest, g_loadRecordTimeFormat, (t) / 6000, (int)(((t) / 100) % 60), (t) % 100)

// Returns 1 when the stage must end early: a championship with more lost
// than remaining rounds, or a replay being skipped.
// FUNCTION: CMR2 0x004100a0
int FUN_004100a0(void)
{
    unsigned int *pState = RallyData_GetChampionshipState();

    if (CGameInfo::FUN_00405d80() == 4 && (int)(5 - (*pState & 7)) < (int)((*pState >> 3) & 7))
        return 1;
    if (CGameInfo::FUN_00405da0() && FUN_0041b370()) {
        CGraphics::ClearTarget();
        return 1;
    }
    return 0;
}

// Loading screen text: the breadcrumb (game mode, rally, stage), the stage
// record and the best time of every player on it, faded in with alpha.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00410100
void FUN_00410100(BYTE alpha)
{
    BYTE colour[4];
    GameInfo0xa4 *pInfo;
    unsigned int time;
    int x;
    int i;
    int shown;
    int next;

    colour[0] = g_itemColour[0];
    colour[1] = g_itemColour[1];
    colour[2] = g_itemColour[2];
    colour[3] = alpha;
    x = (int)(g_pGraphics->resX * 30) / 640;
    if (FUN_00407270()) {
        RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x93), 1, alpha);
        return;
    }
    switch (CGameInfo::FUN_00405d80()) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 8:
    case 9:
    case 10:
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x6b), 0, alpha);
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x41), 0, alpha);
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                CFrontend::GetTextString(RallyDataCountryIndex() & 0xff));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::m_stringDest, 0, alpha);
        if (RallyDataStageIndex() == 10)
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0xba));
        else
            sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat, CFrontend::GetTextString(0x40),
                    RallyDataStageIndex() + 1);
        RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::m_stringDest, 1, alpha);
        pInfo = CGameInfo::FUN_00405fe0();
        time = (pInfo->rallyStageRecordTimes[(RallyDataCountryIndex() & 0xff) * 11 + (RallyDataStageIndex() & 0xff)]
                    .value >> 7) & 0xffff;
        LOAD_TIME_TEXT(time);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x268) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480, (int *)colour, 0x24);
        pInfo = CGameInfo::FUN_00405fe0();
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                pInfo->rallyStageRecordTimes[(RallyDataCountryIndex() & 0xff) * 11 + (RallyDataStageIndex() & 0xff)].ident);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x22a) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480, (int *)colour, 0x24);
        Font_DrawText(0, CFrontend::GetTextString(0x6d), (int)(g_pGraphics->resX * 0x268) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480 - (int)(g_pGraphics->resY * 0x18) / 480, (int *)colour,
                      0x24);
        shown = 0;
        if ((RallyDataState() & 0xff) == 0)
            return;
        for (i = 0; i < (int)(RallyDataState() & 0xff); i++) {
            next = shown;
            if (!FUN_004085a0(i)) {
                next = shown + 1;
                if (!(*(RallyData_FUN_00408cb0(i) + 0x150 +
                        ((RallyDataStageIndex() & 0xff) + (RallyDataCountryIndex() & 0xff) * 12) * 8) &
                      0x80)) {
                    sprintf(CFrontend::m_stringDest, g_noTimeText);
                } else {
                    time = *(unsigned int *)(RallyData_FUN_00408cb0(i) + 0x154 +
                                             ((RallyDataStageIndex() & 0xff) + (RallyDataCountryIndex() & 0xff) * 12) * 8);
                    LOAD_TIME_TEXT(time);
                }
                Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x268) / 640,
                              (int)(g_pGraphics->resY * 0x1a6) / 480 - ((int)(g_pGraphics->resY * 0x18) / 480) * (shown + 3),
                              (int *)colour, 0x24);
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(i));
                Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x22a) / 640,
                              (int)(g_pGraphics->resY * 0x1a6) / 480 - ((int)(g_pGraphics->resY * 0x18) / 480) * (shown + 3),
                              (int *)colour, 0x24);
            }
            shown = next;
        }
        break;
    case 4:
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x6b), 0, alpha);
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x6f), 0, alpha);
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                CFrontend::GetTextString(RallyDataCountryIndex() & 0xff));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::m_stringDest, 1, alpha);
        return;
    case 5:
    case 6:
    case 7:
    case 11:
    case 12:
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x6b), 0, alpha);
        x = RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::GetTextString(0x6c), 0, alpha);
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, RallyData_FUN_0040fe50());
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        RallyData_DrawListItem(x, g_pGraphics->resY / 2, CFrontend::m_stringDest, 1, alpha);
        pInfo = CGameInfo::FUN_00405fe0();
        time = (pInfo->arcadeRecordTimes[(RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)]
                    .value >> 7) & 0xffff;
        LOAD_TIME_TEXT(time);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x268) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480, (int *)colour, 0x24);
        pInfo = CGameInfo::FUN_00405fe0();
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                pInfo->arcadeRecordTimes[(RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)].ident);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x22a) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480, (int *)colour, 0x24);
        Font_DrawText(0, CFrontend::GetTextString(0x6d), (int)(g_pGraphics->resX * 0x268) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480 - (int)(g_pGraphics->resY * 0x18) / 480, (int *)colour,
                      0x24);
        shown = 0;
        if ((RallyDataState() & 0xff) == 0)
            return;
        for (i = 0; i < (int)(RallyDataState() & 0xff); i++) {
            next = shown;
            if (!FUN_004085a0(i)) {
                next = shown + 1;
                if (!(*(RallyData_FUN_00408cb0(i) + 0x4bc +
                        ((RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)) * 8) &
                      0x80)) {
                    sprintf(CFrontend::m_stringDest, g_noTimeText);
                } else {
                    time = *(unsigned int *)(RallyData_FUN_00408cb0(i) + 0x4c0 +
                                             ((RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)) * 8);
                    LOAD_TIME_TEXT(time);
                }
                Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x268) / 640,
                              (int)(g_pGraphics->resY * 0x1a6) / 480 - ((int)(g_pGraphics->resY * 0x18) / 480) * (shown + 3),
                              (int *)colour, 0x24);
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(i));
                Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x22a) / 640,
                              (int)(g_pGraphics->resY * 0x1a6) / 480 - ((int)(g_pGraphics->resY * 0x18) / 480) * (shown + 3),
                              (int *)colour, 0x24);
            }
            shown = next;
        }
        break;
    default:
        return;
    }
    if (shown > 0)
        Font_DrawText(0, CFrontend::GetTextString(0x70), (int)(g_pGraphics->resX * 0x268) / 640,
                      (int)(g_pGraphics->resY * 0x1a6) / 480 - ((int)(g_pGraphics->resY * 0x18) / 480) * (shown + 3),
                      (int *)colour, 0x24);
}

// GLOBAL: CMR2 0x0058c938
BYTE *g_unk0x0058c938;
// GLOBAL: CMR2 0x0058c94c
BYTE *g_unk0x0058c94c;
// GLOBAL: CMR2 0x0058c958
int *g_unk0x0058c958;
// GLOBAL: CMR2 0x0058ca6c
unsigned int g_unk0x0058ca6c;

// Per-sector object lists: sector -> list index (-1 none), entry count and
// 4-byte entries, for two kinds of stage objects.
// GLOBAL: CMR2 0x0058c940
short *g_sectorListIndexA;
// GLOBAL: CMR2 0x0058c960
unsigned short *g_sectorListCountA;
// GLOBAL: CMR2 0x0058ca78
BYTE *g_sectorListEntriesA;
// GLOBAL: CMR2 0x0058c93c
short *g_sectorListIndexB;
// GLOBAL: CMR2 0x0058c964
unsigned short *g_sectorListCountB;
// GLOBAL: CMR2 0x0058c954
BYTE *g_sectorListEntriesB;

extern int g_sectorCount;

// FUNCTION: CMR2 0x00471b30
BYTE *Sector_GetListA(unsigned int sector, unsigned int *pCount)
{
    int index;

    *pCount = 0;
    if (g_sectorListIndexA != NULL && sector < (unsigned int)g_sectorCount &&
        (index = g_sectorListIndexA[sector]) != -1) {
        *pCount = g_sectorListCountA[index];
        return g_sectorListEntriesA + index * 4;
    }
    return NULL;
}

// FUNCTION: CMR2 0x00471b80
BYTE *Sector_GetListB(unsigned int sector, unsigned int *pCount)
{
    int index;

    *pCount = 0;
    if (g_sectorListIndexB != NULL && sector < (unsigned int)g_sectorCount &&
        (index = g_sectorListIndexB[sector]) != -1) {
        *pCount = g_sectorListCountB[index];
        return g_sectorListEntriesB + index * 4;
    }
    return NULL;
}

// Championship state entry for the current round kind (bits 3..5 of the
// state; bits 12..15 select the round within kinds 1..3).
// FUNCTION: CMR2 0x00473620
unsigned int *RallyData_GetRoundEntry(void)
{
    unsigned int *pState;
    unsigned int state;

    pState = RallyData_GetChampionshipState();
    state = *pState;
    switch ((state >> 3) & 7) {
    case 1:
        return pState + ((state >> 12) & 0xf) * 3 + 0x16;
    case 2:
        return pState + ((state >> 12) & 0xf) * 3 + 10;
    case 3:
        return pState + ((state >> 12) & 0xf) * 3 + 4;
    case 4:
        return pState + 1;
    default:
        return NULL;
    }
}

// Moves each element's object in or out of the way to match the car's
// reached flags (multi-player only).
// match 29%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00471bf0
void FUN_00471bf0(BYTE car)
{
    int i;
    BYTE bit;
    BYTE *pObject;

    if ((BYTE)RallyDataState() < 2) {
        RallyData_ValidateIndex(car);
        return;
    }
    for (i = 0; i < (int)g_unk0x0058ca6c; i++) {
        bit = g_unk0x0058c938[i] & (1 << car);
        if (bit != 0 && g_unk0x0058c958[i] == 0) {
            pObject = *(BYTE **)(g_unk0x0058c94c + i * 8);
            *(int *)(pObject + 4) += 0x3e80000;
            (*(BYTE **)(g_unk0x0058c94c + i * 8))[0x14] = 0xff;
            g_unk0x0058c958[i] = 1;
        } else if (bit == 0 && g_unk0x0058c958[i] != 0) {
            pObject = *(BYTE **)(g_unk0x0058c94c + i * 8);
            *(int *)(pObject + 4) -= 0x3e80000;
            (*(BYTE **)(g_unk0x0058c94c + i * 8))[0x14] = 0;
            g_unk0x0058c958[i] = 0;
        }
    }
}

// Copies the 12-byte vector and, when the entry is not already flagged,
// raises the destination's Y component by 0x3e80000.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00471cc0
void RallyData_FUN_00471cc0(int *pDest, void **pParam1)
{
    int *pSrc;
    int **pp;
    unsigned int index;

    pp = (int **)*pParam1;
    pSrc = *pp;
    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = pSrc[2];
    index = (unsigned int)((BYTE *)pParam1[0] - g_unk0x0058c94c) / 8;
    if (index >= g_unk0x0058ca6c)
        return;
    if (RallyDataState() > 1) {
        if (g_unk0x0058c958[index] != 0)
            return;
        pDest[1] = pDest[1] + 0x3e80000;
    } else {
        if ((g_unk0x0058c938[index] & 1) != 0)
            return;
        pDest[1] += 0x3e80000;
    }
}


// Bumps the 0x7f80-masked field of the entry selected by each 0x30-byte record.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ec1a0
void RallyData_FUN_004ec1a0(void)
{
    unsigned int *pEntry;
    unsigned int i;

    i = 0;
    if (CGameInfo::FUN_00405d70() == 0)
        return;
    do {
        pEntry = (unsigned int *)(g_unk0x0052fa5c +
                 (((*(unsigned int *)(g_unk0x00531350 + i * 0x30) >> 0x12) & 0xf) * 0x650));
        if ((*pEntry & 0x7f80) != 0x7f80)
            *pEntry = (*pEntry & 0xffffff80) |
                      (((*pEntry & 0x7f80) + 0x80) & 0x7f80);
        i++;
    } while (i < CGameInfo::FUN_00405d70());
}

// Default setup of a player's car: validates the player's rally data (except
// in mode 4) and stores the default position and heading values.
// FUNCTION: CMR2 0x004ec260
void FUN_004ec260(int player)
{
    int values[5];

    if (CGameInfo::FUN_00405d80() != 4)
        RallyData_ValidateIndex(player);
    *(short *)&values[3] = 0x2d;
    values[0] = 0;
    values[1] = 0x18000;
    values[2] = 0x68000;
    values[4] = 0xe0000;
    RallyData_FUN_004088a0((BYTE)player, values);
}

// GLOBAL: CMR2 0x00520128
BYTE g_unk0x00520128[0x28] = {
    0x04, 0x02, 0x3c, 0x32, 0x32, 0x32, 0x32, 0x06, 0x02, 0x28, 0x46, 0x32, 0x32, 0x32, 0x04, 0x02,
    0x1e, 0x32, 0x32, 0x32, 0x32, 0x02, 0x02, 0x32, 0x32, 0x32, 0x32, 0x32, 0x04, 0x02, 0x32, 0x32,
    0x32, 0x32, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00,
};


// GLOBAL: CMR2 0x0082c698
int g_unk0x0082c698;
// GLOBAL: CMR2 0x0082c69c
int g_unk0x0082c69c;
// GLOBAL: CMR2 0x0082c6a0
int g_unk0x0082c6a0;
// GLOBAL: CMR2 0x0082c6a4
int g_unk0x0082c6a4;
// GLOBAL: CMR2 0x0082c6a8
int g_unk0x0082c6a8;
// GLOBAL: CMR2 0x0082c6ac
int g_unk0x0082c6ac;
// GLOBAL: CMR2 0x0082c6bc
int g_unk0x0082c6bc;

// FUNCTION: CMR2 0x00503e00
void FUN_00503e00(void)
{
    if ((unsigned char)RallyDataCountryIndex() == 3) {
        g_unk0x0082c698 = 0;
        g_unk0x0082c6bc = 6;
        g_unk0x0082c69c = 1;
        g_unk0x0082c6a0 = 2;
        g_unk0x0082c6a4 = 6;
        g_unk0x0082c6a8 = 7;
        g_unk0x0082c6ac = 8;
    } else {
        g_unk0x0082c6bc = 6;
        g_unk0x0082c698 = 0;
        g_unk0x0082c69c = 1;
        g_unk0x0082c6a0 = 2;
        g_unk0x0082c6a4 = 3;
        g_unk0x0082c6a8 = 4;
        g_unk0x0082c6ac = 5;
    }
}

#include "InstallInfo.h"
#include "Game.h"
int *FUN_0050f620(void);
int *FUN_0050f640(void);
int FUN_00458040(void);

extern char g_str0x00519268[4];
extern char g_str0x0051926c[4];
extern char g_str0x00519270[4];
extern char g_str0x00519274[4];
extern char g_str0x00519278[4];
extern char g_str0x0051927c[4];
extern char g_str0x00519280[4];
// GLOBAL: CMR2 0x00519348
char g_str0x00519348[6] = "ITALY";
// GLOBAL: CMR2 0x00519350
char g_str0x00519350[6] = "KENYA";
// GLOBAL: CMR2 0x00519358
char g_str0x00519358[7] = "SWEDEN";
// GLOBAL: CMR2 0x00519360
char g_str0x00519360[7] = "FRANCE";
// GLOBAL: CMR2 0x00519368
char g_str0x00519368[7] = "GREECE";
// GLOBAL: CMR2 0x00519370
char g_str0x00519370[8] = "FINLAND";
// GLOBAL: CMR2 0x0052726c
char g_str0x0052726c[10] = "AUSTRALIA";
// GLOBAL: CMR2 0x00527278
char g_str0x00527278[8] = "SSHOWER";
// GLOBAL: CMR2 0x00527280
char g_str0x00527280[6] = "CLEAR";

// GLOBAL: CMR2 0x0052714c
char *g_unk0x0052714c[8] = {
    g_str0x00519370, g_str0x00519368, g_str0x00519360, g_str0x00519358, g_str0x0052726c, g_str0x00519350, g_str0x00519348, CFrontend::m_strUK
};
// GLOBAL: CMR2 0x0052716c
char *g_unk0x0052716c[8] = {
    g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274, g_str0x00519270, g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK
};
// GLOBAL: CMR2 0x00527128
char *g_unk0x00527128[9] = {
    (char *)(g_unk0x0051682c + 0x70), g_str0x00527280, (char *)(g_unk0x0051682c + 0x68), (char *)(g_unk0x0051682c + 0x60), (char *)(g_unk0x0051682c + 0x58), (char *)(g_unk0x0051682c + 0x50), g_str0x00527278, (char *)(g_unk0x0051682c + 0x48), (char *)(g_unk0x0051682c + 0x78)
};

// GLOBAL: CMR2 0x0082cb48
unsigned int g_unk0x0082cb48;
// GLOBAL: CMR2 0x0082cb4c
void *g_unk0x0082cb4c;
// GLOBAL: CMR2 0x0082cb50
int g_unk0x0082cb50[4];
// GLOBAL: CMR2 0x0082cb60
short g_unk0x0082cb60[8];
// GLOBAL: CMR2 0x0082cb70
BYTE g_unk0x0082cb70[8];
// GLOBAL: CMR2 0x0082c690
void *g_unk0x0082c690;
extern int g_unk0x0082c694;
// GLOBAL: CMR2 0x0082ca20
int g_unk0x0082ca20[9];
// GLOBAL: CMR2 0x0082c9e8
void *g_unk0x0082c9e8;
// GLOBAL: CMR2 0x0082c9f4
int g_unk0x0082c9f4;
// GLOBAL: CMR2 0x0082c9f8
int g_unk0x0082c9f8;
// GLOBAL: CMR2 0x0082c9fa
short g_unk0x0082c9fa;
// GLOBAL: CMR2 0x0082c9fc
short g_unk0x0082c9fc;
// GLOBAL: CMR2 0x0082c9fe
short g_unk0x0082c9fe;
// GLOBAL: CMR2 0x0082ca00
short g_unk0x0082ca00;
// GLOBAL: CMR2 0x0082ca02
short g_unk0x0082ca02;
// GLOBAL: CMR2 0x0052718c
BYTE g_unk0x0052718c[22] = {
    0x94, 0x99, 0x91, 0x8b, 0xa0, 0x8e, 0x94, 0x99, 0x7d, 0x4c, 0x75,
    0x50, 0x6f, 0x44, 0x7d, 0x4c, 0x82, 0x9c, 0x79, 0xa5, 0x00, 0x00
};
// GLOBAL: CMR2 0x00527300
char g_str0x00527300[35] = "%s\\Textures\\Weather\\%s\\MainMap.tga";
// GLOBAL: CMR2 0x005272dc
char g_str0x005272dc[34] = "%s\\Textures\\Weather\\%s\\%s%.2d.tga";
// GLOBAL: CMR2 0x005272b4
char g_str0x005272b4[38] = "%s\\Textures\\Weather\\Symbols\\%d\\%s.tga";
// GLOBAL: CMR2 0x00527290
char g_str0x00527290[33] = "%s\\Textures\\Symbols\\%d\\Arrow.tga";

// 0x50-byte entry of the table at 0x82c6c8 (same layout as GameInfo.cpp).
struct Unk0x0082c6c8 {
    BYTE field_0x0[0x1c];
    int field_0x1c;
    BYTE field_0x20[0x2c];
    int field_0x4c;
};
extern Unk0x0082c6c8 g_unk0x0082c6c8[16];
extern short g_unk0x0082c9ec[4];
extern BYTE g_unk0x0082ca1c;
extern int g_unk0x0082c6c0;
extern int g_unk0x0082cb44;

// Builds the weather textures of the current rally: the main map, the
// per-weather maps and the weather symbols, then rebinds the stage state.
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00503ea0
void FUN_00503ea0(void)
{
    int i;
    int count;
    int stage;
    int index;
    short *pPair;
    int *pMap;

    *(BYTE *)&g_unk0x0082cb48 = 0;
    pMap = g_unk0x0082cb50;
    pPair = g_unk0x0082cb60;
    while ((int)pPair < (int)((BYTE *)g_unk0x0082cb70 + 4)) {
        *pMap = 0;
        pPair[0] = 0;
        pPair[1] = 0;
        pMap++;
        pPair += 2;
    }
    stage = (BYTE)RallyDataStageIndex() >> 2;
    if (stage < 2)
        *(BYTE *)&g_unk0x0082cb48 = 4;
    else
        *(BYTE *)&g_unk0x0082cb48 = (BYTE)RallyDataCountryIndex() % 2 + 2;
    count = g_unk0x0082cb48 & 0xff;
    i = 0;
    pPair = g_unk0x0082cb60 + 1;
    while (i < 4) {
        if (i < count) {
            g_unk0x0082cb70[i] = (BYTE)(stage * 4 + i);
            pPair[-1] = g_unk0x0052718c
                [(RallyDataCountryIndex() * 0xb + g_unk0x0082cb70[i]) * 2];
            pPair[0] = g_unk0x0052718c
                [(RallyDataCountryIndex() * 0xb + g_unk0x0082cb70[i]) * 2 + 1];
        }
        pPair += 2;
        i++;
    }
    sprintf(CFrontend::m_stringDest, g_str0x00527300, CInstallInfo::GetSetupRepDir(),
            g_unk0x0052714c[RallyDataCountryIndex()]);
    g_unk0x0082cb4c = CTexture::FindLoadTexture((GenericFile *)FUN_0050f640(),
                                                CFrontend::m_stringDest, 0, NULL, false, 0);
    for (i = 0; i < 4; i++) {
        if (i < count) {
            index = g_unk0x0082cb70[i] + 1;
            sprintf(CFrontend::m_stringDest, g_str0x005272dc, CInstallInfo::GetSetupRepDir(),
                    g_unk0x0052714c[RallyDataCountryIndex()],
                    g_unk0x0052716c[RallyDataCountryIndex()], index);
            g_unk0x0082cb50[i] = (int)CTexture::FindLoadTexture((GenericFile *)FUN_0050f640(),
                                                                 CFrontend::m_stringDest, 0, NULL, false, 0);
        }
    }
    for (i = 0; i < 9; i++) {
        sprintf(CFrontend::m_stringDest, g_str0x005272b4, CInstallInfo::GetSetupRepDir(),
                0x280, g_unk0x00527128[i]);
        g_unk0x0082ca20[i] = (int)CTexture::FindLoadTexture((GenericFile *)FUN_0050f620(),
                                                             CFrontend::m_stringDest, 0, NULL, false, 0);
    }
    sprintf(CFrontend::m_stringDest, g_str0x00527290, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x0082c690 = CTexture::FindLoadTexture((GenericFile *)FUN_0050f620(),
                                                CFrontend::m_stringDest, 0, NULL, false, 0);
    CGame::RegisterCallback((void *)FUN_00458040, 0);
}

// Sets up the weather entries of the stage: their texture, screen rectangle and
// the fixed texture coordinates.
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x005040f0
void FUN_005040f0(void)
{
    BYTE *pEntry;
    BYTE *pTex;
    int count;
    int i;
    int index;
    int x;
    int y;

    pTex = (BYTE *)g_unk0x0082cb4c;
    g_unk0x0082c9e8 = g_unk0x0082cb4c;
    g_unk0x0082c9ec[0] = 0x1c;
    g_unk0x0082c9ec[1] = 0x72;
    g_unk0x0082c9ec[2] = 0x100;
    g_unk0x0082c9ec[3] = 0xf6;
    g_unk0x0082c9f4 = *(int *)(pTex + 0x11c);
    g_unk0x0082c9f8 = *(int *)(pTex + 0x120);
    g_unk0x0082c9fa = 0xf6;
    g_unk0x0082c9fc = 0xd8;
    g_unk0x0082c9fe = 0x7c;
    g_unk0x0082ca00 = 0x3b;
    g_unk0x0082ca02 = 0x39;
    count = CGameInfo::FUN_00501230();
    g_unk0x0082c694 = count;
    if (count > 0) {
        for (i = 0; i < count; i++) {
            pEntry = (BYTE *)g_unk0x0082c6c8 + i * 0x50;
            *(int *)(pEntry + 0x18) = 0;
            *(int *)(pEntry + 0x14) = 0;
            *(int *)(pEntry + 0x1c) = 0;
            *(int *)(pEntry + 0x24) = 0;
            *(int *)(pEntry + 0x28) = 0;
            *(int *)(pEntry + 0x4c) = 0;
            *(int *)(pEntry + 0x20) = 0;
            *(int *)(pEntry + 0x44) = 0;
            *(int *)(pEntry + 0x40) = 0;
            *(int *)(pEntry + 0x3c) = 0;
            index = (RallyDataStageIndex() & 3) + i;
            *(BYTE *)(pEntry + 0x48) = g_unk0x0082cb70[index];
            *(int *)pEntry = g_unk0x0082cb50[index];
            x = g_unk0x0082cb60[index * 2] + g_unk0x0082c9ec[0];
            y = g_unk0x0082cb60[index * 2 + 1] + g_unk0x0082c9ec[1];
            *(short *)(pEntry + 0x04) = (short)x;
            *(short *)(pEntry + 0x06) = (short)y;
            *(short *)(pEntry + 0x08) = 10;
            *(short *)(pEntry + 0x0a) = 9;
            *(int *)(pEntry + 0x2c) = (short)g_unk0x0082c9f4 * 0x10000 +
                FixMul(FixDiv((x - g_unk0x0082c9ec[0]) << 16, (short)g_unk0x0082c9ec[2] << 16),
                       (short)g_unk0x0082c9f8 << 16);
            *(int *)(pEntry + 0x30) = (short)g_unk0x0082c9f4 * 0x10000 +
                FixMul(FixDiv(((short)x - g_unk0x0082c9ec[0] + 10) << 16,
                              (short)g_unk0x0082c9ec[2] << 16),
                       (short)g_unk0x0082c9f8 << 16);
            *(int *)(pEntry + 0x34) = (short)(g_unk0x0082c9f4 >> 16) * 0x10000 +
                FixMul(FixDiv((y - g_unk0x0082c9ec[1]) << 16, (short)g_unk0x0082c9ec[3] << 16),
                       (short)g_unk0x0082c9fa << 16);
            *(int *)(pEntry + 0x38) = (short)(g_unk0x0082c9f4 >> 16) * 0x10000 +
                FixMul(FixDiv(((short)y - g_unk0x0082c9ec[1] + 9) << 16,
                              (short)g_unk0x0082c9ec[3] << 16),
                       (short)g_unk0x0082c9fa << 16);
        }
    }
    g_unk0x0082ca1c = 0;
    g_unk0x0082c6c0 = CMain::GetFrameDelta();
    g_unk0x0082cb44 = 0;
    FUN_00503e00();
}

// Copies the per-driver stage times into the 0x30-byte records, first for the
// used drivers (in reverse) and then for the unused ones.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408d80
void RallyData_FUN_00408d80(void)
{
    unsigned int *pRec;
    int count;
    int limit;
    int i;

    count = CGameInfo::FUN_00405d70() & 0xff;
    for (i = 0; i < count; i++) {
        pRec = (unsigned int *)(g_unk0x00531350 + i * 0x30);
        *pRec = (*pRec & 0xffffe03f) |
                ((RallyTiming_GetStageTimeSeconds(0xf - i) & 0x7f) << 6);
        *(int *)(g_unk0x00531350 + i * 0x30 + 4) = RallyTiming_FUN_0040d3d0(0xf - i);
    }
    limit = 0x10 - (CGameInfo::FUN_00405d70() & 0xff);
    for (i = 0; i < limit; i++) {
        pRec = (unsigned int *)(g_unk0x00531350 +
                                ((CGameInfo::FUN_00405d70() & 0xff) + i) * 0x30);
        *pRec = (*pRec & 0xffffe03f) |
                ((RallyTiming_GetStageTimeSeconds(i) & 0x7f) << 6);
        *(int *)(g_unk0x00531350 +
                 ((CGameInfo::FUN_00405d70() & 0xff) + i) * 0x30 + 4) =
            RallyTiming_FUN_0040d3d0(i);
    }
}

void RallyData_FUN_00420820(void);

// Per car progress along the route, 0x18 bytes
struct RaceRecord {
    int field_0x0;
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int field_0x10;
    short field_0x14;
    short field_0x16;
};

// GLOBAL: CMR2 0x00538a78
unsigned int g_routeProbeBestDistance[2];
// GLOBAL: CMR2 0x00538a80
unsigned short g_routeProbeIndex[2];
// GLOBAL: CMR2 0x00538a84
int g_unk0x00538a84;
// GLOBAL: CMR2 0x00538a88
int g_unk0x00538a88;
// GLOBAL: CMR2 0x00538a94
int g_unk0x00538a94;
// GLOBAL: CMR2 0x00538a98
short g_routeProbeBestIndex[2];
// GLOBAL: CMR2 0x00538aa8
RaceRecord g_raceRecords[8];
// GLOBAL: CMR2 0x00538c8c
int g_routeProbeCycles[2];

// Restarts race record index; the two players also reset their route probe.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004207a0
void RallyData_FUN_004207a0(int index)
{
    g_raceRecords[index].field_0x0 = g_raceRecords[index].field_0x4;
    g_raceRecords[index].field_0x8 = 0;
    g_raceRecords[index].field_0x14 = 0;
    g_raceRecords[index].field_0xc = 0;
    if (index < 2) {
        g_routeProbeCycles[index] = 0;
        g_routeProbeBestDistance[index] = 0;
        g_routeProbeBestIndex[index] = 0;
        g_routeProbeIndex[index] = 0;
    }
}

// Sets the car's race record position.
// FUNCTION: CMR2 0x004213d0
void RallyData_FUN_004213d0(Car *pCar, int value)
{
    if (g_unk0x00538a84 != 0) {
        g_raceRecords[pCar->field_0xb1a].field_0x0 = value;
        g_raceRecords[pCar->field_0xb1a].field_0x14 = (short)value;
        if (pCar->field_0xb1a < 2)
            g_routeProbeCycles[pCar->field_0xb1a] = 0;
    }
}

// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004207f0
void RallyData_FUN_004207f0(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        g_raceRecords[i].field_0x8 = 0;
        g_raceRecords[i].field_0x0 = g_raceRecords[i].field_0x4;
        g_raceRecords[i].field_0x14 = 0;
        g_raceRecords[i].field_0xc = 0;
    }
    RallyData_FUN_00420820();
}

// match 66%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00420820
void RallyData_FUN_00420820(void)
{
    *(int *)g_routeProbeIndex = 0;
    *(int *)g_routeProbeBestIndex = 0;
    g_routeProbeBestDistance[0] = 0;
    g_routeProbeCycles[0] = 0;
    g_routeProbeBestDistance[1] = 0;
    g_routeProbeCycles[1] = 0;
}

void RallyData_FUN_00421530(int index, int *pOut);
unsigned int RallyData_FUN_00407e90(void);
void FUN_00421230(Car *pCar, int *pProgress);

// Keep the two horizontal components in range before the original fixed point
// length and reciprocal scale helpers run.
#define ROUTE_NORMALIZE(v) do { \
    if (FIX_ABS((v).x) > 0x640000 || FIX_ABS((v).z) > 0x640000) { \
        (v).x /= 512; \
        (v).z /= 512; \
    } \
    int length = FixVecLength(&(v)); \
    if (length == 0) { \
        (v).x = 0; (v).y = 0; (v).z = 0; \
    } else { \
        FixVecScaleRecip(&(v), &(v), length); \
    } \
} while (0)

// Tracks a car along the stage route. Local players periodically probe nearby
// nodes; then the forward and previous segment tests advance or retreat the
// route index and update progress along that segment.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00420a30
void RallyData_UpdateCarRoute(Car *pCar)
{
    int slot = (signed char)pCar->field_0xb1a;
    RaceRecord *record = &g_raceRecords[slot];
    FixVector point;
    FixVector other;
    FixVector delta;
    FixVector scaled;
    FixVector routeDir;
    int node;
    int previous;
    int i;
    int length;
    unsigned int distance;

    if (g_unk0x00538a84 == 0)
        return;

    if (slot < (int)(RallyDataState() & 0xff) && RallyData_GetFlag25() == 0 && RallyData_GetFlag24() == 0) {
        for (i = 0; i < 25; i++) {
            short probe = (short)g_routeProbeIndex[slot];
            RallyData_FUN_00421530((unsigned short)probe, (int *)&point);
            delta.x = point.x - pCar->position.x;
            delta.y = 0;
            delta.z = point.z - pCar->position.z;
            FixVecScale(&scaled, &delta, 0x28f);
            distance = (unsigned int)FixVecDot(&scaled, &scaled);
            if (distance <= g_routeProbeBestDistance[slot] || probe == g_routeProbeBestIndex[slot]) {
                g_routeProbeBestDistance[slot] = distance;
                g_routeProbeBestIndex[slot] = probe;
            }
            g_routeProbeIndex[slot] = (unsigned short)(probe + 1);
            if (g_routeProbeIndex[slot] == (unsigned int)g_unk0x00538a88)
                g_routeProbeIndex[slot] = 0;
        }
        if (++g_routeProbeCycles[slot] > 25) {
            record->field_0x0 = (unsigned short)g_routeProbeBestIndex[slot];
            g_routeProbeCycles[slot] = 0;
        }
    }

    node = record->field_0x0;
    record->field_0x14 = (short)node;
    RallyData_FUN_00421530(node, (int *)&point);
    delta.x = point.x - pCar->position.x;
    delta.y = 0;
    delta.z = point.z - pCar->position.z;
    ROUTE_NORMALIZE(delta);
    RallyRoute_GetNodeDirection(&routeDir, node);

    if (FixVecDot(&delta, &routeDir) < 0) {
        if (g_unk0x00538a94 == 0) {
            if ((unsigned int)node < (unsigned int)(g_unk0x00538a88 - 1)) {
                record->field_0x14++;
                record->field_0x0++;
                record->field_0x8++;
            } else {
                record->field_0x14++;
                record->field_0x8 = g_unk0x00538a84 + 1;
            }
        } else if ((unsigned int)node < (unsigned int)(g_unk0x00538a84 - 1)) {
            record->field_0x14++;
            record->field_0x0++;
            record->field_0x8++;
        } else {
            record->field_0x0 = 0;
            record->field_0x14 = 0;
            record->field_0x8++;
        }
        record->field_0xc = 0;
        if (slot < (int)(RallyDataState() & 0xff) && RallyData_FUN_00407e90() == 0)
            g_routeProbeCycles[slot] = 0;
        FUN_00421230(pCar, (int *)record);
        return;
    }

    if (g_unk0x00538a94 != 0 || node != 0) {
        previous = node;
        if (previous == 0)
            previous = g_unk0x00538a84;
        previous--;
        RallyData_FUN_00421530(previous, (int *)&point);
        delta.x = point.x - pCar->position.x;
        delta.y = 0;
        delta.z = point.z - pCar->position.z;
        ROUTE_NORMALIZE(delta);
        RallyRoute_GetNodeDirection(&routeDir, previous);
        if (FixVecDot(&delta, &routeDir) > 0) {
            record->field_0x0 = previous;
            record->field_0x14 = (short)previous;
            if (g_unk0x00538a94 == 0 && (unsigned int)previous == (unsigned int)(g_unk0x00538a84 - 2))
                record->field_0x8 = previous;
            else
                record->field_0x8--;
            RallyData_FUN_00421530(node, (int *)&point);
            RallyData_FUN_00421530(previous, (int *)&other);
            delta.x = point.x - other.x;
            delta.y = 0;
            delta.z = point.z - other.z;
            if (FIX_ABS(delta.x) > 0x640000 || FIX_ABS(delta.z) > 0x640000) {
                delta.x /= 512;
                delta.z /= 512;
                length = FixVecLength(&delta) << 9;
            } else {
                length = FixVecLength(&delta);
            }
            record->field_0xc += length;
        }
    }
    FUN_00421230(pCar, (int *)record);
}

#undef ROUTE_NORMALIZE

// FUNCTION: CMR2 0x00421370
int RallyData_FUN_00421370(BYTE *p)
{
    if (g_unk0x00538a84 == 0)
        return 0;
    return g_raceRecords[(signed char)p[0xb1a]].field_0x0;
}

// FUNCTION: CMR2 0x004213a0
short RallyData_FUN_004213a0(BYTE *p)
{
    if (g_unk0x00538a84 == 0)
        return 0;
    return g_raceRecords[(signed char)p[0xb1a]].field_0x14;
}

// FUNCTION: CMR2 0x00421420
int RallyData_FUN_00421420(void)
{
    return g_unk0x00538a84;
}

// FUNCTION: CMR2 0x00421430
int RallyData_FUN_00421430(void)
{
    return g_unk0x00538a88;
}

// FUNCTION: CMR2 0x00421440
BYTE *RallyData_FUN_00421440(int index)
{
    if (g_unk0x00538a84 == 0)
        return NULL;
    return g_routeNodes + index * 0x2c;
}

int RallyData_FUN_00421500(void);
void RallyData_FUN_00421530(int index, int *pOut);

// Progress of the car along the route segment that ends at node pProgress[0]:
// the car position projected on the segment direction, as a 16.16 fraction
// of the segment length (stored in pProgress[4]).
// FUNCTION: CMR2 0x00421230
void FUN_00421230(Car *pCar, int *pProgress)
{
    FixVector dir;
    FixVector end;
    FixVector start;
    FixVector delta;
    int node;
    int prev;
    int length;
    int along;

    node = *pProgress;
    prev = node - 1;
    if (prev < 0)
        prev += g_unk0x00538a84;
    if (!RallyData_FUN_00421500() && node <= 0) {
        pProgress[4] = 0;
        return;
    }
    RallyRoute_GetNodeDirection(&dir, prev);
    RallyData_FUN_00421530(node, (int *)&end);
    RallyData_FUN_00421530(prev, (int *)&start);
    delta.x = end.x - start.x;
    delta.y = end.y - start.y;
    delta.z = end.z - start.z;
    length = FixVecDot(&delta, &dir);
    delta.x = pCar->position.x - start.x;
    delta.y = pCar->position.y - start.y;
    delta.z = pCar->position.z - start.z;
    along = FixVecDot(&delta, &dir);
    pProgress[4] = FixDiv(along, length);
}

// FUNCTION: CMR2 0x00421500
int RallyData_FUN_00421500(void)
{
    return g_unk0x00538a94;
}

// FUNCTION: CMR2 0x00421510
RaceRecord *RallyData_FUN_00421510(int index)
{
    return &g_raceRecords[index];
}

// FUNCTION: CMR2 0x00421530
void RallyData_FUN_00421530(int index, int *pOut)
{
    int x = *(int *)(g_routeNodes + index * 0x2c);
    pOut[1] = 0;
    pOut[0] = x;
    pOut[2] = *(int *)(g_routeNodes + index * 0x2c + 8);
}
// GLOBAL: CMR2 0x00538c94
int g_unk0x00538c94;

// FUNCTION: CMR2 0x004209d0
int RallyData_FUN_004209d0(BYTE *p)
{
    return g_raceRecords[(signed char)p[0xb1a]].field_0x10;
}

// Scales the per-record value at p[0xb1a] to a 0..0x10000 ratio.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00421470
int RallyData_FUN_00421470(BYTE *p)
{
    int value;
    int result;

    if (g_unk0x00538a84 == 0)
        return 0;
    value = g_raceRecords[(signed char)p[0xb1a]].field_0x8;
    if (g_unk0x00538a94 != 0) {
        if (value >= (RallyData_FUN_00406990() & 0xff) * g_unk0x00538a84)
            return 0x10000;
    } else {
        if (value >= g_unk0x00538a84)
            return 0x10000;
    }
    result = (value << 16) / (g_unk0x00538c94 >> 16);
    if (result < 0)
        return 0;
    if (result > 0x10000)
        return 0x10000;
    return result;
}

// Per-category records of 0x650 bytes (runs up to the 0x531350 table).

// Record of the selected entry: the 196-byte entry in championship mode
// (mode 4), otherwise the 0x650-byte category record (NULL when the
// category nibble is 15).
// FUNCTION: CMR2 0x00408400
void *RallyData_GetRecord(BYTE index)
{
    unsigned int category;

    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x0052f3e0 + index * 196;
    RallyData_ValidateIndex(index);
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf)
        return g_unk0x0052fa18 + category * 0x650;
    return NULL;
}

// Returns the 4-bit category of the record, or -1 when it is not usable.
// FUNCTION: CMR2 0x00408500
char RallyData_FUN_00408500(BYTE param1)
{
    unsigned int index;

    if (CGameInfo::FUN_00405d80() == 4) {
        index = param1 & 0xff;
        if (strcmp((char *)(g_unk0x0052f3e0 + index * 196), CMain::m_logFileBlankLine) != 0 && (BYTE)param1 < 8)
            return -1;
        return (char)((*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0xe) & 0xf);
    }
    if ((*(unsigned int *)(g_unk0x00531350 + (param1 & 0xff) * 0x30) & 0x2000) != 0)
        return -1;
    return (char)((*(unsigned int *)(g_unk0x00531350 + (param1 & 0xff) * 0x30) >> 0xe) & 0xf);
}

// Returns bit 26 of the selected rally data (a per-rally flag).
// FUNCTION: CMR2 0x00407e70
unsigned int RallyData_FUN_00407e70(void)
{
    return (g_selectedRallyData >> 26) & 1;
}

// Returns the 0x650-byte record of the id, or NULL when its 0x3c0000 field is
// already full. The record base is 0x30 bytes into the 0x52fa5c table.
// FUNCTION: CMR2 0x00408cb0
BYTE *RallyData_FUN_00408cb0(int index)
{
    unsigned int value;

    RallyData_ValidateIndex(index);
    value = *(unsigned int *)(g_unk0x00531350 + index * 0x30);
    if ((value & 0x3c0000) != 0x3c0000)
        return g_unk0x0052fa5c + ((value >> 0x12) & 0xf) * 0x650 + 0x30;
    return NULL;
}

// FUNCTION: CMR2 0x00407e90
unsigned int RallyData_FUN_00407e90(void)
{
    return (g_selectedRallyData >> 27) & 1;
}

// FUNCTION: CMR2 0x00407ea0
unsigned int RallyData_FUN_00407ea0(void)
{
    return (g_selectedRallyData >> 28) & 1;
}

// Neutral type for now: nothing implemented reads it yet.
// GLOBAL: CMR2 0x00536be0
int g_unk0x00536be0;
// GLOBAL: CMR2 0x00536be4
int g_unk0x00536be4;

// FUNCTION: CMR2 0x0040eeb0
int RallyData_FUN_0040eeb0(void)
{
    return g_unk0x00536be4;
}

// Tail-call thunk (the original compiles it to a bare jmp): clears the
// depth buffer before the on-stage 3D pass.
// FUNCTION: CMR2 0x0040eec0
BOOL RallyData_FUN_0040eec0(void)
{
    return CGraphics::ClearZBuffer();
}

// FUNCTION: CMR2 0x00411060
int RallyData_FUN_00411060(void)
{
    return g_unk0x00536be0;
}

// FUNCTION: CMR2 0x004082e0
unsigned int RallyData_FUN_004082e0(void)
{
    return (g_unk0x0052f2b0 >> 0xb) & 1;
}

// FUNCTION: CMR2 0x004082b0
unsigned int RallyData_FUN_004082b0(void)
{
    return g_unk0x0052f2b0 & 7;
}

// Table of 0x148-byte records; count derived from the next known global
// (0x52f2a9), so it may cover further undeclared values.
struct Unk0x0052ebc0 {
    BYTE field_0x0[0x148];
};

// GLOBAL: CMR2 0x0052ebc0
Unk0x0052ebc0 g_unk0x0052ebc0[5];

// FUNCTION: CMR2 0x00407610
Unk0x0052ebc0 *RallyData_FUN_00407610(int index)
{
    return &g_unk0x0052ebc0[index];
}

// GLOBAL: CMR2 0x00516cd0
BYTE g_itemColour[4] = { 255, 255, 255, 255 };
// GLOBAL: CMR2 0x00536bd8
short g_itemRect[4];

// Builds the frontend scene: root node, camera node and projection.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040ef10
void FUN_0040ef10(void)
{
    FixVector position;
    FixAngles angles;
    SceneNode *pNode;

    position.x = 0;
    position.y = 0;
    position.z = -0xa0000;
    memset(&angles, 0, sizeof(angles));
    g_unk0x00536be0 = (int)SceneNode_CreateRoot();
    g_unk0x00536be4 = (int)SceneType2_Create(&position, &angles, NULL, (SceneNode *)g_unk0x00536be0);
    for (pNode = (SceneNode *)g_unk0x00536be0; pNode != NULL; pNode = *(SceneNode **)((BYTE *)pNode + 8))
        *(int *)((BYTE *)pNode + 0x174) = 1;
    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
}

// Draws one item of a horizontal list and, unless it is the last one, the thin
// separator after it; returns the x the next item starts at.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040fd30
int RallyData_DrawListItem(int x, int y, char *pText, char last, BYTE alpha)
{
    BYTE colour[4];
    int width;

    colour[0] = g_itemColour[0];
    colour[1] = g_itemColour[1];
    colour[2] = g_itemColour[2];
    colour[3] = alpha;
    Font_DrawText(2, pText, x, y, (int *)colour, 0x11);
    if (last == 0) {
        width = Font_GetTextWidth(2, (BYTE *)pText);
        width = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
        g_itemRect[0] = (short)width;
        g_itemRect[2] = 2;
        g_itemRect[1] = (short)((int)(g_pGraphics->resY * 200) / 0x1e0);
        g_itemRect[3] = (short)((int)(g_pGraphics->resY * 0x3c) / 0x1e0);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_itemRect, colour, 1);
        return width + (int)(g_pGraphics->resX * 10) / 0x280;
    }
    return x;
}

// GLOBAL: CMR2 0x0052f0fc
int g_unk0x0052f0fc;
// GLOBAL: CMR2 0x0052f290
int g_unk0x0052f290;

// FUNCTION: CMR2 0x0040df60
void RallyData_FUN_0040df60(int param1, int param2)
{
    g_unk0x0052f290 = param1;
    g_unk0x0052f0fc = param2;
}

unsigned int RallyData_FUN_004070e0(void);

// FUNCTION: CMR2 0x004070c0
void RallyData_FUN_004070c0(void)
{
    unsigned int value = RallyData_FUN_004070e0() & 0xff;

    RallyData_FUN_004068b0((BYTE)value);
    RallyData_FUN_004068e0(10);
}

// FUNCTION: CMR2 0x004070e0
unsigned int RallyData_FUN_004070e0(void)
{
    return g_knockout.state >> 16 & 0xf;
}

// FUNCTION: CMR2 0x00407500
void RallyData_FUN_00407500(BYTE param1)
{
    g_selectedRallyData = ((param1 & 3) << 14) | (g_selectedRallyData & 0xffff3fffU);
}

// FUNCTION: CMR2 0x00407800
void RallyData_FUN_00407800(unsigned int param1)
{
    g_selectedRallyData = ((param1 & 1) << 20) | (g_selectedRallyData & 0xffefffffU);
}

// FUNCTION: CMR2 0x00407e20
unsigned int RallyData_GetFlag21(void)
{
    return g_selectedRallyData >> 21 & 1;
}

// FUNCTION: CMR2 0x00407e30
unsigned int RallyData_GetFlag22(void)
{
    return g_selectedRallyData >> 22 & 1;
}

// FUNCTION: CMR2 0x00407e40
unsigned int RallyData_GetFlag23(void)
{
    return g_selectedRallyData >> 23 & 1;
}

// FUNCTION: CMR2 0x00407e80
unsigned int RallyData_GetFlag31(void)
{
    return g_selectedRallyData >> 31;
}

// FUNCTION: CMR2 0x00407eb0
unsigned int RallyData_GetFlag30(void)
{
    return g_selectedRallyData >> 30 & 1;
}

// FUNCTION: CMR2 0x00408290
void RallyData_FUN_00408290(void)
{
    memset(g_unk0x0052ebc0, 0, 4 * sizeof(Unk0x0052ebc0));
}

// FUNCTION: CMR2 0x004082c0
unsigned int RallyData_FUN_004082c0(void)
{
    return g_unk0x0052f2b0 >> 7 & 0xf;
}

// FUNCTION: CMR2 0x004082d0
unsigned int RallyData_FUN_004082d0(void)
{
    return g_unk0x0052f2b0 >> 3 & 0xf;
}

// FUNCTION: CMR2 0x004082f0
BYTE *RallyData_FUN_004082f0(void)
{
    return g_unk0x0052ea68;
}

// FUNCTION: CMR2 0x004083d0
Unk0x0052ebc0 *RallyData_FUN_004083d0(void)
{
    return g_unk0x0052ebc0;
}

// FUNCTION: CMR2 0x004084c0
void RallyData_FUN_004084c0(BYTE index, BYTE param2)
{
    *(unsigned int *)&g_unk0x00531350[index * 0x30] =
        ((param2 & 0xf) << 14) | (*(unsigned int *)&g_unk0x00531350[index * 0x30] & 0xfffc0000U);
    *(unsigned int *)&g_unk0x00531350[index * 0x30 + 4] = 0;
}

// FUNCTION: CMR2 0x0040df80
void RallyData_FUN_0040df80(int index, int value)
{
    g_unk0x0052ea98[index] = value;
}

// GLOBAL: CMR2 0x005337c8
int g_unk0x005337c8;
// GLOBAL: CMR2 0x005337d0
int g_unk0x005337d0;
// GLOBAL: CMR2 0x005337ec
int g_unk0x005337ec;
// The event table's slot 16 holds the event the other five slots are built from.
extern int g_unk0x00533758[21];

// Fills the six per-slot values of the current event: a fixed set when the
// event's group is one of the special ones, otherwise the events picked from
// the event's group avoiding the previous one.
// FUNCTION: CMR2 0x0040dfa0
void FUN_0040dfa0(void)
{
    if (!RallyData_FUN_004069a0())
        return;
    g_unk0x005337ec = (int)CFrontend::FUN_0040ee90(RallyData_FUN_004086b0(0));
    g_unk0x00533758[16] = RallyData_FUN_004086b0(0);
    if (g_unk0x005337ec >= 0 && g_unk0x005337ec <= 5 || g_unk0x005337ec == 0xc) {
        if (CGameInfo::FUN_00406320()) {
            RallyData_FUN_0040df80(0, 0);
            RallyData_FUN_0040df80(3, 0);
            RallyData_FUN_0040df80(1, 8);
            RallyData_FUN_0040df80(4, 8);
            RallyData_FUN_0040df80(2, 10);
            RallyData_FUN_0040df80(5, 10);
            return;
        }
        g_unk0x005337d0 = 0;
        RallyData_FUN_0040df80(0, g_unk0x00533758[16]);
        RallyData_FUN_0040df80(3, g_unk0x00533758[16]);
        g_unk0x005337c8 = FUN_0040e180(g_unk0x00533758[16], -1);
        RallyData_FUN_0040df80(1, g_unk0x005337c8);
        RallyData_FUN_0040df80(4, g_unk0x005337c8);
        g_unk0x005337c8 = FUN_0040e180(g_unk0x00533758[16], g_unk0x005337c8);
    } else {
        if (g_unk0x005337ec == 8) {
            RallyData_FUN_0040df80(0, 0x10);
            RallyData_FUN_0040df80(3, 0x10);
            RallyData_FUN_0040df80(1, 0x10);
            RallyData_FUN_0040df80(4, 0x10);
            RallyData_FUN_0040df80(2, 0x10);
            RallyData_FUN_0040df80(5, 0x10);
            return;
        }
        if (g_unk0x005337ec == 0xd) {
            RallyData_FUN_0040df80(0, 0x15);
            RallyData_FUN_0040df80(3, 0x15);
            RallyData_FUN_0040df80(1, 0x15);
            RallyData_FUN_0040df80(4, 0x15);
            RallyData_FUN_0040df80(2, 0x15);
            RallyData_FUN_0040df80(5, 0x15);
            return;
        }
        RallyData_FUN_0040df80(0, g_unk0x00533758[16]);
        RallyData_FUN_0040df80(3, g_unk0x00533758[16]);
        g_unk0x005337c8 = FUN_0040e210(g_unk0x00533758[16], -1);
        RallyData_FUN_0040df80(1, g_unk0x005337c8);
        RallyData_FUN_0040df80(4, g_unk0x005337c8);
        g_unk0x005337c8 = FUN_0040e210(g_unk0x00533758[16], g_unk0x005337c8);
    }
    RallyData_FUN_0040df80(2, g_unk0x005337c8);
    RallyData_FUN_0040df80(5, g_unk0x005337c8);
}

// GLOBAL: CMR2 0x005337c4
int g_unk0x005337c4;
// GLOBAL: CMR2 0x005337d4
int g_unk0x005337d4[6];

// Picks a random stage (0..11) whose group is not one of the two excluded ones.
// FUNCTION: CMR2 0x0040e180
int FUN_0040e180(int exclude1, int exclude2)
{
    memset(g_unk0x005337d4, 0, sizeof(g_unk0x005337d4));
    if (exclude1 >= 0)
        g_unk0x005337d4[(int)CFrontend::FUN_0040ee90(exclude1)] = 1;
    if (exclude2 >= 0)
        g_unk0x005337d4[(int)CFrontend::FUN_0040ee90(exclude2)] = 1;
    g_unk0x005337c4 = rand() % 12;
    while (g_unk0x005337d4[(int)CFrontend::FUN_0040ee90(g_unk0x005337c4)] == 1)
        g_unk0x005337c4 = rand() % 12;
    return g_unk0x005337c4;
}

// FUNCTION: CMR2 0x0040e330
void RallyData_FUN_0040e330(char param1)
{
    if (param1 != 0) {
        g_unk0x0052f2b0 |= 0x800;
        return;
    }
    g_unk0x0052f2b0 &= ~0x800;
}

// FUNCTION: CMR2 0x0040e360
void RallyData_FUN_0040e360(unsigned int param1)
{
    g_unk0x0052f2b0 = g_unk0x0052f2b0 ^ (g_unk0x0052f2b0 ^ param1) & 7;
}

// FUNCTION: CMR2 0x0040e380
void RallyData_FUN_0040e380(unsigned int param1)
{
    g_unk0x0052f2b0 = ((param1 & 0xf) << 7) | (g_unk0x0052f2b0 & 0xfffff87fU);
}

// FUNCTION: CMR2 0x0040e3a0
void RallyData_FUN_0040e3a0(unsigned int param1)
{
    g_unk0x0052f2b0 = ((param1 & 0xf) << 3) | (g_unk0x0052f2b0 & 0xffffff87U);
}

// GLOBAL: CMR2 0x0051681c
BYTE g_unk0x0051681c[15] = { 0x00, 0x09, 0x0a, 0x08, 0x04, 0x04, 0x09, 0x00,
                             0x0a, 0x0b, 0x08, 0x0b, 0x04, 0x00, 0x0a };

// Row/slot of the driver select screen for one list index: the rally modes
// (5..7) only have six slots and ask the AI helper for the row, everything
// else uses a 15-entry lookup table.
// FUNCTION: CMR2 0x00407fc0
BYTE FUN_00407fc0(int param1)
{
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7) {
        if (param1 >= 0 && param1 < 6)
            return FUN_00456c00(param1);
        return 0;
    }
    if (param1 < 0)
        return 0;
    if (param1 >= 0xf)
        return 0;
    return g_unk0x0051681c[param1];
}

// FUNCTION: CMR2 0x00408010
int RallyData_FUN_00408010(int index)
{
    return g_unk0x0052ea98[index];
}

// Builds the sorted list of distinct values from the 30 entries of the table
// at 0x4075f0 (shifting at most the first 9).
// match 26%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004081d0
void FUN_004081d0(void)
{
    int *pValues;
    int *pSorted = g_unk0x0052ea74Block;
    int count;
    int remaining;
    int value;
    int pos;
    int i;

    pValues = (int *)RallyData_FUN_004075f0();
    count = 1;
    g_unk0x0052eab8 = 1;
    pSorted[0] = *pValues;
    for (remaining = 30; remaining != 0; remaining--, pValues++) {
        value = *pValues;
        pos = -1;
        for (i = 0; i < count; i++) {
            if (value <= pSorted[i]) {
                pos = i;
                break;
            }
        }
        if (pos == -1) {
            pSorted[count] = value;
            count++;
            g_unk0x0052eab8 = count;
        } else if (pSorted[pos] != value) {
            if (pos < 8) {
                for (i = 8; i > pos; i--)
                    pSorted[i] = pSorted[i - 1];
            }
            pSorted[pos] = value;
            count++;
            g_unk0x0052eab8 = count;
        }
    }
}

// FUNCTION: CMR2 0x00408270
BYTE *RallyData_FUN_00408270(void)
{
    return g_unk0x0052ea74;
}

// FUNCTION: CMR2 0x00408280
int RallyData_FUN_00408280(void)
{
    return g_unk0x0052eab8;
}

void FUN_00477f30(void);
void FUN_00477a90(void);
void StageObject_FreeAll(void);
void Mesh_FreeClones(void);

// FUNCTION: CMR2 0x00411110
void FUN_00411110(void)
{
    FUN_00477f30();
    FUN_00477a90();
}

// Frees the stage objects, the stage root node and the cloned meshes.
// FUNCTION: CMR2 0x0040eef0
BYTE FUN_0040eef0(void)
{
    StageObject_FreeAll();
    if (g_unk0x00536be0 != 0)
        SceneNode_Destroy((SceneNode *)g_unk0x00536be0);
    Mesh_FreeClones();
    return 1;
}

short *Car_GetOrder(void);
short Car_GetOrderCount(void);

// Updates the route of every car, last in the race order first.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004209f0
void FUN_004209f0(void)
{
    short *pOrder;
    int i;

    pOrder = Car_GetOrder();
    i = Car_GetOrderCount();
    if (i - 1 >= 0) {
        pOrder += i - 1;
        do {
            RallyData_UpdateCarRoute(Car_Get(*pOrder));
            pOrder--;
            i--;
        } while (i != 0);
    }
}

// Whether flag bit `bit` is set for the entry that pEntry points into.
// FUNCTION: CMR2 0x00471d40
int FUN_00471d40(BYTE **pEntry, int bit)
{
    unsigned int index;
    BYTE mask;

    index = (unsigned int)(*pEntry - g_unk0x0058c94c) >> 3;
    if ((int)index < (int)g_unk0x0058ca6c && (int)index >= 0) {
        mask = 1 << bit;
        return (g_unk0x0058c938[index] & mask) != 0;
    }
    return 0;
}

// Sets or clears bit `bit` of the flag byte of the 8-byte element *pp points at.
// FUNCTION: CMR2 0x00471d80
void FUN_00471d80(BYTE **pp, int bit, int set)
{
    unsigned int index = (unsigned int)(*pp - g_unk0x0058c94c) >> 3;
    BYTE mask;

    if ((int)index < (int)g_unk0x0058ca6c && (int)index >= 0) {
        mask = 1 << bit;
        if (set == 0) {
            g_unk0x0058c938[index] &= ~mask;
            return;
        }
        g_unk0x0058c938[index] |= mask;
    }
}

unsigned int RallyDataState(void);

// Marks the element as reached by the car; the first time, in single player,
// pushes its object out of the way.
// FUNCTION: CMR2 0x00470240
void FUN_00470240(BYTE **pElement, int car)
{
    if (FUN_00471d40((BYTE **)&pElement, car) == 0) {
        FUN_00471d80((BYTE **)&pElement, car, 1);
        if (Car_Get(car)->field_0xc0c == 0 && (BYTE)RallyDataState() == 1) {
            *(int *)(*pElement + 4) += 0x3e80000;
            (*pElement)[0x14] = 0xff;
        }
    }
}

// Row of the 7-byte table g_unk0x00520128 for the current country.
// match 18%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00494a70
BYTE *FUN_00494a70(void)
{
    switch ((BYTE)RallyDataCountryIndex()) {
    case 3:
        return g_unk0x00520128 + 7;
    case 5:
        return g_unk0x00520128 + 14;
    case 7:
        return g_unk0x00520128 + 21;
    case 8:
        return g_unk0x00520128 + 28;
    }
    return g_unk0x00520128;
}

unsigned int FUN_00471bd0(BYTE **pOut);
void FUN_0046f7e0(void);

// Marks every element as reached by every car (start of a replay).
// FUNCTION: CMR2 0x004702a0
void FUN_004702a0(void)
{
    BYTE *pEntries;
    int count;
    int car;
    int i;

    count = FUN_00471bd0(&pEntries);
    for (car = 0; car < Car_GetOrderCount(); car++)
        for (i = 0; i < count; i++)
            FUN_00470240((BYTE **)(pEntries + i * 8), car);
    FUN_0046f7e0();
}

// Sets bit `bit` of the driver's category award mask; returns 0 when the
// driver has no category or already had it.
// FUNCTION: CMR2 0x00409010
BYTE RallyData_FUN_00409010(int index, int bit)
{
    unsigned int category;
    unsigned int mask;

    RallyData_ValidateIndex(index);
    if ((*(unsigned int *)(g_unk0x00531350 + index * 0x30) & 0x3c0000) == 0x3c0000)
        return 0;
    category = (*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf;
    mask = 1 << bit;
    if ((mask & *(unsigned int *)(g_unk0x0052f3e8 + 0x66c + category * 0x650)) != 0)
        return 0;
    *(unsigned int *)(g_unk0x0052f3e8 + 0x66c + category * 0x650) |= mask;
    g_unk0x0052f3e8[0x618 + category] = 1;
    RallyData_FUN_00408f20(index);
    return 1;
}

// Records the best finishing place (3 - place) of the driver's category at
// the current difficulty.
// FUNCTION: CMR2 0x00409090
void RallyData_FUN_00409090(int index, int place)
{
    unsigned int *pBest;
    unsigned int value;

    RallyData_ValidateIndex(index);
    pBest = (unsigned int *)(g_unk0x0052f3e8 + 0xc4c +
                             ((*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf) * 0x650);
    value = 3 - place;
    switch (CGameInfo::FUN_00405d90()) {
    case 0:
        if ((*pBest & 3) < value) {
            *pBest = ((*pBest ^ value) & 3) ^ *pBest;
            RallyData_IncrementCategoryUse(index);
        }
        break;
    case 1:
        if (((*pBest >> 2) & 3) < value) {
            *pBest = ((value & 3) << 2) | (*pBest & 0xfffffff3);
            RallyData_IncrementCategoryUse(index);
        }
        break;
    case 2:
        if (((*pBest >> 4) & 3) < value) {
            *pBest = ((value & 3) << 4) | (*pBest & 0xffffffcf);
            RallyData_IncrementCategoryUse(index);
        }
        break;
    }
}

// Split position of each player (index, 16.16 fraction to the next split).
// GLOBAL: CMR2 0x00536c48
int g_unk0x00536c48[8][2];
// Three split rows shown around each player's rank.
// GLOBAL: CMR2 0x00536c94
int g_unk0x00536c94[2][3];

int RallyData_FUN_004209d0(BYTE *p);
int FUN_004582f0(int index);
int FUN_00409d20(int index);
int FUN_0040a760(int id);
int FUN_0040a720(int id);
int FUN_0040b010(int index);
unsigned int FUN_00409cb0(int index);
void FUN_00411e40(int *pOut, int distance, int percent);

// Updates the split position of the player and of every network player.
// FUNCTION: CMR2 0x00411f00
void FUN_00411f00(void)
{
    int i;
    int percent;

    percent = RallyData_FUN_004209d0((BYTE *)Car_Get(0)) * 100 >> 16;
    FUN_00411e40(g_unk0x00536c48[0], FUN_004582f0(0), percent);
    for (i = 0; i < 7; i++) {
        if ((BYTE)FUN_00409cb0(i))
            FUN_00411e40(g_unk0x00536c48[FUN_0040b010(i)], FUN_0040a720(FUN_00409d20(i)),
                         FUN_0040a760(FUN_00409d20(i)));
    }
}

BYTE StageTiming_FUN_00455ae0(void);
int StageTiming_GetSplitPositionOfDriver(int iDriver, int iSplit);
int StageTiming_GetSplitDriverCount(int iSplit);

// Picks the three split rows to show around the player's rank.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00414720
void FUN_00414720(int car)
{
    int position;

    if (g_stageSplitData[car].split == 0)
        return;
    if (!StageTiming_FUN_00455ae0()) {
        position = 15;
    } else {
        position = StageTiming_GetSplitPositionOfDriver((FUN_0041b370() & 0xff) + car, g_stageSplitData[car].split);
        if (position < 1) {
            g_unk0x00536c94[car][0] = position;
            g_unk0x00536c94[car][1] = position + 1;
            g_unk0x00536c94[car][2] = position + 2;
            return;
        }
    }
    if (StageTiming_GetSplitDriverCount(g_stageSplitData[car].split) - 1 <= position) {
        g_unk0x00536c94[car][0] = position - 2;
        g_unk0x00536c94[car][1] = position - 1;
        g_unk0x00536c94[car][2] = position;
        return;
    }
    g_unk0x00536c94[car][0] = position - 1;
    g_unk0x00536c94[car][1] = position;
    g_unk0x00536c94[car][2] = position + 1;
}

BYTE FUN_004582b0(int index);
void FUN_00415a60(int car);

// Counts down the car's split display and, when it has improved, records the
// best time of the stage with the driver's name.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00415990
void FUN_00415990(int car)
{
    if (g_unk0x00536c08[car] != 0)
        g_unk0x00536c08[car]--;
    if (g_unk0x00536c08[car] == 0) {
        g_unk0x00536c00[car] = 0;
        g_unk0x00536c88[car] = 0;
        g_unk0x00536e88[car] = 0;
        g_unk0x00536c28[car] = 0;
        g_unk0x00536ec4[car] = 0;
    }
    if (!(BYTE)RallyData_GetFlag24() &&
        (!(BYTE)RallyData_GetFlag25() || CGameInfo::FUN_00405d80() != 3 || CGameInfo::FUN_00405e00())) {
        if (FUN_004582b0(car) && (g_unk0x0053706c == 0 || g_unk0x0053706c > g_stageSplitData[car].times[0])) {
            g_unk0x0053706c = g_stageSplitData[car].times[0];
            sprintf(g_unk0x00536ed0, (char *)RallyData_GetRecord(FUN_0041b370() + (char)car));
        }
    } else {
        FUN_00415a60(car);
    }
}

BYTE FUN_00458250(int index);
void FUN_00427990(int index, int value);
unsigned int FUN_0040a410(int split);
void FUN_00411ab0(BYTE param1, int param2);

// On passing a split: stores the split time, fetches the time to beat and
// starts the split display (ahead/behind) for the car.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00413520
void FUN_00413520(int car)
{
    int split;
    int target;

    g_unk0x00536e90[0] = 0;
    if (FUN_00458250(car) == 0) {
        if (g_unk0x00536c14 == 0)
            return;
    } else {
        split = FUN_00458370(car);
        g_unk0x00536ed8 = split;
        if (split > 0 && split < 9) {
            g_stageSplitData[car].times[split + 1] = g_stageSplitData[car].times[0];
            g_stageSplitData[car].lastSplitTime = g_stageSplitData[car].times[0];
            g_stageSplitData[car].split = split;
            FUN_00427990(split, g_stageSplitData[car].times[0]);
        }
        g_unk0x00536c14 = 1;
    }
    target = FUN_0040a410(g_unk0x00536ed8);
    g_stageSplitData[car].targetTime = target;
    if (g_stageSplitData[car].lastSplitTime < target) {
        FUN_00411ab0((BYTE)car, 1);
    } else {
        if (target == 0) {
            g_unk0x00536e90[0] = 1;
            goto show;
        }
        FUN_00411ab0((BYTE)car, 0);
    }
    g_unk0x00536c14 = 0;
show:
    if (g_unk0x00536ed8 != 0) {
        ((BYTE *)g_unk0x005170e0)[car * 4 + 3] = 0xff;
        g_unk0x00536c20[car] = 0x4b;
    }
}

// Whether a driver's category has award `bit` (with `check`, some cheats
// count as having it).
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00408e30
bool RallyData_FUN_00408e30(int index, int bit, char check)
{
    RallyData_ValidateIndex(index);
    if ((*(unsigned int *)(g_unk0x00531350 + index * 0x30) & 0x3c0000) == 0x3c0000)
        return false;
    if (check != 0) {
        if (CGameInfo::FUN_00406410(0xc))
            return true;
        if (bit == 7) {
            if (CGameInfo::FUN_00406410(0) || CGameInfo::FUN_00406410(0x12))
                return true;
        } else if (bit == 0x10) {
            if (CGameInfo::FUN_00406410(1))
                return true;
        } else if (bit == 0xf) {
            if (CGameInfo::FUN_00406410(2))
                return true;
        } else if (bit == 0x14) {
            if (CGameInfo::FUN_00406410(3))
                return true;
        }
    }
    return (*(unsigned int *)(g_unk0x0052f3e8 + 0x66c +
                              ((*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf) * 0x650) &
            (1 << bit)) != 0;
}

// GLOBAL: CMR2 0x0058c944
void *g_unk0x0058c944;
// GLOBAL: CMR2 0x0058c948
void *g_unk0x0058c948;
// GLOBAL: CMR2 0x0058ca70
void *g_unk0x0058ca70;

#define FREE_AND_CLEAR(p)                            \
    if ((p) != NULL) {                               \
        CFileBuffer::FreeGenericFileBuffer((void *)(p)); \
        (p) = NULL;                                  \
    }

// Releases the sector lists and element tables (registered callback of 0x471dd0).
// FUNCTION: CMR2 0x00472720
int FUN_00472720(void)
{
    FREE_AND_CLEAR(g_unk0x0058c948)
    FREE_AND_CLEAR(g_unk0x0058c958)
    FREE_AND_CLEAR(g_unk0x0058ca70)
    FREE_AND_CLEAR(g_sectorListEntriesA)
    FREE_AND_CLEAR(g_unk0x0058c944)
    FREE_AND_CLEAR(g_sectorListIndexA)
    FREE_AND_CLEAR(g_sectorListCountA)
    FREE_AND_CLEAR(g_unk0x0058c94c)
    FREE_AND_CLEAR(g_sectorListCountB)
    FREE_AND_CLEAR(g_sectorListIndexB)
    FREE_AND_CLEAR(g_sectorListEntriesB)
    FREE_AND_CLEAR(g_unk0x0058c938)
    return 1;
}

BYTE *FUN_00464af0(int index);
struct StageTableEntry;
StageTableEntry *FUN_00464b00(int index);
void FUN_00464b60(void);
int FUN_0041f3a0(void);

// Screen rectangle of a view: full screen, or the half chosen by the split
// direction (vertical or horizontal).
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040f050
int *FUN_0040f050(int view)
{
    int *pRect;
    int *pSource;
    int which;

    FUN_00464b60();
    pRect = (int *)FUN_00464af0(view);
    if ((BYTE)RallyDataState() != 1 && FUN_0041f3a0() == 0) {
        if (view == 0)
            which = CGameInfo::FUN_00405dc0() ? 1 : 3;
        else
            which = CGameInfo::FUN_00405dc0() ? 2 : 4;
    } else {
        which = 0;
    }
    pSource = (int *)FUN_00464b00(which);
    pRect[0] = pSource[0];
    pRect[1] = pSource[1];
    return pRect;
}

extern int g_unk0x00536fe0;
int FUN_00448330(int car);

// GLOBAL: CMR2 0x00536c30
int g_unk0x00536c30[3];

// Updates the split display rows for a car.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00415870
void FUN_00415870(int car)
{
    int i;
    int position;

    if (RallyData_GetFlag24() || RallyData_GetFlag25()) {
        for (i = 0; i < 3; i++) {
            if (g_unk0x00536c30[i] > 0)
                g_unk0x00536c30[i]--;
        }
        if (g_unk0x00536fe0 == 0) {
            for (i = 0; i < 3; i++)
                g_unk0x00536c94[car][i] = i;
            g_stageSplitData[car].position = FUN_004481c0(car);
        } else {
            for (i = 0; i < 3; i++)
                g_unk0x00536c94[car][i] = -1;
            position = FUN_00448330(car);
            g_stageSplitData[car].position = position;
            if (position != -1) {
                if (position <= 0) {
                    g_unk0x00536c94[car][0] = position;
                    g_unk0x00536c94[car][1] = position + 1;
                    g_unk0x00536c94[car][2] = position + 2;
                } else if (position < 5) {
                    g_unk0x00536c94[car][0] = position - 1;
                    g_unk0x00536c94[car][1] = position;
                    g_unk0x00536c94[car][2] = position + 1;
                } else {
                    g_unk0x00536c94[car][0] = position - 2;
                    g_unk0x00536c94[car][1] = position - 1;
                    g_unk0x00536c94[car][2] = position;
                }
            }
        }
    }
    if ((BYTE)CGameInfo::FUN_00405d80() == 5 || (BYTE)CGameInfo::FUN_00405d80() == 6)
        FUN_00458290(car);
    else if (FUN_00458250(car))
        g_unk0x00536c20[car] = 0x4b;
}

// GLOBAL: CMR2 0x00533758
int g_unk0x00533758[21];
// GLOBAL: CMR2 0x005337cc
int g_unk0x005337cc;

// Picks an unexcluded event from the three or four events in its group.
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040e210
int FUN_0040e210(int exclude1, int exclude2)
{
    int *choices = &g_unk0x00533758[17];

    if (exclude1 != 0x13 && exclude1 != 0x11 && exclude1 != 0x12) {
        choices[0] = 0;
        choices[1] = 0;
        choices[2] = 0;
        choices[3] = 0;
        if (exclude1 >= 0)
            g_unk0x00533758[5 + exclude1] = 1;
        if (exclude2 >= 0)
            g_unk0x00533758[5 + exclude2] = 1;
        do {
            g_unk0x005337cc = rand() % 4;
        } while (choices[g_unk0x005337cc] == 1);
        return g_unk0x005337cc + 12;
    }
    choices[0] = 0;
    choices[1] = 0;
    choices[2] = 0;
    if (exclude1 >= 0)
        g_unk0x00533758[exclude1] = 1;
    if (exclude2 >= 0)
        g_unk0x00533758[exclude2] = 1;
    do {
        g_unk0x005337cc = rand() % 3;
    } while (choices[g_unk0x005337cc] == 1);
    return g_unk0x005337cc + 0x11;
}

int FUN_004481e0(int index);
int StageTiming_FUN_00455ac0(int split, int index);
int StageTiming_GetSplitDriverIDForPosition(int position, int split);
int StageTiming_GetSplitPositionOfDriver(int driver, int split);
unsigned int FUN_004735a0(unsigned int *pHigh);

// Formats the name shown for a driver in the current result list.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00415750
void FUN_00415750(int driver, int split, int useLongName, int useSplit)
{
    int entry;
    int row;
    char *name;

    sprintf(CFrontend::m_stringDest, CMain::m_logFileBlankLine);
    if (useSplit == 0)
        entry = StageTiming_FUN_00455ac0(FUN_0041b370(), FUN_004481e0(driver));
    else
        entry = StageTiming_GetSplitDriverIDForPosition(driver, split);
    if (entry == -1) {
        sprintf(CFrontend::m_stringDest, CMain::m_logFileBlankLine);
        return;
    }
    for (row = 0; row < CGameInfo::FUN_00405d70(); row++) {
        int candidate = useSplit == 0 ? FUN_004481c0(row) : StageTiming_GetSplitPositionOfDriver(row, split);
        if (candidate == driver) {
            if (CGameInfo::FUN_00405d80() == 4)
                name = (char *)RallyData_GetRecord((BYTE)FUN_004735a0((unsigned int *)entry));
            else
                name = (char *)RallyData_GetRecord((BYTE)row);
            sprintf(CFrontend::m_stringDest, name);
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            return;
        }
    }
    if (CGameInfo::FUN_00405d80() == 4)
        entry = FUN_004735a0((unsigned int *)entry);
    if (useLongName)
        name = (char *)RallyData_FUN_00407f20(entry);
    else
        name = CAIHelper::GetNameForID(entry);
    sprintf(CFrontend::m_stringDest, name);
    CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
}

int FUN_00448260(int car);
int FUN_00448110(void);
int FUN_00448330(int car);
int FUN_00448350(int car);
BYTE FUN_00448370(int car);
int FUN_004482d0(int index, int car);

// GLOBAL: CMR2 0x00536c18
int g_unk0x00536c18[2];
// GLOBAL: CMR2 0x00536fe4
int g_unk0x00536fe4[2];
// GLOBAL: CMR2 0x00537054
int g_unk0x00537054[2];

// Updates the split timer and stage sound for a player.
// FUNCTION: CMR2 0x00413200
void FUN_00413200(int car)
{
    if (FUN_00458250(car)) {
        g_unk0x00537054[car] = 1;
        g_unk0x00536fe4[car] = FUN_00448110() - FUN_00448260(car);
        if (FUN_00448330(car) > 0) {
            FUN_00411ab0(car, 0);
            ((BYTE *)g_unk0x005170e0)[car * 4 + 3] = 0xff;
            g_unk0x00536c20[car] = 0x4b;
            g_unk0x00536c0c[car] = 0;
            g_unk0x00536c18[car] = 0;
            g_stageSplitData[car].split = 1;
        } else {
            FUN_00411ab0(car, 1);
            g_unk0x00536c18[car] = 1;
            g_unk0x00536c0c[car] = 1;
            g_stageSplitData[car].split = 1;
        }
    }
    if (FUN_00448370(car)) {
        if (g_unk0x00536c18[car] == 0)
            return;
        g_unk0x00537054[car] = 1;
        g_unk0x00536c0c[car] = 0;
        int index = FUN_00448350(car);
        int now = FUN_00448110();
        g_unk0x00536fe4[car] = FUN_004482d0(index, car) - now;
        ((BYTE *)g_unk0x005170e0)[car * 4 + 3] = 0xff;
        g_unk0x00536c20[car] = 0x4b;
        g_stageSplitData[car].split = 1;
        g_unk0x00536c18[car] = 0;
    }
    if (g_unk0x00536c18[car] != 0 && g_unk0x00537054[car] != 0) {
        ((BYTE *)g_unk0x005170e0)[car * 4 + 3] = 0xff;
        g_unk0x00536c20[car] = 0x4b;
    }
}



/* --------------------------------------------------------------------------
   Rally-data input dispatch and per-category difficulty records.
   -------------------------------------------------------------------------- */

void FUN_00413200(int car);
void FUN_00413330(int car);
void FUN_00413520(int car);
void FUN_00413610(int car);
void RallyData_IncrementCategoryUse(int index);
void FUN_0049de40(void);

// Notifies the menu handler when the received value is one below the key code.
// FUNCTION: CMR2 0x0040eed0
void FUN_0040eed0(BYTE *pKey, int value)
{
    if ((value & 0xff) == *pKey - 1)
        FUN_0049de40();
}

// Dispatches the current game state to the handler of the active game mode.
// FUNCTION: CMR2 0x00413160
void FUN_00413160(int car)
{
    if (CGameInfo::FUN_00405e00() != 0) {
        switch (CGameInfo::FUN_00405d80()) {
        case 8:
        case 9:
        case 10:
            FUN_00413520(car);
            break;
        case 11:
        case 12:
            FUN_00413610(car);
            break;
        }
    } else if (CGameInfo::FUN_00405d80() != 6 && CGameInfo::FUN_00405d80() != 5) {
        if ((BYTE)RallyData_FUN_00407e90() != 0)
            FUN_00413200(car);
        else
            FUN_00413330(car);
    } else {
        FUN_00413200(car);
    }
}

// Sets the 2-bit difficulty field of the category of a rally entry, keeping the
// best value seen so far. mode selects the game mode, player the player slot.
// FUNCTION: CMR2 0x00409680
void FUN_00409680(int index, int player, int difficulty)
{
    unsigned int *pRecord;
    unsigned char mode;
    unsigned int level;

    RallyData_ValidateIndex(index);
    pRecord = (unsigned int *)(g_unk0x0052f3e8 + 0xc4c +
        ((*(unsigned int *)(g_unk0x00531350 + index * 0x30) >> 0x12) & 0xf) * 0x650);
    level = 3 - difficulty;
    mode = CGameInfo::FUN_00405d90();
    switch (mode) {
    case 0:
        if (level > (*pRecord >> 6 & 3)) {
            *pRecord = (level & 3) << 6 | *pRecord & 0xffffff3f;
            RallyData_IncrementCategoryUse(index);
        }
        break;
    case 1:
        if (player == 0) {
            if (level > (*pRecord >> 8 & 3)) {
                *pRecord = (level & 3) << 8 | *pRecord & 0xfffffcff;
                RallyData_IncrementCategoryUse(index);
                return;
            }
        } else if (level > (*pRecord >> 0xc & 3)) {
            *pRecord = (level & 3) << 0xc | *pRecord & 0xffffcfff;
            RallyData_IncrementCategoryUse(index);
            return;
        }
        break;
    case 2:
        if (player == 0) {
            if (level > (*pRecord >> 10 & 3)) {
                *pRecord = (level & 3) << 10 | *pRecord & 0xfffff3ff;
                RallyData_IncrementCategoryUse(index);
                return;
            }
        } else if (level > (*pRecord >> 0xe & 3)) {
            *pRecord = (level & 3) << 0xe | *pRecord & 0xffff3fff;
            RallyData_IncrementCategoryUse(index);
            return;
        }
        break;
    }
}

/* Sound samples used by the rally-data screens (original .data at 0x517d00). */
// GLOBAL: CMR2 0x00517d00
char g_strTimeoutWav[12] = "timeout.wav";
// GLOBAL: CMR2 0x00517d0c
char g_strCrossWav[12] = "cross.wav";
// GLOBAL: CMR2 0x00517d18
char g_strBlipWav[12] = "blip.wav";
// GLOBAL: CMR2 0x00517d24
char g_strDownWav[12] = "down.wav";
// GLOBAL: CMR2 0x00517d30
char g_strUpWav[8] = "up.wav";

struct StageFile;
int FUN_004b7780(void);
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);
StageFile *StageTiming_GetStageFile0(void);

// Loads the navigation sound samples of the rally-data screens.
// FUNCTION: CMR2 0x00411120
void FUN_00411120(void)
{
    g_unk0x00536ecc = FUN_004b7780();
    Sound_LoadSample(g_strUpWav, 0, (GenericFile *)StageTiming_GetStageFile0());
    g_unk0x00537060 = FUN_004b7780();
    Sound_LoadSample(g_strDownWav, 0, (GenericFile *)StageTiming_GetStageFile0());
    g_unk0x00536cb0 = FUN_004b7780();
    Sound_LoadSample(g_strBlipWav, 0, (GenericFile *)StageTiming_GetStageFile0());
    Sound_LoadSample(g_strCrossWav, 0, (GenericFile *)StageTiming_GetStageFile0());
    Sound_LoadSample(g_strTimeoutWav, 0, (GenericFile *)StageTiming_GetStageFile0());
}

struct Menu;
void Menu_CallCallback2(Menu *pMenu);
int FUN_0041f4b0(void);
BYTE *FUN_00475f70(void);
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);
void FUN_0049d3f0(int, int, void *, int, int);
void FUN_0049de40(void);

// Sets up the projection, clears the targets and draws the challenge scene.
// FUNCTION: CMR2 0x00411070
void FUN_00411070(int param1, int param2)
{
    short rect[4];

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = *(short *)g_pGraphics;
    rect[3] = *(short *)((BYTE *)g_pGraphics + 4);
    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    if (FUN_0041f4b0() == 0)
        Menu_CallCallback2((Menu *)FUN_00475f70());
    Game_PrepareScene((SceneNode *)g_unk0x00536be0, (SceneNode *)g_unk0x00536be4, (int)rect, 0);
    FUN_0049d3f0(g_unk0x00536be0, g_unk0x00536be4, rect, 0, 1);
    FUN_0049de40();
}


extern char g_strFpsFormat0x00516e14[10];

void Scene_BeginShadowBatch(void);
void Scene_EndShadowBatch(void);
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void FUN_004b5ee0(SceneNode *pNode, int param2, BYTE param3);
void FUN_004b5f90(SceneNode *pNode, int radius, short *pSector);
void FUN_00462aa0(unsigned int param1, int param2);
int FUN_00423f30(void);
BYTE FUN_00422fb0(unsigned int index);
void FUN_00428bf0(unsigned int view, short *pRect);
void FUN_00466570(short *param1, short param2, int param3, int param4);
void FUN_00464960(unsigned int param1);
BYTE FUN_0046bd40(int index);
void FUN_0046b8f0(Car *pCar);
void FUN_0046b670(BYTE *pCar);
void FUN_0046e440(void);
void FUN_0046e780(void);
void FUN_0046ea10(int param1);
void FUN_0046f330(int param1);
void FUN_00465780(int param1);
void FUN_004657d0(int car);
void StageObject_UpdateSkidTrails(int carIndex);
void FUN_00461c30(int index);
void FUN_00466100(int param1);
void FUN_0046bb40(void);
unsigned int FUN_0040b1e0(int index);
void SceneNode_SetViewMaskTree(SceneNode *pNode, BYTE mask);
int FUN_0049e1f0(SceneNode *pNode, int bit);
void FUN_004b2970(int value);
float FUN_004b23a0(void);
void FUN_00459790(int param1, int param2);
void FUN_004593a0(void);
void FUN_00422d40(unsigned int player);
void FUN_00471a60(int param1);
void FUN_004177d0(unsigned int player, int param2);
void Dash_Draw(int player, int layer);
void FUN_00417e60(void);
int FUN_004054b0(unsigned int param1);

// Set while the shadow geometry of the current view has been streamed.
// GLOBAL: CMR2 0x00536ad0
int g_unk0x00536ad0;
// Screen rectangle of the view being drawn, filled by FUN_0040f050.
// GLOBAL: CMR2 0x00536ad4
int *g_unk0x00536ad4;
// 1 / 1024: kilobytes to megabytes.
// GLOBAL: CMR2 0x005112f8
float g_unk0x005112f8 = 9.765625e-04f;
// 3 / (8 * 1024 * 1024): frame buffer bytes to megabytes.
// GLOBAL: CMR2 0x005112f4
float g_unk0x005112f4 = 3.5762786865234375e-07f;

// GLOBAL: CMR2 0x00516ce8
char g_strDrawDistance0x00516ce8[87] =
    "Draw Distance, Actual(scaled) : %d  Requested : %d  Maximum : %d  FrontEnd Setting: %d";
// GLOBAL: CMR2 0x00516d40
char g_strSoftwareTl0x00516d40[19] = "Using Software T&L";
// GLOBAL: CMR2 0x00516d54
char g_strHardwareTl0x00516d54[19] = "Using Hardware T&L";
// GLOBAL: CMR2 0x00516d68
char g_strVbMemUsed0x00516d68[30] = "VB mem used = %d K (%0.1f MB)";
// GLOBAL: CMR2 0x00516d88
char g_strVramUsed0x00516d88[66] =
    "VRAM used: Textures = %d K (%0.1f MB)   Buffers = %d K (%0.1f MB)";
// GLOBAL: CMR2 0x00516dcc
char g_strTotalPolygons0x00516dcc[22] = " Total Polygons: %d  ";
// GLOBAL: CMR2 0x00516de4
char g_strOtherPolygons0x00516de4[22] = " Other Polygons: %d  ";
// GLOBAL: CMR2 0x00516dfc
char g_strScenePolygons0x00516dfc[22] = " Scene Polygons: %d  ";

// Per-view frame update of the rally data screens: prepares the cars of the
// view, streams their shadow geometry, refreshes the on-screen elements and,
// when the graphics debug flag (bit 2) is set, prints the FPS / polygon /
// VRAM overlay.
// match 78%: the logic is all there; the difference is MSVC's register
// allocation (the original keeps the raw view in EBX and the masked view in
// EBP, here it is the other way round) and the frame layout (the original
// reuses the incoming parameter slot for the overlay colour).
// FUNCTION: CMR2 0x0040f0c0
void FUN_0040f0c0(int param1, int param2, int param3)
{
    int orderCount;
    short *pOrder;
    short *pOrderEnd;
    int n;
    int i;
    int pixels;
    int resX;
    int resY;
    int resZ;
    int target;
    unsigned int vramKB;
    float vramMB;
    float buffersMB;
    Car *pCar;
    SceneNode *pNode;

    int view = param2;

    if ((BYTE)view == 0) {
        g_unk0x00536ad0 = 0;
        if (FUN_0041f3a0() != 0)
            return;
        if (CGame::FUN_004a9b20() != 0) {
            FUN_00411110();
            CGame::FUN_004a9b10(0);
        }
    }
    pOrder = Car_GetOrder();
    orderCount = Car_GetOrderCount();
    RallyData_FUN_0040eec0();
    g_unk0x00536ad4 = FUN_0040f050((view & 0xff));
    FUN_00471bf0((view & 0xff));
    FUN_00471a60((view & 0xff));
    if (CGraphics::FUN_004b74b0() != 0)
        Car_ApplyViewTransforms((view & 0xff));
    Car_UpdateViewNodes((view & 0xff));
    FUN_00461c30((view & 0xff));
    FUN_0046f330((view & 0xff));
    if ((BYTE)view == 0 && (BYTE)RallyDataState() == 1 &&
        CGameInfo::FUN_00404f20() == 0 &&
        (FUN_00456be0(0)[0x20] == 'A' || FUN_00456be0(0)[0x20] == 'C')) {
        FUN_0046ea10(0);
        FUN_0046e780();
    }
    for (i = 0; i < orderCount; i++)
        FUN_00462aa0(i, (view & 0xff));
    FUN_00428bf0((view & 0xff), (short *)g_unk0x00536ad4);
    FUN_0046e440();
    FUN_00464960(FUN_00422fb0((view & 0xff)));
    Game_PrepareScene((SceneNode *)RallyData_FUN_00411060(), g_viewNodes[(view & 0xff)],
                      (int)g_unk0x00536ad4, (view & 0xff));
    if (g_unk0x00536ad0 == 0) {
        g_unk0x00536ad0 = 1;
        Scene_BeginShadowBatch();
        n = orderCount;
        if (orderCount - 1 >= 0) {
            pOrderEnd = &pOrder[orderCount - 1];
            do {
                pCar = Car_Get(*pOrderEnd);
                pNode = pCar->pNode0x724;
                if (pNode != NULL && pNode->field_0x17c > 0)
                    FUN_004b5f90(pCar->pNode0x724, *(int *)pCar->field_0x758,
                                 (short *)pCar->field_0xb00);
                else
                    FUN_004b5f90(pCar->pNode0x71c, *(int *)pCar->field_0x758,
                                 (short *)pCar->field_0xb00);
                FUN_004b5ee0(pCar->pNode0x720, pCar->field_0xa70,
                             FUN_0046bd40(pCar->field_0xb1a));
                FUN_004b5ee0(pCar->pNode0x71c, pCar->field_0xa70,
                             FUN_0046bd40(pCar->field_0xb1a));
                if (pCar->pNode0x724 != NULL)
                    FUN_004b5ee0(pCar->pNode0x724, pCar->field_0xa70,
                                 FUN_0046bd40(pCar->field_0xb1a));
                pOrderEnd--;
            } while (--n != 0);
        }
        Scene_EndShadowBatch();
    }
    FUN_0046bb40();
    if (CGameInfo::FUN_00405e00() != 0) {
        for (i = 0; i < 7; i++) {
            if ((BYTE)FUN_00409cb0(i) == 0 && (BYTE)FUN_0040b1e0(i) != 0)
                FUN_0046b8f0(Car_Get(FUN_0040b010(i)));
        }
    }
    for (i = orderCount - 1; i >= 0; i--) {
        pCar = Car_Get(pOrder[i]);
        if (!CGameInfo::FUN_00406410(0x10) && i != 0) {
            FUN_0046b8f0(pCar);
            continue;
        }
        if ((g_pGraphics->field913_0x3bc & 0x80) != 0) {
            pNode = pCar->pNode0x724;
            if (pNode != NULL && pNode->field_0x17c > 0) {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 0);
                SceneNode_SetViewMaskTree(pCar->pNode0x724, 0);
                FUN_0049e1f0(pCar->pNode0x724, (view & 0xff));
            } else {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 0);
                SceneNode_SetViewMaskTree(pCar->pNode0x720, 0);
                FUN_0049e1f0(pCar->pNode0x720, (view & 0xff));
            }
        }
        pNode = pCar->pNode0x724;
        if (pNode != NULL && pNode->field_0x17c > 0) {
            target = (int)SceneNode_FindByType(pCar->pNode0x724, 0x14);
            if (CGame::FUN_0049c430() != 0) {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 0);
                SceneNode_SetViewMaskTree(pCar->pNode0x724, 0);
                if (target != 0)
                    SceneNode_SetViewMaskTree((SceneNode *)target, 1);
            } else {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 1);
                SceneNode_SetViewMaskTree(pCar->pNode0x724, 1);
                if (target != 0)
                    SceneNode_SetViewMaskTree((SceneNode *)target, 0);
            }
        } else {
            target = (int)SceneNode_FindByType(pCar->pNode0x720, 0x14);
            if (CGame::FUN_0049c430() != 0) {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 0);
                SceneNode_SetViewMaskTree(pCar->pNode0x720, 0);
                if (target != 0)
                    SceneNode_SetViewMaskTree((SceneNode *)target, 1);
            } else {
                SceneNode_SetViewMaskTree(pCar->pNode0x71c, 1);
                SceneNode_SetViewMaskTree(pCar->pNode0x720, 1);
                if (target != 0)
                    SceneNode_SetViewMaskTree((SceneNode *)target, 0);
            }
        }
        FUN_0046b8f0(pCar);
        FUN_0046b670((BYTE *)pCar);
    }
    FUN_004b2970(!CGameInfo::FUN_00406410(0xf));
    FUN_00422d40(param2);
    if ((g_pGraphics->field913_0x3bc & 4) != 0) {
        BYTE colour[4];

        colour[0] = 0xff;
        colour[1] = 0;
        colour[2] = 0;
        colour[3] = 0xff;
        sprintf(CFrontend::m_stringDest, g_strFpsFormat0x00516e14, FUN_004b23a0());
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0, (int *)colour, 9);
        sprintf(CFrontend::m_stringDest, g_strScenePolygons0x00516dfc, CGame::FUN_0049c400());
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0xf, (int *)colour, 9);
        sprintf(CFrontend::m_stringDest, g_strOtherPolygons0x00516de4, CGame::FUN_0049c410());
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x1e, (int *)colour, 9);
        sprintf(CFrontend::m_stringDest, g_strTotalPolygons0x00516dcc,
                CGame::FUN_0049c410() + CGame::FUN_0049c400());
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x2d, (int *)colour, 9);
        vramKB = CGraphics::FUN_004a5fe0();
        vramMB = (float)CGraphics::FUN_004a5fe0() * g_unk0x005112f8;
        resX = g_pGraphics->resX;
        resY = g_pGraphics->resY;
        resZ = g_pGraphics->depth;
        pixels = resX * resY * resZ;
        buffersMB = (float)resX * resY * resZ * g_unk0x005112f4;
        sprintf(CFrontend::m_stringDest, g_strVramUsed0x00516d88, vramKB, (double)vramMB,
                pixels / 8 / 1024 * 3, (double)buffersMB);
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x3c, (int *)colour, 9);
        sprintf(CFrontend::m_stringDest, g_strVbMemUsed0x00516d68, CGraphics::FUN_004b1970(),
                (double)((float)CGraphics::FUN_004b1970() * g_unk0x005112f8));
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x4b, (int *)colour, 9);
        if (CGraphics::FUN_004a8d60() == 2)
            sprintf(CFrontend::m_stringDest, g_strHardwareTl0x00516d54);
        else
            sprintf(CFrontend::m_stringDest, g_strSoftwareTl0x00516d40);
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x5a, (int *)colour, 9);
        sprintf(CFrontend::m_stringDest, g_strDrawDistance0x00516ce8,
                (FUN_00423f30() > *(int *)&g_pGraphics->field925_0x3c8
                     ? *(int *)&g_pGraphics->field925_0x3c8
                     : FUN_00423f30()) >> 16,
                *(int *)&g_pGraphics->field921_0x3c4 >> 16,
                *(int *)&g_pGraphics->field925_0x3c8 >> 16,
                (int)g_pGraphics->field917_0x3c0 + 1);
        Font_DrawText(1, CFrontend::m_stringDest, 0, 0x69, (int *)colour, 9);
    }
    FUN_00466570(pOrder, (short)orderCount, (int)g_unk0x00536ad4, (view & 0xff));
    if (CGameInfo::FUN_00405e00() != 0) {
        if (CGameInfo::FUN_00405d80() != 0xa) {
            if (**(char **)(FUN_0041b390() + 4) == '\n')
                FUN_00459790(0, 0);
            for (i = 0; i < 7; i++) {
                if ((BYTE)FUN_00409cb0(i) != 0)
                    FUN_00459790(FUN_0040b010(i), 0);
            }
        }
        FUN_004593a0();
    }
    if (RallyData_FUN_00407ea0() != 0 && CGameInfo::FUN_00406310() != 0)
        FUN_00466100((view & 0xff));
    if (CGameInfo::FUN_00404f20() == 0) {
        FUN_004657d0((view & 0xff));
        StageObject_UpdateSkidTrails((view & 0xff));
    }
    FUN_00465780((view & 0xff));
    FUN_00417e60();
    if (CGameInfo::FUN_00404f20() != 0 && FUN_004054b0((view & 0xff)) == 0)
        return;
    if ((BYTE)FUN_00407270() != 0)
        return;
    if ((BYTE)RallyDataState() == 1 && param3 != 0) {
        Dash_Draw((view & 0xff), (int)g_unk0x00536ad4);
        FUN_004125f0((view & 0xff), (short *)g_unk0x00536ad4);
        FUN_004177d0((view & 0xff), (int)g_unk0x00536ad4);
    }
    if ((BYTE)RallyDataState() == 2 && param3 != 0) {
        Dash_Draw((view & 0xff), (int)g_unk0x00536ad4);
        FUN_004125f0((view & 0xff), (short *)g_unk0x00536ad4);
        FUN_004177d0((view & 0xff), (int)g_unk0x00536ad4);
    }
}

// Finds the route node nearest to a car (only nodes within 100.0 in both
// ground axes are considered) and stores it as the car's current and previous
// progress record.
// FUNCTION: CMR2 0x00420850
void FUN_00420850(Car *pCar)
{
    FixVector pos;
    FixVector d;
    unsigned int i;
    int best;
    unsigned int bestIndex;
    int dx;
    int dz;

    best = 0x7d000000;
    bestIndex = 0;
    if (g_unk0x00538a94 != 0) {
        for (i = 0; i < g_unk0x00538a84; i++) {
            RallyData_FUN_00421530(i, (int *)&pos);
            dx = pos.x - pCar->position.x;
            d.x = dx;
            dz = pos.z - pCar->position.z;
            d.y = 0;
            d.z = dz;
            if (dx < 0)
                dx = -dx;
            if (dz < 0)
                dz = -dz;
            if (dx < 0x640000 && dz < 0x640000) {
                int len = FixVecLength(&d);
                if (len < best) {
                    best = len;
                    bestIndex = i;
                }
            }
        }
        if (best != 0x7d000000) {
            g_raceRecords[pCar->field_0xb1a].field_0x0 = bestIndex;
            g_raceRecords[pCar->field_0xb1a].field_0x4 = bestIndex;
        }
    }
}

char *FUN_0041f8f0(void);
GenericFile *FUN_0041f500(void);

// Textures of the challenge screens (net-play blob of each player and the blob
// of the current challenge).
// GLOBAL: CMR2 0x00537070
Texture *g_unk0x00537070;
// GLOBAL: CMR2 0x00537074
Texture *g_unk0x00537074;
// GLOBAL: CMR2 0x00537078
Texture *g_unk0x00537078;
// GLOBAL: CMR2 0x0053707c
Texture *g_unk0x0053707c;
// GLOBAL: CMR2 0x00517d38
char g_strChallengeTrackName0x00517d38[22] = "%s\\chall\\cha%.2d.tga";
// GLOBAL: CMR2 0x00517d50
char g_strChalblobTga0x00517d50[33] = "\\NEWIMAGE\\osd\\chall\\chalblob.tga";
// GLOBAL: CMR2 0x00517d74
char g_strNetblobrTga0x00517d74[33] = "\\NEWIMAGE\\osd\\chall\\netblobr.tga";
extern char g_strPathConcat[];
// GLOBAL: CMR2 0x00517da0
char g_strNetblobTga0x00517da0[32] = "\\NEWIMAGE\\osd\\chall\\netblob.tga";

// Loads the textures of the challenge screens: the net-play blobs for the two
// network challenge screens and the challenge blob of the current track (the
// track one is built from the install dir or, for the DD/network tracks, from
// the name of the loaded file with the extension changed to .DDS).
// FUNCTION: CMR2 0x00411280
void FUN_00411280(void)
{
    bool loaded;
    int track;

    if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9) {
        sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(),
                g_strNetblobTga0x00517da0);
        g_unk0x00537070 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(),
                                                    CFrontend::m_stringDest, &loaded, 0, 0, 0);
        sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(),
                g_strNetblobrTga0x00517d74);
        g_unk0x00537074 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(),
                                                    CFrontend::m_stringDest, &loaded, 0, 0, 0);
    }
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6 &&
        CGameInfo::FUN_00405d80() != 7 && CGameInfo::FUN_00405d80() != 4 &&
        CGameInfo::FUN_00405d80() != 0xb && CGameInfo::FUN_00405d80() != 0xc &&
        (BYTE)RallyData_GetFlag25() == 0)
        return;
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(),
            g_strChalblobTga0x00517d50);
    g_unk0x00537078 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(),
                                                CFrontend::m_stringDest, &loaded, 0, 0, 0);
    if (CGameInfo::FUN_00405d80() == 4 || (BYTE)RallyData_GetFlag25() != 0) {
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, FUN_0041f8f0());
        track = strlen(CFrontend::m_stringDest);
        CFrontend::m_stringDest[track - 2] = '.';
        CFrontend::m_stringDest[track - 1] = 'D';
        CFrontend::m_stringDest[track] = 'D';
        CFrontend::m_stringDest[track + 1] = 'S';
        CFrontend::m_stringDest[track + 2] = '\0';
    } else {
        sprintf(CFrontend::m_stringDest, g_strChallengeTrackName0x00517d38,
                CInstallInfo::GetTracksDir(),
                (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff) + 1);
    }
    g_unk0x0053707c = CTexture::FindLoadTexture(FUN_0041f500(), CFrontend::m_stringDest, &loaded, 0,
                                                0, 0);
}
