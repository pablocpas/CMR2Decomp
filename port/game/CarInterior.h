#ifndef _CAR_INTERIOR_H
#define _CAR_INTERIOR_H

#include "FixedPoint.h"

struct SceneNode;

// Observed prefix of CIN section 5. Wiper animation reads the two signed
// limits, driver pose reads its signed rest yaw, and attachment reads the
// driver offset. Unconsumed bytes remain unknown. The view has sizeof 0x30;
// its last observed read ends at 0x2e, not an established full file size.
struct CarInteriorProfile {
    FixAngles firstWiperRotation;       // 0x00
    short firstWiperMaxAngle;           // 0x08
    short field_0xa;
    FixVector driverPoseOffset;         // 0x0c
    short driverRestYaw;                // 0x18
    short field_0x1a;
    FixAngles steeringBaseRotation;     // 0x1c
    FixAngles secondWiperRotation;      // 0x24
    short secondWiperMaxAngle;          // 0x2c
};

// Seven separately cached views into one CIN interior profile.
struct CarInteriorProfilePointers {
    FixAngles *firstWiperRotation;
    short *firstWiperMaxAngle;
    FixVector *driverPoseOffset;
    short *driverRestYaw;
    FixAngles *steeringBaseRotation;
    FixAngles *secondWiperRotation;
    short *secondWiperMaxAngle;
};

struct CarInteriorNodes {
    SceneNode *steeringRestNode;        // 0x00 type 0x1a
    SceneNode *steeringBlendNode;       // 0x04 type 0x1c
    SceneNode *steeringReferenceNode;   // 0x08 created, copies steering blend transform
    SceneNode *driverPoseNode;          // 0x0c created under body
    SceneNode *steeringWheelNode;       // 0x10 type 0x1b
    SceneNode *wiperNodes[2];           // 0x14 types 0x16/0x17
};

// Two 12-byte states in the shared stage block. Old int/short views overlap:
// mode and sweep direction are ints; current angle and step are shorts.
struct CarWiperState {
    int mode;                          // 0 off, 1 finish, 2 slow, 3 fast
    int sweepingForward;
    short angle;
    short step;
};

// Driver pose state: two rows, with a phase angle, eased rotation and offset.
// Its mount offset is reset and used to build the cockpit mount matrix.
// This is runtime state, not a CIN file record.
struct CarDriverPoseState {
    short phaseAngle;
    FixAngles rotation;
    short field_0xa;
    FixVector offset;
    FixVector mountOffset;
};

struct Texture;
struct CarInteriorDashTextures {
    Texture *revCounter;
    Texture *digit;
};

// Contiguous cockpit region of the stage block, 0x58d4d0..0x58d55f. Keeping
// this view inside the original BYTE storage preserves neighbouring aliases.
struct CarInteriorRuntimeTables {
    unsigned char modelClasses[8];
    CarWiperState wipers[2];
    CarInteriorProfilePointers profiles[2];
    CarInteriorNodes nodes[2];
};

#endif
