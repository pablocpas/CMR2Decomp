#ifndef CMR2_CHASE_CAMERA_H
#define CMR2_CHASE_CAMERA_H

#include "FixedPoint.h"

// A driver's chase camera settings (DriverProfile::chaseCamera and
// ChampionshipDriver::chaseCamera), edited in the in-race camera menu and
// applied with ChaseCamera_SetOffset/SetPitch/SetLag. Defaults: offset
// (0, 1.5, 6.5), pitch 0x2d, lag 14.0.
struct ChaseCameraSettings {
    FixVector offset;   // 0x00 x (unused, 0), height and distance behind the car
    short pitch;        // 0x0c 12-bit angle the view is tilted down by
    short pad_0xe;
    int lag;            // 0x10 follow time: the view turns by timeStep / lag per frame
};

#endif
