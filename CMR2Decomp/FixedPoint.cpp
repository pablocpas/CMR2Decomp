#include <windows.h>
#include "FixedPoint.h"
#include "Graphics.h"

// GLOBAL: CMR2 0x0072d67c
int g_fixMatrixMultiplyCount;

// Converts a 16.16 matrix into a Direct3D float matrix.
// FUNCTION: CMR2 0x004ba090
D3DMATRIX *FixMatrix_ToFloat(D3DMATRIX *pOut, FixMatrix *pIn)
{
    pOut->_11 = (float)pIn->right.x * CGraphics::m_oneOver65536;
    pOut->_12 = (float)pIn->right.y * CGraphics::m_oneOver65536;
    pOut->_13 = (float)pIn->right.z * CGraphics::m_oneOver65536;
    pOut->_14 = (float)pIn->rw * CGraphics::m_oneOver65536;
    pOut->_21 = (float)pIn->up.x * CGraphics::m_oneOver65536;
    pOut->_22 = (float)pIn->up.y * CGraphics::m_oneOver65536;
    pOut->_23 = (float)pIn->up.z * CGraphics::m_oneOver65536;
    pOut->_24 = (float)pIn->uw * CGraphics::m_oneOver65536;
    pOut->_31 = (float)pIn->forward.x * CGraphics::m_oneOver65536;
    pOut->_32 = (float)pIn->forward.y * CGraphics::m_oneOver65536;
    pOut->_33 = (float)pIn->forward.z * CGraphics::m_oneOver65536;
    pOut->_34 = (float)pIn->fw * CGraphics::m_oneOver65536;
    pOut->_41 = (float)pIn->position.x * CGraphics::m_oneOver65536;
    pOut->_42 = (float)pIn->position.y * CGraphics::m_oneOver65536;
    pOut->_43 = (float)pIn->position.z * CGraphics::m_oneOver65536;
    pOut->_44 = (float)pIn->pw * CGraphics::m_oneOver65536;
    return pOut;
}

// Float 4x4 matrix product out = a * b (row vectors), through local copies.
// TODO: CMR2 0x004b9ff0 (implemented, match 39%)
void FloatMatrix_Multiply(D3DMATRIX *pOut, D3DMATRIX *pA, D3DMATRIX *pB)
{
    float result[16];
    float b[16];
    float a[16];
    int i, j, k;
    float *pR;
    float *pA2;
    float *pB2;
    float *pCol;

    *(D3DMATRIX *)a = *pA;
    *(D3DMATRIX *)b = *pB;
    for (i = 0; i < 16; i += 4) {
        pB2 = b;
        pR = &result[i];
        for (j = 4; j != 0; j--) {
            *pR = 0.0f;
            pA2 = &a[i];
            pCol = pB2;
            for (k = 4; k != 0; k--) {
                *pR += *pCol * *pA2;
                pCol += 4;
                pA2++;
            }
            pR++;
            pB2++;
        }
    }
    *pOut = *(D3DMATRIX *)result;
}

// Converts a Direct3D float matrix into 16.16.
// FUNCTION: CMR2 0x004ba160
FixMatrix *FloatMatrix_ToFix(FixMatrix *pOut, D3DMATRIX *pIn)
{
    ((int *)pOut)[0] = (int)(__int64)(((float *)pIn)[0] * CGraphics::m_65536);
    ((int *)pOut)[1] = (int)(__int64)(((float *)pIn)[1] * CGraphics::m_65536);
    ((int *)pOut)[2] = (int)(__int64)(((float *)pIn)[2] * CGraphics::m_65536);
    ((int *)pOut)[3] = (int)(__int64)(((float *)pIn)[3] * CGraphics::m_65536);
    ((int *)pOut)[4] = (int)(__int64)(((float *)pIn)[4] * CGraphics::m_65536);
    ((int *)pOut)[5] = (int)(__int64)(((float *)pIn)[5] * CGraphics::m_65536);
    ((int *)pOut)[6] = (int)(__int64)(((float *)pIn)[6] * CGraphics::m_65536);
    ((int *)pOut)[7] = (int)(__int64)(((float *)pIn)[7] * CGraphics::m_65536);
    ((int *)pOut)[8] = (int)(__int64)(((float *)pIn)[8] * CGraphics::m_65536);
    ((int *)pOut)[9] = (int)(__int64)(((float *)pIn)[9] * CGraphics::m_65536);
    ((int *)pOut)[10] = (int)(__int64)(((float *)pIn)[10] * CGraphics::m_65536);
    ((int *)pOut)[11] = (int)(__int64)(((float *)pIn)[11] * CGraphics::m_65536);
    ((int *)pOut)[12] = (int)(__int64)(((float *)pIn)[12] * CGraphics::m_65536);
    ((int *)pOut)[13] = (int)(__int64)(((float *)pIn)[13] * CGraphics::m_65536);
    ((int *)pOut)[14] = (int)(__int64)(((float *)pIn)[14] * CGraphics::m_65536);
    ((int *)pOut)[15] = (int)(__int64)(((float *)pIn)[15] * CGraphics::m_65536);
    return pOut;
}

