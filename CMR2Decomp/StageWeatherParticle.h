#ifndef _STAGE_WEATHER_PARTICLE_H
#define _STAGE_WEATHER_PARTICLE_H

#include <windows.h>

// One of the shared 400 precipitation particles (0x24 bytes). Rain stores
// the ground height in the same word snow uses as a phase in degrees.
struct StageWeatherParticle {
    int x;                            // 0x00 position in 16.16
    int y;                            // 0x04
    int z;                            // 0x08
    int spin;                         // 0x0c snow phase velocity
    int swayAmplitude;                // 0x10 random 16.16 snow sway amplitude
    union {
        int snowPhaseDegrees;         // 0x14
        int splashGroundHeight;        // 0x14
    };
    int scale;                        // 0x18 random wind drift factor
    BYTE field_0x1c;                  // 0x1c initialized to 0x4b; role unproved
    BYTE textureVariant;              // 0x1d one of three snow textures
    BYTE field_0x1e[2];               // 0x1e no observed reader/writer
    int wrapped;                      // 0x20 wrapped through the bottom
};
extern StageWeatherParticle g_weatherParticles[400];

#endif
