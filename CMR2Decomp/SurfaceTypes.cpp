#include <windows.h>
#include "SurfaceTypes.h"
#include "FixedPoint.h"
#include "Car.h"

int g_surfaceDrag[87];
int g_surfaceGrip[48][2];
int g_surfaceGrip2[48][2];
int g_surface0x51e2b8[48];
int g_surfaceSoftness[48][2];
int g_surface0x51e4f8[48];
int g_surface0x51e5b8[48];
int g_surface0x51e678[48];
int g_surface0x51e738[48];
BYTE g_surfaceEffect[48][2];
BYTE g_surfaceNoise[144];
BYTE g_surfaceNext[48];
BYTE g_surfaceDragIndex[48];

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// Rebuilds the per-corner grip parameters of a car by blending the surface it
// stands on with the next one, and smooths the resulting rolling noise level.
// FUNCTION: CMR2 0x004781d0
void Car_UpdateSurfaceParams(Car *pCar, int blend)
{
    int i = 7;
    short noise = 0;
    short noiseNext = 0;
    int *pComp = (int *)((BYTE *)pCar + 0x8bc);
    short *pSurf = (short *)((BYTE *)pCar + 0xabc);
    int *pOut = (int *)((BYTE *)pCar + 0x180);
    int s0;
    unsigned short next;
    int s1;
    int gripA;
    int gripB;
    int grip2A;
    int grip2B;
    int v2b8;
    int v4f8;
    int v5b8;
    int v738;
    int dSoftA;
    int dSoftB;
    int v678;
    int diff;

    for (;;) {
        s0 = *pSurf;
        next = g_surfaceNext[s0];
        s1 = (short)next;
        diff = g_surfaceGrip[s1][0] - g_surfaceGrip[s0][0];
        gripA = FixMul(diff, blend) + g_surfaceGrip[s0][0];
        diff = g_surfaceGrip[s1][1] - g_surfaceGrip[s0][1];
        gripB = FixMul(diff, blend) + g_surfaceGrip[s0][1];
        diff = g_surfaceGrip2[s1][0] - g_surfaceGrip2[s0][0];
        grip2A = FixMul(diff, blend) + g_surfaceGrip2[s0][0];
        diff = g_surfaceGrip2[s1][1] - g_surfaceGrip2[s0][1];
        grip2B = FixMul(diff, blend) + g_surfaceGrip2[s0][1];
        diff = g_surface0x51e2b8[s1] - g_surface0x51e2b8[s0];
        v2b8 = FixMul(diff, blend) + g_surface0x51e2b8[s0];
        diff = g_surface0x51e4f8[s1] - g_surface0x51e4f8[s0];
        v4f8 = FixMul(diff, blend) + g_surface0x51e4f8[s0];
        diff = g_surface0x51e5b8[s1] - g_surface0x51e5b8[s0];
        v5b8 = FixMul(diff, blend) + g_surface0x51e5b8[s0];
        diff = g_surface0x51e738[s1] - g_surface0x51e738[s0];
        v738 = FixMul(diff, blend) + g_surface0x51e738[s0];
        diff = g_surfaceSoftness[s1][0] - g_surfaceSoftness[s0][0];
        dSoftA = FixMul(diff, blend) + g_surfaceSoftness[s0][0];
        diff = g_surfaceSoftness[s1][1] - g_surfaceSoftness[s0][1];
        dSoftB = FixMul(diff, blend) + g_surfaceSoftness[s0][1];
        diff = g_surface0x51e678[s1] - g_surface0x51e678[s0];
        v678 = FixMul(diff, blend) + g_surface0x51e678[s0];

        if (i >= 4) {
            pOut[0] = gripB;
            pOut[-1] = gripA;
            pOut[1] = grip2A;
store_grip2B:
            pOut[2] = grip2B;
        } else {
            int comp = pComp[-4];

            if (comp != 0) {
                int soft;

                if (comp > 0x10000) {
                    comp = 0x10000;
                }
                soft = FixMul(comp, dSoftA);
                pOut[-1] = soft + gripA;
                pOut[1] = soft + grip2A;
                pOut[0] = FixDiv(FixMul(gripA, gripB), pOut[-1]);
                pOut[2] = FixDiv(FixMul(grip2A, grip2B), pOut[1]);
            } else {
                pOut[1] = grip2A;
                pOut[-1] = gripA;
                pOut[0] = gripB;
                pOut[2] = grip2B;
            }
            comp = *pComp;
            if (comp != 0) {
                int soft;

                if (comp > 0x10000) {
                    comp = 0x10000;
                }
                soft = FixMul(comp, dSoftB);
                grip2B = pOut[2] + soft;
                pOut[0] = pOut[0] + soft;
                goto store_grip2B;
            }
        }

        pOut[5] = v2b8;
        pOut[4] = v738;
        pOut[7] = v5b8;
        pOut[6] = v4f8;
        pOut[3] = v678;
        i--;
        pSurf--;
        pComp--;
        pOut -= 9;
        if (i < 0) {
            int level;
            int previous;
            int diff;

            i = 4;
            pOut = (int *)((BYTE *)pCar + 0x1c8);
            pSurf = (short *)((BYTE *)pCar + 0xab4);
            do {
                int id = *pSurf;
                BYTE next = g_surfaceNext[id];
                int drag;
                int dragNext;

                *((BYTE *)pOut - 4) = g_surfaceEffect[id][0];
                *((BYTE *)pOut - 3) = g_surfaceEffect[id][1];
                drag = FixMul(g_surfaceDrag[g_surfaceDragIndex[id] + *(BYTE *)((BYTE *)pCar + 0xb29) * 9], 0x51e);
                dragNext = FixMul(g_surfaceDrag[g_surfaceDragIndex[(int)(short)(unsigned short)next] +
                                                *(BYTE *)((BYTE *)pCar + 0xb29) * 9], 0x51e);
                pOut[1] = 0;
                pOut[0] = FixMul(dragNext - drag, blend) + drag;
                pOut -= 3;
                noise = noise + (unsigned short)g_surfaceNoise[id];
                noiseNext = noiseNext + (unsigned short)g_surfaceNoise[(int)(short)(unsigned short)next];
                pSurf--;
                i--;
            } while (i != 0);

            level = FixMul(((int)noiseNext >> 2) * 0x10000 + ((int)noise & 0xfffffffc) * -0x4000, blend);
            level = FixMul(((int)noise & 0xfffffffc) * 0x4000 + level, 0x28f);
            previous = *(int *)((BYTE *)pCar + 0xa74);
            diff = level - previous;
            *(int *)((BYTE *)pCar + 0xa78) = level;
            if (FIX_ABS(diff) < 0x3334) {
                *(int *)((BYTE *)pCar + 0xa74) = level;
                return;
            }
            if (diff > 0) {
                *(int *)((BYTE *)pCar + 0xa74) = previous + 0x3333;
                return;
            }
            *(int *)((BYTE *)pCar + 0xa74) = previous - 0x3333;
            return;
        }
    }
}
