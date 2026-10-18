#ifndef CMR2_PROFILE_EVENT_RESULTS_H
#define CMR2_PROFILE_EVENT_RESULTS_H

#include <windows.h>

// Results of the event being played (DriverProfile::eventResults, returned by
// RallyData_GetCategoryOptionRecord). The Frontend_MergeBest* functions merge
// them into the matching CategoryBestResults tables. Not in SavedGame.h: any
// new struct defined there shifts code in GameInfo.cpp.
struct ProfileEventResults {
    unsigned int playerResult;     // 0x00, level 4 bits, 2 bits, score 8 bits
    int playerPoints;              // 0x04
    BYTE playerPlacings[3];        // 0x08, counts of the first three places
    BYTE reserved;
    unsigned int rallyResult[2];   // 0x0c, result word and time (rallyResults)
    unsigned int rallyStageTime;   // 0x14, rallyStageTimes
    unsigned int arcadeResult;     // 0x18, level 3 bits, 2 bits, score 6 bits
    int arcadeExtra;               // 0x1c
    unsigned int arcadeStageTime;  // 0x20, arcadeStageTimes
};

typedef char ProfileEventResultsLayoutCheck[(sizeof(ProfileEventResults) == 0x24) ? 1 : -1];

#endif
