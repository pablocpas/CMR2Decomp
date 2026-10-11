#ifndef _STAGE_TIMING_SPREAD_H
#define _STAGE_TIMING_SPREAD_H

// Runtime interpolation workspace, owned by InitRaceDriverRecords. The
// four ten-word bands share a word span for the original biased cursors.
struct StageTimingSpread {
    union {
        int sampleWords[40];
        struct {
            int progress[10];     // 0x00 accumulated active route fraction
            int startWeight[10];  // 0x28 fraction before the start position
            int middleWeight[10]; // 0x50 remaining active fraction
            int endWeight[10];    // 0x78 fraction after the end position
        };
    };
    int startPositionFixed; // 0xa0
    int endPositionFixed;   // 0xa4
    int startBlendFixed;    // 0xa8
    int endBlendFixed;      // 0xac
};

#endif
