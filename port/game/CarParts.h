#ifndef _CARPARTS_H
#define _CARPARTS_H

#include "FixedPoint.h"
#include "Mesh.h"

// Packed 16.16 copy of one part vertex (built by CarDamage_BuildPartVertexBuffer).
struct CarPartVertex {
    FixVector pos;               // 0x00
    FixVector normal;            // 0x0c
    signed char normal8[3];      // 0x18 normalised normal, 1.7 bits
    signed char rawNormal[3];    // 0x1b from the vertex colour bytes
    BYTE pad[2];
};

// Float vertex of a part mesh (Mesh::pVertexData, 0x30 bytes).
struct CarPartFloatVertex {
    float pos[3];    // 0x00
    float normal[3]; // 0x0c
    DWORD colour;    // 0x18
    BYTE pad[0x14];
};

// Object that owns a part mesh.
struct CarPartNode {
    BYTE pad0[0xc];
    Mesh *pMesh;     // 0x0c
    BYTE pad10[0x20];
    int key;         // 0x30 low byte = material key
};

// Breakable parts of one car body (0x4d0 bytes per car, table at
// g_carPartSets): render meshes, owners, packed vertices and boxes.
struct CarPartSet {
    Mesh *meshes[15];                 // 0x000
    CarPartNode *nodes[15];           // 0x03c
    CarPartVertex *vertices[15];      // 0x078
    FixVector centres[15];            // 0x0b4
    FixVector halfExtents[15];        // 0x168
    int damageGrid[3][3];             // 0x21c dent depth per body cell
    int damageValues[34];             // 0x240 derived 16.16 damage; see StageObject_RebuildDamagePartValues
    int damageScales[34];             // 0x2c8 multiplicative damage factors
    int damageBiases[34];             // 0x350 additive damage factors
    int steeringWobble;               // 0x3d8 amplitude of damage-induced steering noise
    int suspensionDamageOffset[4];    // 0x3dc per-wheel suspension offset
    int wheelDamageDrag[4];           // 0x3ec per-wheel drag torque
    int frontBrakeScale;              // 0x3fc
    int rearBrakeScale;               // 0x400
    int engineTorqueScale;            // 0x404 damage multiplier applied to throttle torque
    int bodyDamageDrag;               // 0x408 additional aerodynamic drag
    BYTE field_0x40c[4];
    int maxX;                         // 0x410
    int minX;                         // 0x414
    int maxZ;                         // 0x418
    int minZ;                         // 0x41c
    int vertexCount[15];              // 0x420
    int count;                        // 0x45c
    BYTE breakPartIndex[8];           // 0x460 model-part id for each of the eight break slots
    BYTE gearShiftDamage;             // 0x468 quantised damageValues[14]
    BYTE glassCooldown;               // 0x469 limits glass shard bursts, decays once per step
    BYTE field_0x46a[2];
    int glassDebrisEmitted;           // 0x46c prevents cooldown decay during a glass/debris burst
    int partDamaged[8];               // 0x470 cracked/damaged appearance has been applied
    int partBroken[8];                // 0x490 broken appearance has been applied
    int partHidden[4];               // 0x4b0
    int lineGrounded[3];               // 0x4c0
    int field_0x4cc;
};

// A flexible line drawn from a car model (0x20-byte serialized descriptor).
// The original reserves three dynamic states per car. The model's count
// selects the active descriptors; a grounded line is drawn as one segment.
struct CarFlexibleLineDescriptor {
    FixVector position;             // 0x00 local attachment point
    FixVector axis;                 // 0x0c local rest axis
    int length;                     // 0x18 16.16 line length
    BYTE colour[4];                 // 0x1c RGBA
};

struct CarFlexibleLineState {
    FixVector position;             // 0x00 deflection, or world position if grounded
    FixVector previousPosition;     // 0x0c
    FixVector drawPosition;         // 0x18 interpolated deflection
    FixVector groundEnd;            // 0x24 end of the grounded segment
    int velocityX;                  // 0x30 spring velocity across the rest axis
    int velocityZ;                  // 0x34
    int grounded;                   // 0x38 ground segment has been resolved
};

extern CarFlexibleLineState **g_carLineStates;
extern CarFlexibleLineDescriptor *g_carLineDescriptors[8];
extern int *g_carLineCounts[8];

// One damage link of a car's damage record (13 bytes, linked by index).
struct CarDamageLink {
    BYTE data[0xc];
    signed char next;                // 0x0c  next link, -1 = end
};

// Packed list of twenty 13-byte impact records. The count is a byte;
// firstLink and each link's next byte select indices rather than pointers.
struct CarDamageLinkPool {
    CarDamageLink links[20];
    BYTE count;                     // 0x104
    BYTE firstLink;                 // 0x105
};

// One 0x40-byte damage snapshot, copied to/from driver-group records.
struct CarDamageSnapshot {
    BYTE intensity[34];             // 0x00 quantised per-part damage
    BYTE padding[2];                // 0x22 alignment before the flag arrays
    int partHidden[4];              // 0x24 moving parts whose node is hidden
    int lineGrounded[3];            // 0x34 resolved flexible-line ground segments
};

// Damage state of one car: active state and preserved stage snapshots.
// Two packed 0x106-byte link pools followed by two 0x40-byte snapshots.
struct CarDamageRecord {
    CarDamageLinkPool impacts;       // 0x000
    CarDamageLinkPool stageImpacts;  // 0x106
    CarDamageSnapshot damage;        // 0x20c
    CarDamageSnapshot stageDamage;   // 0x24c
    int stageSnapshotCaptured;      // 0x28c
};

struct SceneNode;
struct Car;

// Moving part of a car (bonnet, boot, doors...): one 0x1a0-byte record per car
// in each of the four slot tables g_carPartStateTables.parts[slot]. CarPart_InitWheelHubSlot installs
// the slot's integrator at +0x110; g_partState points at the record being
// simulated, g_partCar / g_partSet at its car and the car's part set.
struct PartState {
    SceneNode *pNode;           // 0x000 node the part is drawn with
    FixMatrix *pWorld;          // 0x004 world matrix
    FixMatrix *pForceFrame;     // 0x008 matrix that brings forces into part space
    FixMatrix matrix;           // 0x00c local transform of this step
    FixMatrix prevMatrix;       // 0x04c and of the previous step
    FixMatrix drawMatrix;       // 0x08c interpolated between the two for drawing
    FixMatrix worldMatrix;      // 0x0cc matrix * *pParent
    FixMatrix *pParent;         // 0x10c
    void (*update)(void);       // 0x110 per-slot integrator
    FixVector swingAxis;        // 0x114
    FixVector position;         // 0x120 body offset
    FixVector previousPosition; // 0x12c
    FixVector velocity;         // 0x138
    FixVector correction;       // 0x144 accumulated translation
    BYTE flags;                 // 0x150 1 active, 2/4/8 state, high nibble = slot
    BYTE field_0x151[3];
    int field_0x154;            // 0x154
    short angle;                // 0x158 swing / rest angle (12-bit)
    short field_0x15a;
    int boundsRadius;            // 0x15c
    int field_0x160;            // 0x160
    FixVector angularVelocity;  // 0x164 accumulated swing offset
    FixVector stiffness;        // 0x170 ground response gains
    FixBasis basis;             // 0x17c right / up / forward
};
typedef char PartStateSize[sizeof(PartState) == 0x1a0 ? 1 : -1];

extern CarDamageRecord *g_carDamageRecords;
extern CarPartSet *g_carPartSets;
extern PartState *g_partState;
extern Car *g_partCar;
extern CarPartSet *g_partSet;

#endif
