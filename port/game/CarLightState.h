#ifndef _CAR_LIGHT_STATE_H
#define _CAR_LIGHT_STATE_H

#include "FixedPoint.h"

struct Texture;
struct GlowLight;

// Two sets of five signed levels per car. Animation derives displayed levels
// from faded levels, preserves the applied copy until a repaint, and supplies
// the same displayed levels to the point-glow callers. The two masks select
// channels independently; their spatial meaning is not inferred here.
struct CarLightTextureState {
    Texture *textures[2];
    short levels[2][5];
    short appliedLevels[2][5];
    short fadedLevels[2][5];
    unsigned char channelMasks[2];
    unsigned char combineFirstTwoChannels;
};

// Shared 100-record pool used by spawn, ground motion, collision and glow
// interpolation. Interior views retain the same storage, not separate arrays.
struct CarHeadlightGlowRecord {
    FixVector velocity;
    FixVector position;
    FixVector normal;
    int fade;
    FixVector prevPosition;
    FixVector prevNormal;
    int prevFade;
    int height;
    int life;
    short triangle;
    short field_0x4e;
    GlowLight *pGlow;
    int active;
    unsigned char car;
    unsigned char field_0x59[3];
};

extern CarLightTextureState g_carLightTextureStates[8];
extern CarHeadlightGlowRecord g_carHeadlightGlowRecords[100];
extern int g_carHeadlightGlowCooldowns[8];

#endif
