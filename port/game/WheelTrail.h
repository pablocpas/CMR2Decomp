#ifndef _WHEEL_TRAIL_H
#define _WHEEL_TRAIL_H

#include "FixedPoint.h"

// Per-car, per-wheel state of the dust / skid trail left on the ground.

// GLOBAL: CMR2 0x005430a0
extern int g_trailTimer[8][4];
// GLOBAL: CMR2 0x00543120
extern char g_trailLevel[8][4];
// GLOBAL: CMR2 0x00543140
extern int g_trailReset[8][4];
// GLOBAL: CMR2 0x00543200
extern FixVector g_trailPos[8][4];
// GLOBAL: CMR2 0x00543400
extern FixVector g_trailLastPos[8][4];
// GLOBAL: CMR2 0x005435d0
extern int g_trailSurface[8][4];
// GLOBAL: CMR2 0x00543708
extern int g_unk0x00543708[8][4];
// GLOBAL: CMR2 0x00543788
extern int g_trailState[8][4];
// GLOBAL: CMR2 0x00543830
extern FixVector g_trailOffset[8][4];
// GLOBAL: CMR2 0x005439b8
extern int g_trailCount[8];
// GLOBAL: CMR2 0x00543ad8
extern int g_trailPrevState[8][4];
// GLOBAL: CMR2 0x00543b58
extern FixVector g_trailDelta[8][4];

void WheelTrail_Update(int carIndex);

#endif