// FUNCTION: CMR2 0x004ba2b0
void FixMatrix_Identity(FixMatrix *pOut)
{
    int m[16] = { 0x10000, 0, 0, 0,
                  0, 0x10000, 0, 0,
                  0, 0, 0x10000, 0,
                  0, 0, 0, 0x10000 };

    *pOut = *(FixMatrix *)m;
}

// Applies a rotation about the matrix's right axis while preserving position.
// FUNCTION: CMR2 0x00422e70
void FixMatrix_RotateAboutRight(FixMatrix *pOut, unsigned int angle)
{
    FixVector position;
    FixMatrix rotation;
    FixVector axis;

    FixMatrix_GetPosition(&position, pOut);
    pOut->position.x = 0;
    pOut->position.y = 0;
    pOut->position.z = 0;
    FixMatrix_Identity(&rotation);

    axis.x = 0x10000;
    axis.y = 0;
    axis.z = 0;
    FixMatrix_SetRight(&axis, &rotation);

    axis.x = 0;
    axis.y = g_sinTable[(angle + 0x400) & 0xfff];
    axis.z = g_sinTable[angle & 0xfff];
    FixMatrix_SetUp(&axis, &rotation);

    axis.x = 0;
    axis.y = -g_sinTable[angle & 0xfff];
    axis.z = g_sinTable[(angle + 0x400) & 0xfff];
    FixMatrix_SetForward(&axis, &rotation);

    FixMatrix_Multiply(pOut, &rotation, pOut);
    FixMatrix_SetPosition(&position, pOut);
}

