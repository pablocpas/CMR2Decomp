#ifndef _SURFACE_TYPES_H
#define _SURFACE_TYPES_H

// Per-surface-type driving parameters, 48 surfaces. Every table is indexed by
// the surface id stored per wheel; the car blends between the surface it is on
// and the one it is crossing into.

// GLOBAL: CMR2 0x0051de5c
extern int g_surfaceDrag[87];
// GLOBAL: CMR2 0x0051dfb8
extern int g_surfaceGrip[48][2];
// GLOBAL: CMR2 0x0051e138
extern int g_surfaceGrip2[48][2];
// GLOBAL: CMR2 0x0051e2b8
extern int g_surface0x51e2b8[48];
// GLOBAL: CMR2 0x0051e378
extern int g_surfaceSoftness[48][2];
// GLOBAL: CMR2 0x0051e4f8
extern int g_surface0x51e4f8[48];
// GLOBAL: CMR2 0x0051e5b8
extern int g_surface0x51e5b8[48];
// GLOBAL: CMR2 0x0051e678
extern int g_surface0x51e678[48];
// GLOBAL: CMR2 0x0051e738
extern int g_surface0x51e738[48];
// GLOBAL: CMR2 0x0051e7f8
extern BYTE g_surfaceEffect[48][2];
// GLOBAL: CMR2 0x0051e858
extern BYTE g_surfaceNoise[144];
// GLOBAL: CMR2 0x0051e8e8
extern BYTE g_surfaceNext[48];
// GLOBAL: CMR2 0x0051e918
extern BYTE g_surfaceDragIndex[48];

struct Car;
void Car_UpdateSurfaceParams(Car *pCar, int blend);

#endif
