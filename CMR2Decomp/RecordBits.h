#ifndef CMR2_RECORD_BITS_H
#define CMR2_RECORD_BITS_H

// Flags words of the GameInfo0xa4SubStruct12 best-result records. Kept out of
// GameInfo.h: one more type there changes MSVC6's register ties in
// GameInfo.cpp (0x4f4b90 drops from 94.9% to 76.7%).

// Arcade records (thirdLoop): a 3-bit level and a 6-bit score.
struct RecordScoreBits {
    unsigned car : 6;
    unsigned automatic : 1;
    unsigned level : 3;
    unsigned score : 6;
    unsigned rest : 16;
};
#define SCORE_BITS(pRecord) (*(RecordScoreBits *)&(pRecord)->flags)

// Records with a 7-bit score and a 4-bit level (firstLoop,
// FrontendRecords_InsertStageCategoryRecord).
struct RecordLevelBits {
    unsigned car : 6;
    unsigned automatic : 1;
    unsigned score : 7;
    unsigned level : 4;
    unsigned rest : 14;
};
#define LEVEL_BITS(pRecord) (*(RecordLevelBits *)&(pRecord)->flags)

#endif
