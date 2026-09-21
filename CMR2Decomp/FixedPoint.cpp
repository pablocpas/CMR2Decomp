#include <windows.h>
#include "FixedPoint.h"

// GLOBAL: CMR2 0x0072d67c
int g_fixMatrixMultiplyCount;

// FUNCTION: CMR2 0x004ba2b0
void FixMatrix_Identity(FixMatrix *pOut)
{
    int m[16] = { 0x10000, 0, 0, 0,
                  0, 0x10000, 0, 0,
                  0, 0, 0x10000, 0,
                  0, 0, 0, 0x10000 };

    *pOut = *(FixMatrix *)m;
}

// FUNCTION: CMR2 0x004b9f20
void FixMatrix_Multiply(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB)
{
    int b[16];
    int a[16];
    int result[16];
    int ai, bi;
    int i, j, k;
    int *pR;
    int *pA2;
    int *pB2;

    g_fixMatrixMultiplyCount++;
    *(FixMatrix *)a = *pA;
    *(FixMatrix *)b = *pB;
    for (i = 0; i < 4; i++) {
        pR = &result[i * 4];
        for (j = 4; j != 0; j--) {
            *pR = 0;
            pA2 = &a[i * 4];
            pB2 = &b[4 - j];
            for (k = 4; k != 0; k--) {
                ai = *pA2;
                bi = *pB2;
                *pR += FixMul(ai, bi);
                pA2++;
                pB2 += 4;
            }
            pR++;
        }
    }
    *pOut = *(FixMatrix *)result;
}
