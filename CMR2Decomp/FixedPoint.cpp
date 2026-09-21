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

// Copies the rotation part (right/up/forward) of a matrix.
// FUNCTION: CMR2 0x004bab80
void FixMatrix_CopyRotation(FixMatrix *pSrc, FixMatrix *pDst)
{
    pDst->right.x = pSrc->right.x;
    pDst->right.y = pSrc->right.y;
    pDst->right.z = pSrc->right.z;
    pDst->up.x = pSrc->up.x;
    pDst->up.y = pSrc->up.y;
    pDst->up.z = pSrc->up.z;
    pDst->forward.x = pSrc->forward.x;
    pDst->forward.y = pSrc->forward.y;
    pDst->forward.z = pSrc->forward.z;
    pDst->position.x = pSrc->position.x;
    pDst->position.y = pSrc->position.y;
    pDst->position.z = pSrc->position.z;
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

// Rodrigues rotation of vectors a and b about the unit axis k, in place,
// with the scratch values in locals (see ROTATE_ABOUT_AXIS in SceneNode.cpp
// for the version using globals).
#define ROTATE_BASIS(k, a, b, angle)                                                  \
    {                                                                                 \
        int s, c, omc, kxx, kyy, kzz, kxs, kys, kzs, kxy, kxz, kyz;                  \
        int r00, r01, r02, r10, r11, r12, r20, r21, r22;                              \
        FixVector v;                                                                  \
        int len;                                                                      \
                                                                                      \
        s = FixSin(-(angle));                                                         \
        c = FixCos(angle);                                                            \
        omc = 0x10000 - c;                                                            \
        kxx = FixMul(k.x, k.x);                                                       \
        kyy = FixMul(k.y, k.y);                                                       \
        kzz = FixMul(k.z, k.z);                                                       \
        kxs = FixMul(k.x, s);                                                         \
        kys = FixMul(k.y, s);                                                         \
        kzs = FixMul(k.z, s);                                                         \
        kxy = FixMul(FixMul(k.x, k.y), omc);                                          \
        kxz = FixMul(FixMul(k.x, k.z), omc);                                          \
        kyz = FixMul(FixMul(k.z, k.y), omc);                                          \
                                                                                      \
        r00 = FixMul(c, 0x10000 - kxx) + kxx;                                         \
        r01 = kxy - kzs;                                                              \
        r02 = kys + kxz;                                                              \
        r10 = kzs + kxy;                                                              \
        r11 = FixMul(c, 0x10000 - kyy) + kyy;                                         \
        r21 = kxs + kyz;                                                              \
        r12 = kyz - kxs;                                                              \
        r20 = kxz - kys;                                                              \
        r22 = FixMul(c, 0x10000 - kzz) + kzz;                                         \
                                                                                      \
        v.x = FixMul(r00, a.x) + FixMul(r10, a.y) + FixMul(r20, a.z);                 \
        v.y = FixMul(r01, a.x) + FixMul(r11, a.y) + FixMul(r21, a.z);                 \
        v.z = FixMul(r02, a.x) + FixMul(r12, a.y) + FixMul(r22, a.z);                 \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            a.x = 0;                                                                  \
            a.y = 0;                                                                  \
            a.z = 0;                                                                  \
        } else {                                                                      \
            FixVecScaleRecip(&a, &v, len);                                            \
        }                                                                             \
                                                                                      \
        v.x = FixMul(r00, b.x) + FixMul(r10, b.y) + FixMul(r20, b.z);                 \
        v.y = FixMul(r01, b.x) + FixMul(r11, b.y) + FixMul(r21, b.z);                 \
        v.z = FixMul(r02, b.x) + FixMul(r12, b.y) + FixMul(r22, b.z);                 \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            b.x = 0;                                                                  \
            b.y = 0;                                                                  \
            b.z = 0;                                                                  \
        } else {                                                                      \
            FixVecScaleRecip(&b, &v, len);                                            \
        }                                                                             \
    }

// Rotates a basis in place by the three 12-bit angles (about up, then
// forward, then right).
// FUNCTION: CMR2 0x00429f20
void FixBasis_Rotate(FixBasis *pBasis, unsigned short *pAngles)
{
    if (pAngles[1] != 0)
        ROTATE_BASIS(pBasis->up, pBasis->right, pBasis->forward, pAngles[1])
    if (pAngles[2] != 0)
        ROTATE_BASIS(pBasis->forward, pBasis->right, pBasis->up, pAngles[2])
    if (pAngles[0] != 0)
        ROTATE_BASIS(pBasis->right, pBasis->up, pBasis->forward, pAngles[0])
}
