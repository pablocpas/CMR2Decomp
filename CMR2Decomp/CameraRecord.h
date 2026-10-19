#ifndef CMR2_CAMERA_RECORD_H
#define CMR2_CAMERA_RECORD_H

#include "FixedPoint.h"

// Camera record of a view (100 bytes): two per player in g_viewRecords, and
// the same layout for each view's blended and previous state. Camera types
// (Camera_Start): 1, 2, 10 car-mounted views placed from the car's CIN offsets
// (View_PositionFromCarProfile); 3 the interior view; 4, 5, 6 the chase views
// (ChaseCamera_Start); 7 a trackside camera spot; 8 a blend in
// progress (view states only); 9 the replay playback camera.
struct CameraRecord {
    BYTE index;             // 0x00 record index
    BYTE view;              // 0x01
    BYTE car;               // 0x02 car followed
    BYTE pad_0x3;
    int type;               // 0x04 camera type
    FixMatrix matrix;       // 0x08
    int carLift;            // 0x48 share of the clearance lift given to the car's nodes
    int nearPlane;          // 0x4c projection near plane
    int field_0x50;         // 0x50 (lookDistance - 1); copied out, never read
    int zoom;               // 0x54 projection zoom (0xa000 by default)
    int liftPitch;          // 0x58 downward look per unit of clearance lift
    int lookDistance;       // 0x5c distance of the point the lifted view keeps in sight
    int clearance;          // 0x60 height above the stage, eased
};

#endif
