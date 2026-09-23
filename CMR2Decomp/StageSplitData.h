#ifndef _STAGE_SPLIT_DATA_H
#define _STAGE_SPLIT_DATA_H

// Split progress of one car through the current stage (0x48 bytes).
struct StageSplitData {
	int position;          // race position at the last split (0xf when unknown)
	int split;             // index of the last split passed
	int lastSplitTime;
	int targetTime;        // time to beat at the next split
	int times[14];         // times[0] is the start, times[n] the time at split n
};

extern StageSplitData g_stageSplitData[1];

#endif
