#ifndef _STAGE_WEATHER_H
#define _STAGE_WEATHER_H

#include <windows.h>
#include "FixedPoint.h"
#include "StageWeatherParticle.h"

// Runtime weather of one camera, allocated by 0x45e5b0 (0x178 bytes).
// Particle counts, rates and fades use 16.16; the pool indices are signed
// shorts. The 5x5 terrain cache follows the camera's precipitation box.
struct ViewWeatherState {
    int kind;                          // 0x000 0 clear, 1 rain, 2 snow
    int targetKind;                    // 0x004
    FixVector centre;                  // 0x008 precipitation box centre
    FixVector previousCentre;          // 0x014 camera motion before offset
    FixVector relativeVelocity;        // 0x020 velocity minus camera motion
    FixVector velocity;                // 0x02c wind and vertical fall speed
    FixVector snowSwayAxis;             // 0x038 horizontal camera right axis
    FixVector splashDisplacement;      // 0x044 negative velocity * timestep
    int inverseFallDisplacement;       // 0x050 reciprocal of +0x48
    int intensity;                     // 0x054 route-dependent weather strength
    int particleCount;                 // 0x058 current count in 16.16
    int targetParticleCount;           // 0x05c
    int particleCountChangeRate;        // 0x060
    int precipitationFade;             // 0x064 shelter fade, 0..0x10000
    int sunVisibility;                 // 0x068 smoothed screen visibility
    unsigned int routePosition;        // 0x06c
    unsigned int lastRoutePosition;    // 0x070
    short particleCapacity;            // 0x074 share of the 400-particle pool
    short firstParticle;               // 0x076
    short groundTriangles[25];         // 0x078 cached terrain triangle indices
    BYTE field_0xaa[2];                // 0x0aa no observed reader/writer
    int groundHeights[25];             // 0x0ac
    int relativeGroundHeights[25];     // 0x110 centre.y - height - 0x61999
    int sheltered;                     // 0x174 special route interval
};

// Lighting blend and wind-driven sun yaw for one view (0x2c bytes).
// Route positions are unsigned; values and angles have signed 16.16 views.
struct ViewLightingRamp {
    unsigned int routePosition;        // 0x00
    unsigned int lastRoutePosition;    // 0x04
    int routeBlend;                    // 0x08
    int routeBlendOffset;              // 0x0c fractional route progress
    int blend;                         // 0x10 routeBlend + routeBlendOffset
    int sunYaw;                        // 0x14 wraps at 360 degrees
    int previousBlend;                 // 0x18
    int drawBlend;                     // 0x1c interpolated weather blend
    int previousSunYaw;                // 0x20
    int drawSunYaw;                    // 0x24
    int lightingDirty;                 // 0x28
};

// Third route ramp, used by Car_UpdateSurfaceParams (0x0c bytes per car).
struct CarSurfaceRamp {
    unsigned int routePosition;
    unsigned int lastRoutePosition;
    int blend;
};

// Wind control was a FixVector, but its components are independent scalars.
struct WeatherWindTransition {
    int targetStrength;
    int changeRate;
    int holdTime;
};

extern ViewWeatherState *g_viewWeather;
extern ViewLightingRamp *g_viewLightingRamps;
extern CarSurfaceRamp *g_carSurfaceRamps;

// Lighting preset of one weather condition as stored in the stage files:
// three mesh-height parameters, icon size, fog limits and eleven byte
// colours (RGB plus a fourth byte used as alpha/factor by some of them).
struct StageLightPreset {
    FixVector meshHeightParameters; // 0x00
    int field_0xc;
    int sunIconSize;        // 0x10
    int fogStart;           // 0x14
    int fogEnd;             // 0x18
    BYTE colour[11][4];     // 0x1c
};

// One expanded weather lighting preset (0xb4 bytes). RGB channels are
// signed 16.16 values; alpha/weights retain their distinct scales.
struct StageLightingValues {
    FixVector lowColour;              // 0x00
    FixVector highColourDelta;        // 0x0c highColour - lowColour
    FixVector heightReferenceColour;  // 0x18
    FixVector sunIconColour;          // 0x24
    FixVector groundColour;           // 0x30
    FixVector objectColour;           // 0x3c
    FixVector ambientColour;          // 0x48
    FixVector lightColour;            // 0x54
    FixVector mesh5Colour;            // 0x60
    FixVector meshHeightParameters;   // 0x6c passed to Track_ShiftMeshAndAmbientHeights
    FixVector fogColour;              // 0x78
    FixVector skyColour;              // 0x84
    int skyAlpha;                     // 0x90
    int fogStart;                     // 0x94
    int fogEnd;                       // 0x98
    int heightReferenceBlend;         // 0x9c 0..0x10000
    int groundReferenceBlend;         // 0xa0 0..0x10000
    int objectAlpha;                  // 0xa4 16.16 byte value
    int mesh5Alpha;                   // 0xa8 16.16 byte value
    int sunIconSize;                  // 0xac scaled into screen dimensions
    int intensity;                    // 0xb0 attenuation against ambient
};

// One shared block, rather than separate globals: original stores may alias
// both presets and state. Lightning sectors occupy two signed WORDs.
struct StageLightingState {
    StageLightingValues primary;      // 0x000
    StageLightingValues secondary;    // 0x0b4
    int blend;                        // 0x168
    int intensity;                    // 0x16c
    short lightningSector;            // 0x170 -1 when no flash
    short previousLightningSector;    // 0x172
    int fogEnabled;                   // 0x174
};
extern StageLightingState g_stageLighting;


#endif
