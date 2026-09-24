#include <windows.h>
#include "SurfaceTypes.h"
#include "FixedPoint.h"
#include "Car.h"

int g_surfaceDrag[63] = {
    0, 0, -65536, -32768, -65536, -65536, -65536, -131072,
    -98304, 0, -49152, 0, -32768, -32768, -32768, -32768,
    -98304, -65536, 0, -16384, -32768, 0, -16384, -16384,
    -32768, -98304, -32768, 0, -32768, -16384, -16384, 0,
    -32768, -16384, -32768, 0, 0, -16384, -32768, -16384,
    -32768, 0, -16384, -65536, -32768, 0, -32768, -16384,
    -32768, -16384, -16384, 0, -32768, 0, 0, -65536,
    -49152, -65536, -16384, -32768, -32768, 0, -16384,
};
// GLOBAL: CMR2 0x0051df58
BYTE g_surfaceIndexMap[96] = {
    0x19,0x18,0x06,0x0b,0x0a,0x09,0x08,0x07,0x0b,0x0a,0x09,0x08,0x07,0x0b,0x0b,0x0a,
    0x0b,0x01,0x01,0x0d,0x0d,0x05,0x05,0x05,0x05,0x02,0x0c,0x0c,0x0c,0x0d,0x02,0x0c,
    0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x0c,0x0c,0x0c,0x10,0x03,0x03,
    0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,0x0d,0x10,0x0f,0x0f,0x12,0x16,0x17,
    0x1e,0x11,0x14,0x15,0x13,0x1d,0x0e,0x0c,0x01,0x05,0x05,0x0d,0x0c,0x05,0x05,0x05,
    0x0c,0x09,0x13,0x1a,0x0e,0x0e,0x0f,0x0f,0x0f,0x0f,0x0e,0x1b,0x1b,0x1b,0x1b,0x1c
};

