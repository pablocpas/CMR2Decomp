#include "StageSplitData.h"

// Only one car keeps split data; the per-car globals that follow start at 0x536e88.
// GLOBAL: CMR2 0x00536dfc
StageSplitData g_stageSplitData[1];
