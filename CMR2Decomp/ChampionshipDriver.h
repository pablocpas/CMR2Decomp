#ifndef CMR2_CHAMPIONSHIP_DRIVER_H
#define CMR2_CHAMPIONSHIP_DRIVER_H

#include <stddef.h>
#include <windows.h>

#include "SavedGame.h"
#include "TyreWear.h"
#include "ChaseCamera.h"

// One championship driver entry (CMR2 0x0052f3e0, eight of them), used
// instead of the driver's profile in game mode 4. Kept out of SavedGame.h:
// declaring it there changes an addition in OptionMenu_DrawResultsRallyInfo.
struct ChampionshipDriver {
    char name[4];
    int car;                             // 0x04
    unsigned int options;                // 0x08, as DriverProfile::options
    BYTE flags;                          // 0x0c, bit 0 automatic gearbox, bit 1 edited
    BYTE hudFlags;                       // 0x0d, as DriverProfile::hudFlags
    BYTE reserved[2];
    TyreWear wear;                       // 0x10, as DriverProfile::wear
    ChaseCameraSettings chaseCamera;     // 0xb0, as DriverProfile::chaseCamera
};

typedef char ChampionshipDriverLayoutCheck1[(sizeof(ChampionshipDriver) == 0xc4) ? 1 : -1];
typedef char ChampionshipDriverLayoutCheck2[(offsetof(ChampionshipDriver, chaseCamera) == 0xb0) ? 1 : -1];
typedef char ChampionshipDriverLayoutCheck3[(offsetof(DriverProfile, bestResults) == 0x84) ? 1 : -1];
typedef char ChampionshipDriverLayoutCheck4[(offsetof(DriverProfile, wear) == 0x58c) ? 1 : -1];
typedef char ChampionshipDriverLayoutCheck5[(offsetof(DriverProfile, chaseCamera) == 0x634) ? 1 : -1];

#endif