// Inverts an affine 16.16 matrix (3x3 rotation/scale plus translation) through
// its cofactors; a singular matrix gives the identity.
// FUNCTION: CMR2 0x004ba440
void FixMatrix_Invert(FixMatrix *pOut, FixMatrix *pIn)
{
    int *m;
    int *o;
    int p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17;
    int c0, c1, c2, c4, c5, c6, c8, c9, c10;
    int det;
    int tx, ty, tz;

    m = (int *)pIn;
    p0 = FixMul(m[5], m[10]);
    p1 = FixMul(m[9], m[6]);
    p2 = FixMul(m[4], m[10]);
    p3 = FixMul(m[8], m[6]);
    p4 = FixMul(m[4], m[9]);
    p5 = FixMul(m[8], m[5]);
    p6 = FixMul(m[1], m[10]);
    p7 = FixMul(m[9], m[2]);
    p8 = FixMul(m[0], m[10]);
    p9 = FixMul(m[8], m[2]);
    p10 = FixMul(m[0], m[9]);
    p11 = FixMul(m[8], m[1]);
    p12 = FixMul(m[1], m[6]);
    p13 = FixMul(m[5], m[2]);
    p14 = FixMul(m[0], m[6]);
    p15 = FixMul(m[4], m[2]);
    p16 = FixMul(m[0], m[5]);
    p17 = FixMul(m[4], m[1]);
    c0 = p0 - p1;
    c1 = p7 - p6;
    c2 = p12 - p13;
    c4 = p3 - p2;
    c5 = p8 - p9;
    c6 = p15 - p14;
    c8 = p4 - p5;
    c9 = p11 - p10;
    c10 = p16 - p17;
    det = FixMul(c0, m[0]) + FixMul(c1, m[4]) + FixMul(c2, m[8]);
    if (det == 0) {
        FixMatrix_Identity(pOut);
        return;
    }
    det = FixDiv(0x10000, det);
    o = (int *)pOut;
    o[11] = 0;
    o[7] = 0;
    o[3] = 0;
    o[0] = FixMul(det, c0);
    o[1] = FixMul(det, c1);
    o[2] = FixMul(det, c2);
    o[4] = FixMul(det, c4);
    o[5] = FixMul(det, c5);
    o[6] = FixMul(det, c6);
    o[8] = FixMul(det, c8);
    o[9] = FixMul(det, c9);
    o[10] = FixMul(det, c10);
    tx = -m[12];
    ty = -m[13];
    tz = -m[14];
    o[12] = FixMul(tx, o[0]) + FixMul(ty, o[4]) + FixMul(tz, o[8]);
    o[13] = FixMul(tx, o[1]) + FixMul(ty, o[5]) + FixMul(tz, o[9]);
    o[14] = FixMul(tx, o[2]) + FixMul(ty, o[6]) + FixMul(tz, o[10]);
    o[15] = 0x10000;
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

// Rotates a float vector through the matrix columns.
// FUNCTION: CMR2 0x004b9d40
int FloatMatrix_RotateVector(float *pOut, float *pV, float *pM)
{
    pOut[0] = pV[0] * pM[0] + pM[4] * pV[1] + pM[8] * pV[2];
    pOut[1] = pM[1] * pV[0] + pM[5] * pV[1] + pM[9] * pV[2];
    pOut[2] = pM[2] * pV[0] + pM[6] * pV[1] + pM[10] * pV[2];
    return 1;
}

// Rotates a float vector by the three rows of a float 4x4 matrix.
// FUNCTION: CMR2 0x004b9ec0
int FloatMatrix_InverseRotateVector(float *pOut, float *pV, float *pM)
{
    double first = pM[2] * pV[2];
    first += pM[1] * pV[1];
    pOut[0] = (float)(first + pM[0] * pV[0]);
    pOut[1] = pM[5] * pV[1] + (pM[4] * pV[0] + pM[6] * pV[2]);
    pOut[2] = pM[9] * pV[1] + (pM[8] * pV[0] + pM[10] * pV[2]);
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

// Builds the rotation matrix for a rotation of angle (12-bit) about the
// unit axis (Rodrigues), with the scratch products left in the globals.
// FUNCTION: CMR2 0x004adb10
void FixMatrix_FromAxisAngle(FixMatrix *pOut, FixVector *pAxis, int angle)
{
    int neg;

    neg = -angle;
    g_rotSin = g_sinTable[neg & 0xfff];
    g_rotCos = g_sinTable[(neg + 0x400) & 0xfff];
    g_rotOneMinusCos = 0x10000 - g_rotCos;
    g_rotAxisXX = FixMul(pAxis->x, pAxis->x);
    g_rotAxisYY = FixMul(pAxis->y, pAxis->y);
    g_rotAxisZZ = FixMul(pAxis->z, pAxis->z);
    g_rotAxisXY = FixMul(FixMul(pAxis->x, pAxis->y), g_rotOneMinusCos);
    g_rotAxisXZ = FixMul(FixMul(pAxis->x, pAxis->z), g_rotOneMinusCos);
    g_rotAxisYZ = FixMul(FixMul(pAxis->z, pAxis->y), g_rotOneMinusCos);

    pOut->right.x = FixMul(g_rotCos, 0x10000 - g_rotAxisXX) + g_rotAxisXX;
    pOut->right.y = g_rotAxisXY - FixMul(pAxis->z, g_rotSin);
    pOut->right.z = FixMul(pAxis->y, g_rotSin) + g_rotAxisXZ;
    pOut->rw = 0;
    pOut->up.x = FixMul(pAxis->z, g_rotSin) + g_rotAxisXY;
    pOut->up.y = FixMul(g_rotCos, 0x10000 - g_rotAxisYY) + g_rotAxisYY;
    pOut->up.z = g_rotAxisYZ - FixMul(pAxis->x, g_rotSin);
    pOut->uw = 0;
    pOut->forward.x = g_rotAxisXZ - FixMul(pAxis->y, g_rotSin);
    pOut->forward.y = FixMul(pAxis->x, g_rotSin) + g_rotAxisYZ;
    pOut->forward.z = FixMul(g_rotCos, 0x10000 - g_rotAxisZZ) + g_rotAxisZZ;
    pOut->fw = 0;
    pOut->position.x = 0;
    pOut->position.y = 0;
    pOut->position.z = 0;
    pOut->pw = 0x10000;
}

// Rotation about the Z axis by a 12-bit angle.
// FUNCTION: CMR2 0x004ba320
void FixMatrix_RotationZ(FixMatrix *pOut, unsigned int angle)
{
    int m[16] = { g_sinTable[(angle + 0x400) & 0xfff], FixSin(angle), 0, 0,
                  -FixSin(angle), g_sinTable[(angle + 0x400) & 0xfff], 0, 0,
                  0, 0, 0x10000, 0,
                  0, 0, 0, 0x10000 };

    *pOut = *(FixMatrix *)m;
}

// out = in * M (rotation plus translation).
// FUNCTION: CMR2 0x004baa40
void FixMatrix_TransformPoint(FixVector *pOut, FixVector *pIn, FixMatrix *pM)
{
    pOut->x = FixMul(pM->right.x, pIn->x) + FixMul(pM->up.x, pIn->y) + FixMul(pM->forward.x, pIn->z) + pM->position.x;
    pOut->y = FixMul(pM->right.y, pIn->x) + FixMul(pM->up.y, pIn->y) + FixMul(pM->forward.y, pIn->z) + pM->position.y;
    pOut->z = FixMul(pM->right.z, pIn->x) + FixMul(pM->up.z, pIn->y) + FixMul(pM->forward.z, pIn->z) + pM->position.z;
}

// Applies pM to pIn about the pivot (x, y): translate(-pivot), pM,
// translate(+pivot). pIn is updated with each intermediate result.
// FUNCTION: CMR2 0x004ba3b0
void FixMatrix_TransformAboutPivot(FixVector *pOut, FixVector *pIn, FixVector *pPivot, FixMatrix *pM)
{
    FixMatrix t;

    FixMatrix_Identity(&t);
    t.position.x = -pPivot->x;
    t.position.y = -pPivot->y;
    FixMatrix_TransformPoint(pOut, pIn, &t);
    pIn->x = pOut->x;
    pIn->y = pOut->y;
    pIn->z = pOut->z;
    FixMatrix_TransformPoint(pOut, pIn, pM);
    pIn->x = pOut->x;
    pIn->y = pOut->y;
    pIn->z = pOut->z;
    t.position.x = pPivot->x;
    t.position.y = pPivot->y;
    FixMatrix_TransformPoint(pOut, pIn, &t);
}

// FUNCTION: CMR2 0x004babe0
void FixMatrix_CopyRotationFrom(FixMatrix *pDst, FixMatrix *pSrc)
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

// Length of a vector; components beyond +-100.0 are scaled down by 512
// first so the squares do not overflow.
// FUNCTION: CMR2 0x004bb0a0
unsigned int FixVec_Length(FixVector *pV)
{
    FixVector scaled;

    if (pV->x <= 0x640000 && pV->x >= -0x640000 && pV->y <= 0x640000 && pV->y >= -0x640000 && pV->z <= 0x640000 && pV->z >= -0x640000)
        return FixVecLength(pV);
    scaled.x = pV->x / 512;
    scaled.y = pV->y / 512;
    scaled.z = pV->z / 512;
    return FixVecLength(&scaled) << 9;
}

// Normalises a vector; components beyond +-100.0 are scaled down by 512
// first so the squares do not overflow.
// FUNCTION: CMR2 0x004bae10
void FixVec_Normalize(FixVector *pOut, FixVector *pIn)
{
    FixVector scaled;
    int len;

    if (pIn->x <= 0x640000 && pIn->x >= -0x640000 && pIn->y <= 0x640000 && pIn->y >= -0x640000 && pIn->z <= 0x640000 && pIn->z >= -0x640000) {
        len = FixVecLength(pIn);
        if (len == 0) {
            pOut->x = 0;
            pOut->y = 0;
            pOut->z = 0;
            return;
        }
        FixVecScaleRecip(pOut, pIn, len);
        return;
    }
    scaled.x = pIn->x / 512;
    scaled.y = pIn->y / 512;
    scaled.z = pIn->z / 512;
    len = FixVecLength(&scaled);
    if (len == 0) {
        pOut->x = 0;
        pOut->y = 0;
        pOut->z = 0;
        return;
    }
    FixVecScaleRecip(pOut, &scaled, len);
}

// Scratch vectors of FixMatrix_Interpolate
// GLOBAL: CMR2 0x005391d8
FixVector g_interpA;
// GLOBAL: CMR2 0x005391b8
FixVector g_interpB;
// GLOBAL: CMR2 0x005391e8
FixVector g_interpRight;
// GLOBAL: CMR2 0x00538e20
FixVector g_interpUp;
// GLOBAL: CMR2 0x00539008
FixVector g_interpForward;
// GLOBAL: CMR2 0x00538e10
FixVector g_interpPos;

#define FIX_LERP(out, a, b, t)                                                      \
    out.x = b.x - a.x;                                                              \
    out.y = b.y - a.y;                                                              \
    out.z = b.z - a.z;                                                              \
    FixVecScale(&out, &out, t);                                                     \
    out.x += a.x;                                                                   \
    out.y += a.y;                                                                   \
    out.z += a.z;

#define FIX_NORMALIZE(v)                                                            \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            v.x = 0;                                                                \
            v.y = 0;                                                                \
            v.z = 0;                                                                \
        } else {                                                                    \
            FixVecScaleRecip(&v, &v, len);                                          \
        }                                                                           \
    }

