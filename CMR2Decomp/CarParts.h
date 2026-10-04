#ifndef _CARPARTS_H
#define _CARPARTS_H

#include "FixedPoint.h"
#include "Mesh.h"

// Packed 16.16 copy of one part vertex (built by FUN_0046afe0).
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
// g_unk0x00588b94): render meshes, owners, packed vertices and boxes.
struct CarPartSet {
    Mesh *meshes[15];                // 0x000
    CarPartNode *nodes[15];          // 0x03c
    CarPartVertex *vertices[15];     // 0x078
    FixVector centres[15];           // 0x0b4
    FixVector halfExtents[15];       // 0x168
    int damageGrid[3][3];            // 0x21c dent depth per body cell
    int field_0x240[0x22];           // 0x240 per-part values rebuilt from damageGrid (FUN_00468c10)
    int field_0x2c8[0x22];           // 0x2c8 scale applied to field_0x240
    int field_0x350[0x22];           // 0x350 bias added after the scale
    int field_0x3d8;                 // 0x3d8
    int field_0x3dc[4];              // 0x3dc
    int field_0x3ec[4];              // 0x3ec
    int field_0x3fc[3];              // 0x3fc
    int field_0x408;                 // 0x408
    BYTE field_0x40c[4];
    int maxX;                        // 0x410
    int minX;                        // 0x414
    int maxZ;                        // 0x418
    int minZ;                        // 0x41c
    int vertexCount[15];             // 0x420
    int count;                       // 0x45c
    BYTE field_0x460[8];
    BYTE field_0x468;                // 0x468
    BYTE field_0x469;                // 0x469
    BYTE field_0x46a[2];
    int field_0x46c;                 // 0x46c
    int field_0x470[8];              // 0x470
    int field_0x490[8];              // 0x490
    int field_0x4b0[4];              // 0x4b0
    int field_0x4c0[3];              // 0x4c0
    int field_0x4cc;
};

// One damage link of a car's damage record (13 bytes, linked by index).
struct CarDamageLink {
    BYTE data[0xc];
    signed char next;                // 0x0c  next link, -1 = end
};

// Damage record of one car (0x290 bytes per car, table at g_unk0x00588b98).
struct CarDamageRecord {
    CarDamageLink links[20];         // 0x000
    BYTE hasLinks;                   // 0x104
    BYTE firstLink;                  // 0x105
    BYTE field_0x106[0x106];
    BYTE intensity[0x22];            // 0x20c
    BYTE field_0x22e[2];
    int field_0x230[4];              // 0x230
    int field_0x240[3];              // 0x240
    BYTE field_0x24c[0x40];
    int field_0x28c;                 // 0x28c
};

struct SceneNode;
struct Car;

// Moving part of a car (bonnet, boot, doors...): one 0x1a0-byte record per car
// in each of the four slot tables g_unk0x00590d7c[slot]. FUN_00480e50 installs
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
    int field_0x15c;            // 0x15c
    int field_0x160;            // 0x160
    FixVector angularVelocity;  // 0x164 accumulated swing offset
    FixVector stiffness;        // 0x170 ground response gains
    FixBasis basis;             // 0x17c right / up / forward
};
typedef char PartStateSize[sizeof(PartState) == 0x1a0 ? 1 : -1];

extern PartState *g_partState;
extern Car *g_partCar;
extern CarPartSet *g_partSet;

#endif
