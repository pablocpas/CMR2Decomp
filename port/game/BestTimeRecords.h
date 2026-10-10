#ifndef CMR2_BEST_TIME_RECORDS_H
#define CMR2_BEST_TIME_RECORDS_H

// ON-DISK: fixed-width words in the selected category's save record.
// No relocated pointers. The rally time is the second word of this tail.
struct BestTimeRallyTail {
    unsigned int field_0x0;
    unsigned int centiseconds; // +0x4 (category save view +0x38)
};

// The original reuses each scratch slot for a numeric index/count in one
// pass and an address in another. Never convert the active pointer to int.
// This is runtime scratch storage, not a serialized record.
union BestTimeCursor {
    int index;
    unsigned char *bytes;
    unsigned int *word;
    BestTimeRallyTail *rally;
};

#endif
