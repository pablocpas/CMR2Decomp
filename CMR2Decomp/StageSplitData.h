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

typedef unsigned char BYTE;
// The reset routine also clears the DWORD immediately before the records.
// Keep that observed field in the same allocation as the two records and flags.
struct StageSplitState {
    int resetPrefix;
    BYTE recordBlock[0x94];
    int referenceTimes[13];
};
// The final colour DWORD at 0x536df8 is also the reset prefix. These are
// overlapping views in the original, followed by the two records and references.
union StageSplitRuntime {
    int colourWords[0x3a];
    struct {
        int colourPrefix[0x39];
        StageSplitState state;
    } split;
};
extern StageSplitRuntime g_stageSplitRuntime;
#define g_stageSplitState (g_stageSplitRuntime.split.state)
#define g_unk0x00536d14 (g_stageSplitRuntime.colourWords)
#define g_unk0x00536e90 (g_stageSplitState.referenceTimes)
#define g_stageSplitBlock (g_stageSplitState.recordBlock)
#define g_stageSplitData ((StageSplitData *)g_stageSplitBlock)

#endif
