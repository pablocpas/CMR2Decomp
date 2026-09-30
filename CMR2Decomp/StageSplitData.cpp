#include "StageSplitData.h"

// Colour views overlap the reset prefix, followed by two 0x48-byte records,
// their per-car flags and the reference split times. All share one allocation.
// GLOBAL: CMR2 0x00536d14
StageSplitRuntime g_stageSplitRuntime;
