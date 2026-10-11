#ifndef _RACE_CALL_RECORD_H
#define _RACE_CALL_RECORD_H
// Runtime queue slot; packed .cod words are numeric IDs, not addresses.
struct RaceCallRecord {
    int primaryCallId;          // 0x00 decoded for the main icon / audio call
    int secondaryCallId;        // 0x04 zero or the second icon / audio call
    unsigned int flags;        // 0x08 low byte countdown, 0x100 queued, 0x200 assigned
};
#endif
