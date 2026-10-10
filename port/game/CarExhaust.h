#ifndef _CAR_EXHAUST_H
#define _CAR_EXHAUST_H

#include "FixedPoint.h"

struct CarLightPoint;
struct GlowLight;

// The CIN light points of type 9 select the two exhaust emitters. These are
// independent per-car arrays, not two texture tables or a contiguous record.
extern CarLightPoint *g_carExhaustPoints[8][2];
extern GlowLight *g_carExhaustGlows[8][2];
extern FixVector g_carExhaustWorldPositions[8];
extern int g_exhaustParticleSide;
extern unsigned char g_carExhaustBurstFrames[8];
void CarExhaust_SetPoint(CarLightPoint *point, int side, int car);
void CarExhaust_SetGlow(GlowLight *glow, int side, int car);

#endif
