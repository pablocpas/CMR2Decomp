#ifndef _AI_REFERENCE_ROUTE_H
#define _AI_REFERENCE_ROUTE_H

// ON-DISK: four fixed arrays of 370 signed words in an AI archive (0x1720
// bytes); no embedded pointers. Loading translates x/z by the route origin.
struct AIReferenceRoute {
    int headingDegrees[370]; // 0x0000 fixed degrees, compared with body heading
    int speedSamples[370];   // 0x05c8 speed comparison after fixed conversion
    int positionX[370];      // 0x0b90
    int positionZ[370];      // 0x1158
};

extern AIReferenceRoute *g_aiReferenceRoutes[9];

#endif
