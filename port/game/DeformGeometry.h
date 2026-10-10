#ifndef _DEFORM_GEOMETRY_H
#define _DEFORM_GEOMETRY_H

#include "FixedPoint.h"

struct Mesh;
// Packed source vertex shared by car damage and option preview (0x20 bytes).
struct DeformVertex {
    FixVector pos;               // 0x00
    FixVector normal;            // 0x0c
    signed char normal8[3];      // 0x18 normalised normal, 1.7 bits
    signed char rawNormal[3];    // 0x1b from the vertex colour bytes
    BYTE pad[2];
};

// Float vertex of a part mesh (Mesh::pVertexData, 0x30 bytes).
struct DeformFloatVertex {
    float pos[3];    // 0x00
    float normal[3]; // 0x0c
    DWORD colour;    // 0x18
    BYTE pad[0x14];
};

struct SceneNode;

// Common leading arrays of car-part and option-preview mesh records.
struct DeformMeshSources {
    Mesh *meshes[15];             // 0x00
    SceneNode *nodes[15];         // 0x3c
    DeformVertex *vertices[15];   // 0x78
};

#endif
