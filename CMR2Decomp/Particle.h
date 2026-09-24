#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <windows.h>
#include "FixedPoint.h"

// Definition of one particle kind (0x70 bytes).
struct ParticleType {
    int lifetime;
    FixVector spread;
    int gravity;            // 0x10
    int drag;               // 0x14 air drag toward g_particleWind
    int bounce;             // 0x18 vertical restitution on the floor
    int friction;           // 0x1c horizontal loss on the floor
    int size;
    int sizeVariation;      // 0x24 also the size the particle grows/shrinks to
    int sizeStep;           // 0x28
    short field0x2c;        // angle step
    BYTE type;              // 0x2e also the initial alpha
    BYTE alphaEnd;          // 0x2f
    BYTE alphaStep;         // 0x30
    BYTE colour[3];
    BYTE pad0x34;
    BYTE flags;             // 0x35 1 template, 2 kill below floor, 4 kill at alpha end, 8 size ramp,
                            //      0x10 size ramp set by range, 0x20 spin, 0x40 alpha ramp, 0x80 bounce
    BYTE directionFlags;
    BYTE pad0x37;
    int field0x38;
    int field0x3c;          // 0x3c billboard top/left/bottom/right (0x3c..0x48)
    int field0x40;
    int field0x44;
    int field0x48;
    int field0x4c;          // 0x4c animation frames (texture per frame)
    int field0x50;          // 0x50 frame count
    int field0x54;          // 0x54 delay before the animation starts
    int field0x58;          // 0x58 time per frame
    int field0x5c;          // 0x5c custom draw callback
    void (*update)(void *, ParticleType *, int);        // 0x60 replaces the default motion
    void (*postUpdate)(void *, ParticleType *, int);    // 0x64
    void (*callback)(void *, ParticleType *, int);
    int field0x6c;
};

// Runtime particle record (0x68 bytes).
struct Particle {
    ParticleType *pType;
    FixVector position;
    FixVector vector0x10;
    FixVector vector0x1c;
    FixVector vector0x28;
    FixVector sourceVector;
    int field0x40;
    int age;
    int field0x48;
    int size;
    short field0x50;
    BYTE type0x52;
    BYTE type0x53;
    BYTE field0x54;
    BYTE field0x55;
    BYTE type0x56;
    BYTE active;
    BYTE field0x58;
    BYTE colour[3];
    BYTE pad0x5c[4];
    int field0x60;
    short field0x64;         // effect data: car << 8 | variant
    short field0x66;
};

void Particle_Spawn(int typeIndex, FixVector *pSource, FixVector *pPosition, int field0x48, int field0x40,
                    BYTE *pColour, BYTE field0x54, int callbackParam, BYTE field0x55);

#endif
