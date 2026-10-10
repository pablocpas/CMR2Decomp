#ifndef _STAGE_H
#define _STAGE_H
#if defined(_MSC_VER) && _MSC_VER <= 1200
#ifndef CMR2_LAYOUT_CHECK
#define CMR2_LAYOUT_CHECK(name, condition) typedef char name[condition ? 1 : -1]
#endif
#else
#include "LayoutChecks.h"
#endif
#include "GenericFileLoader.h"

struct StageArchiveTables {
    GenericFile archives[32];
    int driverCount;
    int secondaryCount;
};
CMR2_LAYOUT_CHECK(StageArchiveSizeCheck, sizeof(StageArchiveTables) == 0x188);
extern StageArchiveTables g_stageArchiveTables;
#define g_carModelArchives (g_stageArchiveTables.archives)

int Stage_GetSplitPositionFixed(int index);
int GetStageSplitCount(void);

#endif
