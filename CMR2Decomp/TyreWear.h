#ifndef CMR2_TYRE_WEAR_H
#define CMR2_TYRE_WEAR_H

// Tyre wear of the four wheels of a driver's car (DriverProfile::wear and
// ChampionshipDriver::wear). Tyre_AddWear adds to wear and damage and keeps
// their peaks; CarTyres_CommitPlayerWear saves the values at the start of a
// stage and restores them. The second wear value is set from the larger of
// its peak and peakWear when a stage is committed. Not in SavedGame.h: any new
// struct defined there shifts code in GameInfo.cpp.
struct TyreWear {
    int peakWear[4];       // 0x00
    int peakWear2[4];      // 0x10
    int peakDamage[4];     // 0x20
    int savedWear2[4];     // 0x30
    int savedWear[4];      // 0x40
    int savedDamage[4];    // 0x50
    int wear[4];           // 0x60
    int wear2[4];          // 0x70
    int damage[4];         // 0x80
    int state[4];          // 0x90, 0 good, 1 worn, 2 badly worn, 3 damaged
};

typedef char TyreWearLayoutCheck[(sizeof(TyreWear) == 0xa0) ? 1 : -1];

#endif
