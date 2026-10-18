#ifndef CMR2_PRECIPITATION_QUAD_H
#define CMR2_PRECIPITATION_QUAD_H

#include "Sprite.h"

// CMR2 0x00543da0: the particle scale of the weather (views / 400) and the
// rain streak quad that StageWeather_DrawViewPrecipitation fills for each
// drop; StageWeather_InitParticleAndLightState sets its colours and uv.
struct PrecipitationQuad {
    int particleScale;      // 0x00
    int reserved;           // 0x04
    Quad2DVertices quad;    // 0x08
};

typedef char PrecipitationQuadLayoutCheck[(sizeof(PrecipitationQuad) == 0x98) ? 1 : -1];

#endif