// FUNCTION: CMR2 0x00478a10
short FUN_00478a10(short index)
{
    return g_surfaceIndexMap[index];
}
int g_surfaceGrip[48][2] = {
    15073, 98304, 14745, 91750, 13107, 78643, 9830, 78643,
    13107, 78643, 13107, 78643, 15400, 91750, 14090, 91750,
    14417, 91750, 14745, 91750, 14417, 91750, 15400, 91750,
    15728, 98304, 15728, 98304, 15728, 137625, 15728, 137625,
    15728, 137625, 12451, 85196, 11796, 88473, 13107, 81920,
    13107, 176947, 13107, 176947, 13107, 81920, 13107, 73400,
    14417, 203161, 15728, 207093, 15728, 98304, 15400, 91750,
    14745, 91750, 14417, 203161, 13107, 79953, 13434, 91750,
    14090, 91750, 14090, 91750, 12451, 98304, 13107, 98304,
    13107, 98304, 13107, 98304, 7864, 78643, 9830, 78643,
    9830, 78643, 11796, 137625, 11141, 176947, 13762, 176947,
    14090, 91750, 13434, 91750, 11141, 176947, 15728, 207093,
};
int g_surfaceGrip2[48][2] = {
    12451, 98304, 12124, 91750, 10485, 78643, 7208, 78643,
    10485, 78643, 10485, 78643, 12779, 91750, 11468, 91750,
    11796, 91750, 12451, 91750, 12779, 91750, 13107, 91750,
    13107, 98304, 13107, 98304, 13107, 137625, 13107, 137625,
    13107, 137625, 9830, 88473, 9175, 88473, 10485, 81920,
    10485, 176947, 10485, 176947, 10485, 88473, 10485, 85852,
    13762, 196608, 14417, 204472, 13107, 98304, 12779, 91750,
    12124, 91750, 13762, 196608, 10485, 92405, 10813, 91750,
    11796, 91750, 11468, 91750, 9830, 98304, 10485, 98304,
    10485, 98304, 10485, 98304, 5242, 78643, 7864, 78643,
    7864, 78643, 9830, 137625, 10485, 173670, 13107, 173670,
    11468, 91750, 10813, 91750, 10485, 173670, 14417, 204472,
};
int g_surface0x51e2b8[48] = {
    0, 3932, 0, 0, 0, 0, 2621, 6553,
    5242, 7864, 6553, 3932, 0, 0, 0, 0,
    0, 1966, 0, 0, 0, 0, 3932, 13762,
    0, 0, 0, 3932, 3932, 0, 13762, 6553,
    6553, 5242, 3932, 3932, 3932, 3932, 0, 0,
    0, 0, 0, 0, 3932, 6553,
};
int g_surfaceSoftness[48][2] = {
    0, 0, 2621, 6553, 0, 0, 0, 0,
    0, 0, 0, 0, 2621, 6553, 3932, 6553,
    3276, 6553, 3932, 6553, 3276, 6553, 2621, 6553,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 13107, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 26869, 0, 36700,
    0, 0, 0, 0, 0, 0, 2621, 6553,
    2621, 6553, 0, 0, 0, 36700, 2621, 6553,
    2621, 6553, 2621, 6553, 1310, 3276, 1310, 3276,
    1310, 3276, 1310, 3276, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    2621, 6553, 2621, 6553,
};
int g_surface0x51e4f8[48] = {
    0, 9175, 5242, 5242, 2621, 2621, 3276, 4587,
    2621, 4587, 2621, 2621, 5242, 0, 0, 0,
    7864, 0, 0, 5242, 2621, 2621, 0, 0,
    0, 0, 5242, 3932, 9175, 0, 0, 9175,
    2621, 3276, 0, 0, 3276, 3276, 3276, 3276,
    3276, 7864, 0, 0, 3932, 9175,
};
int g_surface0x51e5b8[48] = {
    0, 5242, 2621, 1310, 327, 327, 458, 65,
    196, 196, 327, 655, 2621, 0, 0, 0,
    1310, 0, 0, 524, 524, 524, 0, 0,
    0, 0, 2621, 655, 5242, 0, 0, 5242,
    655, 458, 0, 0, 2621, 2621, 2621, 2621,
    2621, 5242, 0, 0, 655, 5242,
};
int g_surface0x51e678[48] = {
    13107, 0, 9830, 13107, 9830, 4915, 0, 3276,
    1638, 0, 0, 0, 11141, 0, 52428, 327680,
    0, 0, 0, 26214, 13107, 13107, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 16384, 0, 6553, 4915, 16384, 9830,
    4915,
};
int g_surface0x51e738[48] = {
    13107, 13107, 0, 0, 0, 0, 13107, 19660,
    16384, 19660, 16384, 13107, 32768, 13107, 32768, 32768,
    16384, 6553, 3276, 13107, 13107, 13107, 15073, 12451,
    13107, 16384, 13107, 13107, 13107, 13107, 12451, 13107,
    13107, 13107, 19660, 19660, 32768, 19660, 0, 0,
    0, 16384, 13107, 16384, 13107, 13107, 13107, 16384,
};
BYTE g_surfaceEffect[48][2] = {
    0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64,
    0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64,
    0x32, 0x50, 0x50, 0x8c, 0x50, 0x8c, 0x50, 0x8c, 0x50, 0x8c, 0x50, 0x8c, 0x50, 0x8c, 0x50, 0x8c,
    0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x50, 0x8c, 0x4b, 0x64,
    0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64,
    0x4b, 0x64, 0x32, 0x50, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64, 0x4b, 0x64,
};
BYTE g_surfaceNoise[48] = {
    0x50, 0x50, 0x50, 0x64, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x64, 0x64,
    0x64, 0x4b, 0x46, 0x50, 0x64, 0x82, 0x50, 0x50, 0x6e, 0x82, 0x50, 0x50, 0x50, 0x6e, 0x50, 0x50,
    0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x64, 0x50, 0x50, 0x64, 0x6e, 0x64, 0x50, 0x50, 0x6e, 0x82,
};
// Blend rate of each surface towards its "next" surface (see 0x4789d0).
unsigned short g_surfaceBlendRate[48] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
    0x3333, 0x3333, 0x4ccc, 0x4ccc, 0x4ccc, 0xb333, 0xb333, 0xb333, 0x3333, 0x3333, 0xb333,
};
BYTE g_surfaceNext[48] = {
    0x22, 0x1f, 0x02, 0x26, 0x27, 0x28, 0x21, 0x07, 0x08, 0x09, 0x0a, 0x20, 0x24, 0x23, 0x0e, 0x0f,
    0x29, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x2a, 0x2b, 0x25, 0x2c, 0x2d, 0x2e, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
};
BYTE g_surfaceDragIndex[48] = {
    0x03, 0x05, 0x08, 0x08, 0x08, 0x08, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03, 0x00, 0x00,
    0x01, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x01, 0x01, 0x03, 0x05, 0x05, 0x01, 0x07, 0x06,
    0x06, 0x06, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x02, 0x02, 0x02, 0x06, 0x06, 0x02, 0x02,
};

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
