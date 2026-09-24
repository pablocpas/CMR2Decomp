#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <windows.h>
#include "FixedPoint.h"

// Definition of one particle kind (0x70 bytes).
struct ParticleType {
    int lifetime;
    FixVector spread;
    BYTE pad0x10[0x10];
    int size;
    int sizeVariation;
    BYTE pad0x28[4];
    short field0x2c;
    BYTE type;
    BYTE pad0x2f[2];
    BYTE colour[3];
    BYTE pad0x34;
    BYTE flags;
    BYTE directionFlags;
    BYTE pad0x37;
    int field0x38;
    BYTE pad0x3c[0x2c];
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
