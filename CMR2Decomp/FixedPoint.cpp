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

// out = v.x * right + v.y * up + v.z * forward (local -> parent space).
// FUNCTION: CMR2 0x004b9c20
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM)
{
    pOut->x = FixMul(pM->right.x, pV->x) + FixMul(pM->up.x, pV->y) + FixMul(pM->forward.x, pV->z);
    pOut->y = FixMul(pM->right.y, pV->x) + FixMul(pM->up.y, pV->y) + FixMul(pM->forward.y, pV->z);
    pOut->z = FixMul(pM->right.z, pV->x) + FixMul(pM->up.z, pV->y) + FixMul(pM->forward.z, pV->z);
    return 1;
}

// out = (v . right, v . up, v . forward) (parent -> local space).
// FUNCTION: CMR2 0x004b9da0
int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM)
{
    pOut->x = FixMul(pM->right.x, pV->x) + FixMul(pM->right.y, pV->y) + FixMul(pM->right.z, pV->z);
    pOut->y = FixMul(pM->up.x, pV->x) + FixMul(pM->up.y, pV->y) + FixMul(pM->up.z, pV->z);
    pOut->z = FixMul(pM->forward.x, pV->x) + FixMul(pM->forward.y, pV->y) + FixMul(pM->forward.z, pV->z);
    return 1;
}

// FUNCTION: CMR2 0x004bac40
void FixMatrix_GetPosition(FixVector *pOut, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->position;
    *pOut = *p;
}

// FUNCTION: CMR2 0x004bac60
void FixMatrix_GetRight(FixVector *pOut, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->right;
    *pOut = *p;
}

// FUNCTION: CMR2 0x004bac80
void FixMatrix_GetUp(FixVector *pOut, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->up;
    *pOut = *p;
}

// FUNCTION: CMR2 0x004baca0
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->forward;
    *pOut = *p;
}

// FUNCTION: CMR2 0x004bacc0
void FixMatrix_SetPosition(FixVector *pV, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->position;
    *p = *pV;
}

// FUNCTION: CMR2 0x004bace0
void FixMatrix_SetRight(FixVector *pV, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->right;
    *p = *pV;
}

// FUNCTION: CMR2 0x004bad00
void FixMatrix_SetUp(FixVector *pV, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->up;
    *p = *pV;
}

// FUNCTION: CMR2 0x004bad20
void FixMatrix_SetForward(FixVector *pV, FixMatrix *pM)
{
    FixVector *p;

    p = &pM->forward;
    *p = *pV;
}
