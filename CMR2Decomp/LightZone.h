#ifndef _LIGHTZONE_H
#define _LIGHTZONE_H

// Kept out of SceneNode.h: declaring these there shifts code in TUs that never
// use them (OptionMenu_DrawResultsRallyInfo).

struct StageObject;
struct MeshVertexF;
struct MeshTriangle;

// ON-DISK (stage lighting data), relocated in place by Scene_LoadLighting:
// the pointers below are 32-bit file offsets until then.

// Shadow mesh of a light zone (0x14 bytes).
struct LightZoneMesh {
    int triangleCount;              // 0x0
    int vertexCount;                // 0x4
    MeshVertexF *pVertices;         // 0x8
    MeshTriangle *pTriangles;       // 0xc  until loaded, +4 is the index of the item under it
    int *pLevels;                   // 0x10 light level of each vertex
};

// A scene object a light zone shades (0x30 bytes): the scene lights attenuate
// its intensity, which colours the next vertexCount vertices of the zone mesh.
struct LightZoneItem {
    int ownerX;                     // 0x0  position of the object it belongs to
    int ownerZ;                     // 0x4
    int x;                          // 0x8  centre in the light basis
    int z;                          // 0xc
    int halfSizeRight;              // 0x10
    int halfSizeForward;            // 0x14
    int field_0x18;                 // 16.16 scale - 1 of the projected shadow, x
    int field_0x1c;                 // 16.16 scale - 1 of the projected shadow, y
    int intensity;                  // 0x20 16.16
    int field_0x24;
    int vertexCount;                // 0x28
    StageObject *pOwner;            // 0x2c
};

// Light zone of a sector (0x14 bytes).
struct LightZone {
    short sector;                   // 0x0  sector at (x, z), -1 if none
    unsigned short itemCount;       // 0x2
    int x;                          // 0x4
    int z;                          // 0x8
    LightZoneItem *pItems;          // 0xc
    LightZoneMesh *pMesh;           // 0x10
};

#endif