// Interpolates between two matrices: the right vector and either the up
// (mode 0) or forward vector are lerped and renormalised, the third axis
// comes from cross products; the position is lerped with tPos.
// FUNCTION: CMR2 0x004224e0
void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode)
{
    FixMatrix_GetRight(&g_interpA, pA);
    FixMatrix_GetRight(&g_interpB, pB);
    FIX_LERP(g_interpRight, g_interpA, g_interpB, tRight)
    if (mode != 0) {
        FixMatrix_GetForward(&g_interpA, pA);
        FixMatrix_GetForward(&g_interpB, pB);
        FIX_LERP(g_interpForward, g_interpA, g_interpB, tAxis)
        FIX_NORMALIZE(g_interpForward)
        FixVecCross(&g_interpUp, &g_interpForward, &g_interpRight);
        FIX_NORMALIZE(g_interpUp)
    } else {
        FixMatrix_GetUp(&g_interpA, pA);
        FixMatrix_GetUp(&g_interpB, pB);
        FIX_LERP(g_interpUp, g_interpA, g_interpB, tAxis)
        FIX_NORMALIZE(g_interpUp)
        FixVecCross(&g_interpForward, &g_interpRight, &g_interpUp);
        FIX_NORMALIZE(g_interpForward)
    }
    FixVecCross(&g_interpRight, &g_interpUp, &g_interpForward);
    FixMatrix_GetPosition(&g_interpA, pA);
    FixMatrix_GetPosition(&g_interpB, pB);
    FIX_LERP(g_interpPos, g_interpA, g_interpB, tPos)
    FixMatrix_Identity(pOut);
    FixMatrix_SetRight(&g_interpRight, pOut);
    FixMatrix_SetUp(&g_interpUp, pOut);
    FixMatrix_SetForward(&g_interpForward, pOut);
    FixMatrix_SetPosition(&g_interpPos, pOut);
}

// Rotates a basis by the angular velocity pW: each row gains pW x row.
// FUNCTION: CMR2 0x00441430
void FixBasis_Integrate(FixVector *pRows, FixVector *pW)
{
    FixVector delta;
    int i = 3;

    do {
        FixVector *pRow = pRows;
        FixVecCross(&delta, pW, pRow);
        pRow->x = pRow->x + delta.x;
        pRow->y = pRow->y + delta.y;
        pRow->z = pRow->z + delta.z;
        pRows++;
        i--;
    } while (i != 0);
}
