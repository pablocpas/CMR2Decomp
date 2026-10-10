#ifndef _STAGE_H
#define _STAGE_H

#include "GenericFileLoader.h"

struct StageArchiveTables {
    GenericFile archives[32];
    int driverCount;
    int secondaryCount;
};
typedef char StageArchiveSizeCheck[sizeof(StageArchiveTables) == 0x188 ? 1 : -1];
extern StageArchiveTables g_stageArchiveTables;
#define g_carModelArchives (g_stageArchiveTables.archives)

int Stage_GetSplitPositionFixed(int index);
int GetStageSplitCount(void);

#endif
