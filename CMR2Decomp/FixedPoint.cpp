#include <windows.h>
#include "FixedPoint.h"
#include "Graphics.h"
#include "GameInfo.h"
#include "Car.h"
#include "StageTiming.h"

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
// FUNCTION: CMR2 0x004b9ff0
D3DMATRIX *FloatMatrix_Multiply(D3DMATRIX *pOut, D3DMATRIX *pA, D3DMATRIX *pB)
{
    float a[4][4];
    float b[4][4];
    float result[4][4];
    int i, j, k;

    *(D3DMATRIX *)a = *pA;
    *(D3DMATRIX *)b = *pB;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            result[i][j] = 0.0f;
            for (k = 0; k < 4; k++) {
                float value = b[k][j];
                result[i][j] += value * a[i][k];
            }
        }
    }
    *pOut = *(D3DMATRIX *)result;
    return pOut;
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

// Projects a point through a 16.16 view-projection matrix to screen offsets
// from the centre (16.16 pixels, y up). The z row is computed but unused.
extern const float g_unk0x00511424;

// FUNCTION: CMR2 0x004ba870
void FixMatrix_ProjectToScreen(int *pOut, FixVector *pV, int *pM)
{
    float halfH;
    float halfW;
    float x;
    float y;
    float w;
    int z;

    halfH = (float)(int)g_pGraphics->resY * g_unk0x00511424;
    halfW = (float)(int)g_pGraphics->resX * g_unk0x00511424;
    x = (float)(FixMul(pM[0], pV->x) + FixMul(pM[4], pV->y) + FixMul(pM[8], pV->z) + pM[12]) *
        CGraphics::m_oneOver65536;
    y = (float)(FixMul(pM[1], pV->x) + FixMul(pM[5], pV->y) + FixMul(pM[9], pV->z) + pM[13]) *
        CGraphics::m_oneOver65536;
    z = FixMul(pM[2], pV->x) + FixMul(pM[6], pV->y) + FixMul(pM[10], pV->z);
    w = (float)((int)(FixMul(pM[3], pV->x) + FixMul(pM[7], pV->y) + FixMul(pM[11], pV->z) + pM[15]) *
                CGraphics::m_oneOver65536);
    pOut[0] = (int)(__int64)(x / (w / halfW) * CGraphics::m_65536);
    pOut[1] = (int)(__int64)(-(y / (w / halfH)) * CGraphics::m_65536);
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
void FixMatrix_RotateAboutRight(FixMatrix *pOut, unsigned short angle)
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
    p1 = FixMul(m[6], m[9]);
    p2 = FixMul(m[4], m[10]);
    p3 = FixMul(m[6], m[8]);
    p4 = FixMul(m[4], m[9]);
    p5 = FixMul(m[5], m[8]);
    p6 = FixMul(m[1], m[10]);
    p7 = FixMul(m[2], m[9]);
    p8 = FixMul(m[0], m[10]);
    p9 = FixMul(m[2], m[8]);
    p10 = FixMul(m[0], m[9]);
    p11 = FixMul(m[1], m[8]);
    p12 = FixMul(m[1], m[6]);
    p13 = FixMul(m[2], m[5]);
    p14 = FixMul(m[0], m[6]);
    p15 = FixMul(m[2], m[4]);
    p16 = FixMul(m[0], m[5]);
    p17 = FixMul(m[1], m[4]);
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

// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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
                *pR += FixMul(bi, ai);
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
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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

// Projects a world point to screen pixels through the view node's camera
// (off-screen marker -100,-100 when behind the camera).
// FUNCTION: CMR2 0x004bad40
void FUN_004bad40(int *pOut, FixVector *pPoint, BYTE *pView)
{
    FixVector v;
    FixMatrix inverse;

    FixMatrix_Invert(&inverse, (FixMatrix *)(pView + 0xd8));
    FixMatrix_TransformPoint(&v, pPoint, &inverse);
    if (v.z < 0x10000) {
        pOut[0] = -0x640000;
        pOut[1] = -0x640000;
        return;
    }
    FixMatrix_ProjectToScreen(pOut, &v, (int *)((BYTE *)CGraphics::m_pTextureManager + 0x23e4));
    pOut[0] += (int)(__int64)((*(int *)g_pGraphics / 2) * CGraphics::m_65536);
    pOut[1] += (int)(__int64)((*(int *)((BYTE *)g_pGraphics + 4) / 2) * CGraphics::m_65536);
}

// Normalises a vector; components beyond +-100.0 are scaled down by 512
// first so the squares do not overflow.
// FUNCTION: CMR2 0x004bae10
void FixVec_Normalize(FixVector *pOut, FixVector *pIn)
{
    FixVector scaled;
    int len;
    int scaledLen;

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
    scaledLen = FixVecLength(&scaled);
    if (scaledLen == 0) {
        pOut->x = 0;
        pOut->y = 0;
        pOut->z = 0;
        return;
    }
    FixVecScaleRecip(pOut, &scaled, scaledLen);
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
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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

extern BYTE g_unk0x00538d2c[0xcc];
#define g_unk0x00538df0 ((int *)(g_unk0x00538d2c + 0xc4))
extern int g_unk0x00538e04[2];
extern int g_unk0x005391cc[2];
int FUN_0041f3a0(void);
int FUN_00407270(void);
int RallyData_FUN_00411880(void);

// Sets the camera projection for the selected player's view.
// match 66%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00422d40
void FUN_00422d40(unsigned int player)
{
    int i = player & 0xff;
    int base = FixMul(g_unk0x005391cc[i], 0x275c2);
    int ratio = (int)(__int64)(((double)g_pGraphics->resX / (double)g_pGraphics->resY) * CGraphics::m_65536);
    int fovX = FixMul(0x123d7, FixMul(base, ratio));
    int fovY = FixMul(0x2147a, base);

    if (FUN_0041f3a0() == 0 && RallyData_FUN_00411880()) {
        if (CGameInfo::FUN_00405dc0())
            fovY *= 2;
        else
            fovX *= 2;
    }
    if (CGameInfo::FUN_004063f0(2)) {
        CGraphics::SetProjection(-fovX, fovY, g_unk0x00538e04[i], g_unk0x00538df0[i]);
        return;
    }
    if (FUN_00407270() && g_unk0x00538e04[i] < 0x960000) {
        CGraphics::SetProjection(fovX, fovY, 0x960000, g_unk0x00538df0[i]);
        return;
    }
    CGraphics::SetProjection(fovX, fovY, g_unk0x00538e04[i], g_unk0x00538df0[i]);
}

// Layout of one force-feedback slot (0x539200). The full definition lives in
// StageTiming.cpp; keep both copies in sync.
struct Unk0x00539278 {
    int field_0x0;
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int field_0x10;
    int field_0x14;
    int field_0x18;
    int field_0x1c;
    int field_0x20;
    int field_0x24;
    int field_0x28;
    signed char field_0x2c;         // device index, < 0 when none
    BYTE pad_0x2d[3];
    int field_0x30;
    int field_0x34;                 // slot in use
};
extern Unk0x00539278 g_forceFeedbackSlots[2];
extern Unk0x00539278 *g_unk0x00539278;
extern BYTE *g_unk0x0053937c;

// Rebuilds the rotation part of a matrix as an orthonormal basis that keeps
// the current forward direction and the world up axis.
// FUNCTION: CMR2 0x00423070
void FixMatrix_RebuildBasis(FixMatrix *pOut)
{
    FixVector forward;
    FixVector up;
    FixVector right;
    int len;

    up.x = 0;
    up.y = 0x10000;
    up.z = 0;
    FixMatrix_GetForward(&forward, pOut);
    FixVecCross(&right, &up, &forward);
    len = FixVecLength(&right);
    if (len == 0) {
        right.x = 0;
        right.y = 0;
        right.z = 0;
    } else {
        FixVecScaleRecip(&right, &right, len);
    }
    FixVecCross(&forward, &right, &up);
    FixMatrix_SetRight(&right, pOut);
    FixMatrix_SetUp(&up, pOut);
    FixMatrix_SetForward(&forward, pOut);
}

// Updates the player's force-feedback slot from the car's slip: the input
// vector is rotated into car space and its z / length scaled to 16.16.
// FUNCTION: CMR2 0x004241d0
void ForceFeedback_UpdateSlot(BYTE *pCar, FixVector *pIn, int nonzero)
{
    FixVector v;
    int x;

    g_unk0x0053937c = pCar;
    g_unk0x00539278 = &g_forceFeedbackSlots[*(signed char *)(pCar + 0xb1a)];
    FixMatrix_InverseRotateVector(&v, pIn, *(FixMatrix **)(pCar + 0x750));
    if (nonzero != 0) {
        x = v.z;
        if (x < 0)
            x = -x;
        x = FixMul(x - 0x1999, 0x28000);
        if (x < 0)
            x = 0;
        else if (x > 0x10000)
            x = 0x10000;
        if (x > g_unk0x00539278->field_0x24) {
            g_unk0x00539278->field_0x24 = x;
            if (v.z >= 0)
                g_unk0x00539278->field_0x30 = 0;
            else
                g_unk0x00539278->field_0x30 = 1;
        }
    }
    x = (unsigned int)FixVecLength(&v);
    x -= 0x1999;
    x = FixMul(x, 0x28000);
    if (x < 0)
        x = 0;
    else if (x > 0x10000)
        x = 0x10000;
    if (x > g_unk0x00539278->field_0x28)
        g_unk0x00539278->field_0x28 = x;
}

// Per-player camera mode (4 = free camera) and the camera up/forward vectors.
extern BYTE g_unk0x0053cff8[8];
extern FixVector g_unk0x0053d000[6];
extern FixVector g_unk0x0053d048[4];
// Per-player dashboard gauge values.
extern short g_unk0x0053d090[4];
// View offset of a player's camera (HudDash.cpp).
void FUN_00447ee0(FixVector *pOut, BYTE *pSel);

void FUN_00447a40(BYTE *pObj, FixMatrix *pRef);

// Caches the reference matrix's up and right basis vectors in the player's
// camera slots, then builds the view object's matrix from them.
// FUNCTION: CMR2 0x00447a00
void FUN_00447a00(BYTE *pObj, FixMatrix *pRef)
{
    FixMatrix_GetUp(&g_unk0x0053d048[2 + pObj[0]], pRef);
    FixMatrix_GetRight(&g_unk0x0053d000[2 + pObj[0]], pRef);
    FUN_00447a40(pObj, pRef);
}

// Builds a view object's matrix: the basis comes from the player's camera
// up/forward vectors, the position from the player's view offset plus the
// object's own, and then the object's remaining fields are set.
// FUNCTION: CMR2 0x00447a40
void FUN_00447a40(BYTE *pObj, FixMatrix *pRef)
{
    FixMatrix identity;
    FixVector right;
    FixVector base;
    FixVector off;
    FixVector pos;
    FixMatrix *pM;
    BYTE sel;
    int index;
    int v;

    sel = pObj[0];
    FUN_00447ee0(&base, pObj);
    FixMatrix_Identity(&identity);
    FixMatrix_SetPosition(&base, &identity);
    index = sel & 0xff;

    FixVecCross(&right, &g_unk0x0053d048[2 + index], &g_unk0x0053d000[2 + index]);
    pM = (FixMatrix *)(pObj + 8);
    FixMatrix_SetRight(&right, pM);
    FixMatrix_SetUp(&g_unk0x0053d048[2 + index], pM);
    FixMatrix_SetForward(&g_unk0x0053d000[2 + index], pM);
    *(int *)(pObj + 0x38) = 0;
    *(int *)(pObj + 0x3c) = 0;
    *(int *)(pObj + 0x40) = 0;
    FixMatrix_Multiply(pM, &identity, pM);
    FixMatrix_GetPosition(&pos, pM);
    FixMatrix_GetPosition(&off, pRef);
    pos.x += off.x;
    pos.y += off.y;
    pos.z += off.z;
    FixMatrix_SetPosition(&pos, pM);
    if (g_unk0x0053cff8[index] == 4)
        FixMatrix_RotateAboutRight(pM, (unsigned short)g_unk0x0053d090[2 + pObj[1]]);

    *(int *)(pObj + 0x48) = 0;
    *(int *)(pObj + 0x4c) = 0x10000;
    *(int *)(pObj + 0x58) = 0x10000;
    *(int *)(pObj + 0x54) = 0xa000;
    v = base.z;
    if (v < 0)
        v = -v;
    *(int *)(pObj + 0x5c) = v;
    *(int *)(pObj + 0x50) = v - 0x10000;
}

extern BYTE *g_pCarSetup;
int *FUN_00469680(int index);

// Copies the car's body and world matrices into the local transform of its two
// body scene nodes.
// FUNCTION: CMR2 0x0043ecd0
void FUN_0043ecd0(Car *pCar)
{
    FixVector v;

    g_pCurrentCar = pCar;
    g_pCarSetup = (BYTE *)FUN_00469680((int)pCar->field_0xb1a);
    Car_StoreBodyMatrix();
    FixMatrix_GetRight(&v, g_pCurrentCar->pBodyMatrix);
    g_pCurrentCar->pNode0x720->current.right = v;
    FixMatrix_GetUp(&v, g_pCurrentCar->pBodyMatrix);
    g_pCurrentCar->pNode0x720->current.up = v;
    FixMatrix_GetForward(&v, g_pCurrentCar->pBodyMatrix);
    g_pCurrentCar->pNode0x720->current.forward = v;
    FixMatrix_GetPosition(&v, g_pCurrentCar->pBodyMatrix);
    g_pCurrentCar->pNode0x720->current.position = v;
    FixMatrix_GetRight(&v, g_pCurrentCar->pWorld);
    g_pCurrentCar->pNode0x71c->current.right = v;
    FixMatrix_GetUp(&v, g_pCurrentCar->pWorld);
    g_pCurrentCar->pNode0x71c->current.up = v;
    FixMatrix_GetForward(&v, g_pCurrentCar->pWorld);
    g_pCurrentCar->pNode0x71c->current.forward = v;
    FixMatrix_GetPosition(&v, g_pCurrentCar->pWorld);
    g_pCurrentCar->pNode0x71c->current.position = v;
}

void Car_UpdateCorners(Car *pCar);

// Resets a car's physics state from a saved one (used when the body is put
// back on the road): copies the stored fields and rebuilds the matrix-derived
// vectors of the body.
// The body matches the original instruction by instruction, but MSVC6 homes the first parameter in ESI
// instead of the original's EBX, which renames every scratch register (known ceiling, CONOCIMIENTO 4.u).
// match 30%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00426d80
void FUN_00426d80(Car *pDst, Car *pSrc)
{
    int i;

    *(int *)((BYTE *)pDst + 0xb54) = *(int *)((BYTE *)pSrc + 0xd0);
    pDst->field_0x7a4 = 0;
    pDst->heading = *(unsigned short *)((BYTE *)pSrc + 0xc4);
    pDst->field_0x79c = *(int *)((BYTE *)pSrc + 0xb8);
    *(FixVector *)((BYTE *)pDst + 0x414) = pDst->velocity;
    *((BYTE *)pDst + 0xb35) = *((BYTE *)pSrc + 0xcc);
    pDst->field_0xc00 = *(int *)((BYTE *)pSrc + 0xd4);
    pDst->velocity = *(FixVector *)((BYTE *)pSrc + 0x70);
    pDst->angularVelocity = *(FixVector *)((BYTE *)pSrc + 0x7c);
    FixMatrix_CopyRotation((FixMatrix *)pSrc, pDst->pWorld);
    pDst->pWorld->uw = *(int *)((BYTE *)pDst + 0x2ec);

    for (i = 0; i < 4; i++) {
        *(FixVector *)((BYTE *)pDst + 0x30c + i * 0xc) =
            *(FixVector *)((BYTE *)pDst + 0x300 + i * 0xc);
        *(FixVector *)((BYTE *)pDst + 0x300 + i * 0xc) = pDst->corners[i];
        pDst->field_0xabe[i] = pDst->wheelSurface[i];
    }
    for (i = 0; i < 8; i++) {
        pDst->cornerNormal[i] = pDst->cornerAxis[i];
        pDst->field_0xbac[4 + i] = 0;
    }
    *(int *)((BYTE *)pDst + 0x7a8) = pDst->field_0x7a4;
    *(FixVector *)((BYTE *)pDst + 0x2f4) = *(FixVector *)((BYTE *)pDst + 0x2e8);
    *(FixVector *)((BYTE *)pDst + 0x2e8) = pDst->position;
    pDst->normal0x498 = pDst->groundNormal;
    for (i = 0; i < 9; i++)
        ((int *)((BYTE *)pDst + 0x384))[i] = ((int *)((BYTE *)pDst + 0x360))[i];

    FixMatrix_GetPosition((FixVector *)((BYTE *)pDst + 0x2d0), pDst->pWorld);
    FixMatrix_GetRight(&pDst->right, pDst->pWorld);
    FixMatrix_GetUp(&pDst->up, pDst->pWorld);
    FixMatrix_GetForward(&pDst->forward, pDst->pWorld);
    Car_UpdateCorners(pDst);
    pDst->field_0x5c4.x = 0;
    pDst->field_0x5c4.y = 0;
    pDst->field_0x5c4.z = 0;
    pDst->field_0x5d0.x = 0;
    pDst->field_0x5d0.y = 0;
    pDst->field_0x5d0.z = 0;
    *(int *)((BYTE *)pDst + 0xc20) = 1;
}

// One suspension step of the current car: for each wheel, the spring force from
// the corner heights and the damper force are integrated into the suspension
// travel (0x9c8) and its rate (0x9d8).
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042e8e0
void FUN_0042e8e0(void)
{
    FixVector d;
    FixVector mid;
    FixVector v;
    int h;
    int f;
    int w;

    for (w = 0; w < 4; w++) {
        if (g_pCurrentCar->field_0xb74 != 0) {
            h = g_pCurrentCar->cornerHeight[w] - g_pCurrentCar->corners[w].y;
            if (h < 0)
                h = -h;
            if (h < 0x4ccc &&
                (((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].x != 0 ||
                 ((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].y != 0 ||
                 ((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].z != 0)) {
                d.x = (g_pCurrentCar->corners[w].x - ((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].x) -
                      (((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].x - ((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].x);
                d.y = (g_pCurrentCar->corners[w].y - ((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].y) -
                      (((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].y - ((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].y);
                d.z = (g_pCurrentCar->corners[w].z - ((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].z) -
                      (((FixVector *)((BYTE *)g_pCurrentCar + 0x300))[w].z - ((FixVector *)((BYTE *)g_pCurrentCar + 0x330))[w].z);
                FixMatrix_InverseRotateVector(&v, &d, g_pCurrentCar->pWorld);
                f = FixMul(v.z, FixMul(0x9c28, g_physicsScale));
                if (f > 0)
                    ((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w] -= f;
            }
        }
        f = FixMul(g_physicsTimeStep,
                   -(FixMul(((int *)((BYTE *)g_pCurrentCar + 0x9e8))[w], g_pCurrentCar->wheel0x9d8[w]) +
                     FixMul(((int *)((BYTE *)g_pCurrentCar + 0x9f8))[w], ((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w])));
        ((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w] += f;
        ((int *)((BYTE *)g_pCurrentCar + 0x9d8))[w] +=
            FixMul(g_physicsTimeStep, ((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w]);
        if (((int *)((BYTE *)g_pCurrentCar + 0x9d8))[w] < -0x3333) {
            ((int *)((BYTE *)g_pCurrentCar + 0x9d8))[w] = -0x3333;
            if (((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w] < 0)
                ((int *)((BYTE *)g_pCurrentCar + 0x9c8))[w] = 0;
        }
    }
}

// Per-object tables of the stage object payload (see StageObjects.cpp for the
// tables at 0x590d90 and 0x590db0 they index).

// GLOBAL: CMR2 0x00590ec0
BYTE g_unk0x00590ec0[16];
extern int g_carSplitValues[8];
extern int g_unk0x00590db0[64];

// Positions a stage object: its matrix is rebuilt from the object's split
// vector and the reference matrix's position, then its scale fields are set.
// FUNCTION: CMR2 0x004869e0
void FUN_004869e0(BYTE *pObj, FixMatrix *pRef)
{
    FixMatrix identity;
    FixVector src;
    FixVector pos;
    FixVector off;
    FixMatrix *pM;
    unsigned int mode;
    int base;

    if (RallyData_FUN_00411880() != 0 && CGameInfo::FUN_00405dc0() != 0 && FUN_0041f3a0() == 0) {
        if (g_unk0x00590d8c[pObj[0]] == 0)
            mode = 3;
        else if (g_unk0x00590d8c[pObj[0]] == 2)
            mode = 4;
        else
            mode = g_unk0x00590d8c[pObj[0]];
    } else {
        mode = g_unk0x00590d8c[pObj[0]];
    }
    base = g_carSplitValues[g_unk0x00590ec0[pObj[0]]];
    src = ((FixVector *)base)[mode];

    FixMatrix_Identity(&identity);
    FixMatrix_SetPosition(&src, &identity);
    pM = (FixMatrix *)(pObj + 8);
    pM->position.x = 0;
    pM->position.y = 0;
    pM->position.z = 0;
    FixMatrix_Multiply(pM, &identity, pM);
    FixMatrix_GetPosition(&pos, pM);
    FixMatrix_GetPosition(&off, pRef);
    pos.x += off.x;
    pos.y += off.y;
    pos.z += off.z;
    FixMatrix_SetPosition(&pos, pM);

    *(int *)(pObj + 0x48) = g_unk0x00590db0[pObj[0]];
    *(int *)(pObj + 0x4c) = 0x1999;
    *(int *)(pObj + 0x50) = 0;
    *(int *)(pObj + 0x54) = 0xa000;
    *(int *)(pObj + 0x58) = 0;
    *(int *)(pObj + 0x5c) = 0x10000;
}

// Per-octant reference vectors used to probe the ground.
// GLOBAL: CMR2 0x00589458
FixVector g_unk0x00589458[8];

int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri, unsigned short *pSurface,
                          int defaultY);

// Probes the ground under a stage object: casts the ground normal at the given
// point and returns the signed distance from the point to the ground plane.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004702f0
int FUN_004702f0(BYTE *pObj, FixVector *pPoint)
{
    unsigned short surface;
    FixVector neg;
    FixVector rot;
    FixVector v;
    int idx;
    int len;

    *(int *)(pObj + 0x110) = Track_GetGroundHeight(pPoint, (FixVector *)(pObj + 0xd4),
                                                   (short *)(pObj + 0x11c), &surface,
                                                   *(int *)(pObj + 0x110));
    if (*(short *)(pObj + 0x11c) == -1) {
        *(int *)(pObj + 0xd4) = 0;
        *(int *)(pObj + 0xd8) = 0x10000;
        *(int *)(pObj + 0xdc) = 0;
        *(int *)(pObj + 0x110) = pPoint->y - 0xa0000;
    }
    len = FixVecLength((FixVector *)(pObj + 0xd4));
    if (len == 0) {
        *(int *)(pObj + 0xd4) = 0;
        *(int *)(pObj + 0xd8) = 0;
        *(int *)(pObj + 0xdc) = 0;
    } else {
        FixVecScaleRecip((FixVector *)(pObj + 0xd4), (FixVector *)(pObj + 0xd4), len);
    }
    FixVecScale(&neg, (FixVector *)(pObj + 0xd4), -0x10000);
    FixMatrix_InverseRotateVector(&rot, &neg, *(FixMatrix **)(pObj + 0xc8));
    idx = 0;
    if (rot.y >= 0)
        idx = 4;
    if (rot.x < 0)
        idx += 2;
    if (rot.z < 0)
        idx += 1;
    FixMatrix_RotateVector(&v, &g_unk0x00589458[idx], *(FixMatrix **)(pObj + 0xc8));
    v.x += pPoint->x;
    v.y += pPoint->y;
    v.z += pPoint->z;
    v.x = pPoint->x - v.x;
    v.y = *(int *)(pObj + 0x110) - v.y;
    v.z = pPoint->z - v.z;
    return FixDiv(FixVecDot(&v, (FixVector *)(pObj + 0xd4)), *(int *)(pObj + 0xd8));
}

extern double g_unk0x00511300;

// Rebuilds an object's local matrix from a reference matrix: mirrors it about
// the object's split axis and applies the car's tilt rotation.
// match 44%: same structure and calls; MSVC6 picks a different index register (EAX vs ESI)
// and emits setne instead of the original's neg/sbb for the angle mask.
// FUNCTION: CMR2 0x00486910
void FUN_00486910(BYTE *pObj, int *pSrc)
{
    BYTE *pMat = pObj + 8;
    int idx;
    int angle;
    int v;

    memcpy(pMat, pSrc, 0x40);
    idx = *pObj;
    if (g_unk0x00590d8c[idx] == 2) {
        *(int *)pMat = pSrc[8];
        *(int *)(pObj + 0xc) = pSrc[9];
        *(int *)(pObj + 0x10) = pSrc[10];
        *(int *)(pObj + 0x28) = -pSrc[0];
        *(int *)(pObj + 0x2c) = -pSrc[1];
        angle = -pSrc[2];
    } else {
        *(int *)pMat = -pSrc[8];
        *(int *)(pObj + 0xc) = -pSrc[9];
        *(int *)(pObj + 0x10) = -pSrc[10];
        *(int *)(pObj + 0x28) = pSrc[0];
        *(int *)(pObj + 0x2c) = pSrc[1];
        angle = pSrc[2];
    }
    *(int *)(pObj + 0x30) = angle;
    angle = (-(g_unk0x00590d8c[idx] != 0) & 0xfffffff6) + 10;
    v = (int)(__int64)((double)angle * CGraphics::m_65536);
    FixMatrix_RotateAboutRight((FixMatrix *)pMat,
                               (unsigned short)(__int64)((double)v * g_unk0x00511300));
    FUN_004869e0(pObj, (FixMatrix *)pSrc);
}

// Rebuilds an object's local matrix from a reference matrix and interpolates
// it against the previous one (mode 0 keeps the third axis).
// match 65%: same structure and calls; register allocation of the source loads differs
// (setne vs the original's neg/sbb mask for the angle).
// FUNCTION: CMR2 0x00486810
void FUN_00486810(BYTE *pObj, int *pSrc, int param_3)
{
    int m[16];
    int angle;
    int tAxis;
    __int64 v;

    memcpy(m, pSrc, 0x40);
    if (g_unk0x00590d8c[*pObj] == 2) {
        m[0] = pSrc[8];
        m[1] = pSrc[9];
        m[2] = pSrc[10];
        m[8] = -pSrc[0];
        m[9] = -pSrc[1];
        m[10] = -pSrc[2];
    } else {
        m[0] = -pSrc[8];
        m[1] = -pSrc[9];
        m[2] = -pSrc[10];
        m[8] = pSrc[0];
        m[9] = pSrc[1];
        m[10] = pSrc[2];
    }
    angle = g_unk0x00590d8c[*pObj] == 0 ? 10 : 0;
    v = (int)(__int64)((double)angle * CGraphics::m_65536);
    FixMatrix_RotateAboutRight((FixMatrix *)m,
                               (unsigned short)(__int64)((double)(int)v * g_unk0x00511300));
    if (param_3 != 0)
        tAxis = 0x10000;
    else
        tAxis = FixMul(0x4ccc, g_physicsTimeStep);
    FixMatrix_Interpolate((FixMatrix *)(pObj + 8), (FixMatrix *)(pObj + 8), (FixMatrix *)m,
                          0x10000, tAxis, 0x10000, 0);
    FUN_004869e0(pObj, (FixMatrix *)pSrc);
}

// Per-car network pose record: the body matrix plus the axes and position it
// was built from. Eight contiguous 0xec-byte rows start at 0x5393d8; the type
// is owned by StageTiming.cpp (g_unk0x005393d8 is the first row there), so the
// layout below must stay in sync with it.
struct CarNetRecord {
    FixMatrix matrix;       // 0x00
    FixVector right;        // 0x40
    FixVector up;           // 0x4c
    FixVector forward;      // 0x58
    FixVector position;     // 0x64
    BYTE pad_0x70[0x7c];    // 0x70
};
// (defined in StageTiming.cpp, which owns the GLOBAL annotation)
extern CarNetRecord g_unk0x005393d8;

void FUN_0042c870(int index);
void FUN_004263d0(int param_1);

// Refreshes the network pose record of every car of the given order that is
// still in play (field_0xc20): the record matrix takes the body axes and the
// body position, the record velocity (0x70) and angular velocity (0x7c) are
// extrapolated one step from the body state, and the resulting movement is
// turned into the unit direction at 0x94 the remote cars are advanced along.
// Records whose pose did not change are marked (0xdc = 0) instead.
// match 79%: implementada; el bucle y todos los helpers de vectores inline (13 shrd / 15 shld /
// 28 imul / 1 idiv, como el original) generan las mismas instrucciones, pero MSVC6 mantiene la
// base del registro en EDI con otro reparto de temporales.
// FUNCTION: CMR2 0x00426fc0
void FUN_00426fc0(Car *pCars, short *pOrder, short count)
{
    FixVector saved;
    FixVector v;
    FixVector t;
    FixVector mpos;
    FixVector d;
    FixVector scaled;
    Car *pCar;
    CarNetRecord *pRec;
    int dot;
    int len;
    int ax;
    int az;
    int i;

    for (i = count - 1; i >= 0; i--) {
        pCar = pCars + pOrder[i];
        pRec = &g_unk0x005393d8 + pOrder[i];

        if (*(int *)((BYTE *)pCar + 0xc20) == 0)
            continue;

        FixMatrix_SetPosition(&pCar->position, &pRec->matrix);
        FixMatrix_SetRight(&pCar->right, &pRec->matrix);
        FixMatrix_SetUp(&pCar->up, &pRec->matrix);
        FixMatrix_SetForward(&pCar->forward, &pRec->matrix);

        if (*(int *)((BYTE *)pRec + 0xd8) != 0) {
            FUN_0042c870(pCar->field_0xb1a);
            pCar->field_0xbf8 = 1;
            *(int *)((BYTE *)pRec + 0xd8) = 0;
        }

        saved.x = *(int *)((BYTE *)pRec + 0x70);
        saved.y = *(int *)((BYTE *)pRec + 0x74);
        saved.z = *(int *)((BYTE *)pRec + 0x78);
        *(FixVector *)((BYTE *)pRec + 0x70) = pCar->velocity;
        v = pCar->field_0x5c4;
        FixVecScale(&v, &v, 0x3333);
        FixVecScale(&v, &v, g_physicsTimeStep);
        *(int *)((BYTE *)pRec + 0x70) += v.x;
        *(int *)((BYTE *)pRec + 0x74) += v.y;
        *(int *)((BYTE *)pRec + 0x78) += v.z;

        *(FixVector *)((BYTE *)pRec + 0x7c) = pCar->angularVelocity;
        t.x = FixMul(pCar->field_0x5d0.x, -FixMul(pCar->inertia.x, pCar->field_0x75c));
        t.y = FixMul(pCar->field_0x5d0.y, -FixMul(pCar->inertia.y, pCar->field_0x75c));
        t.z = FixMul(pCar->field_0x5d0.z, -FixMul(pCar->inertia.z, pCar->field_0x75c));
        *(int *)((BYTE *)pRec + 0x7c) += t.x;
        *(int *)((BYTE *)pRec + 0x80) += t.y;
        *(int *)((BYTE *)pRec + 0x84) += t.z;

        *(int *)((BYTE *)pRec + 0xd4) = pCar->field_0xc00;
        *(int *)((BYTE *)pCar + 0x960) = *(int *)((BYTE *)pRec + 0xb4);
        FUN_004263d0((int)pRec);

        dot = FixVecDot(&pCar->up, &pCar->groundNormal);
        pCar->field_0x96c -= FixMul(0x1eb8, g_physicsTimeStep);
        if (pCar->field_0x96c < 0)
            pCar->field_0x96c = 0;
        if (pCar->field_0x96c <= 0 && dot > 0xfc28) {
            ax = pCar->angularVelocity.x;
            if (ax < 0)
                ax = -ax;
            if (ax < 0x28f) {
                az = pCar->angularVelocity.z;
                if (az < 0)
                    az = -az;
                if (az < 0x28f) {
                    pCar->field_0xc00 = 0;
                    pCar->field_0x96c = 0;
                    *(int *)((BYTE *)pRec + 0xd4) = 0;
                }
            }
        }

        if (saved.x == *(int *)((BYTE *)pRec + 0x70) &&
            saved.z == *(int *)((BYTE *)pRec + 0x78)) {
            *(int *)((BYTE *)pRec + 0xdc) = 0;
        } else {
            *(int *)((BYTE *)pRec + 0xdc) = 1;
            *(int *)((BYTE *)pRec + 0x94) = *(int *)((BYTE *)pRec + 0x70) - saved.x;
            *(int *)((BYTE *)pRec + 0x98) = *(int *)((BYTE *)pRec + 0x74) - saved.y;
            *(int *)((BYTE *)pRec + 0x9c) = *(int *)((BYTE *)pRec + 0x78) - saved.z;

            len = FixVecLength((FixVector *)((BYTE *)pRec + 0x94));
            if (len == 0) {
                *(int *)((BYTE *)pRec + 0x94) = 0;
                *(int *)((BYTE *)pRec + 0x98) = 0;
                *(int *)((BYTE *)pRec + 0x9c) = 0;
            } else {
                FixVecScaleRecip((FixVector *)((BYTE *)pRec + 0x94),
                                 (FixVector *)((BYTE *)pRec + 0x94), len);
            }

            FixMatrix_GetPosition(&mpos, &pRec->matrix);
            d.x = pRec->position.x - mpos.x;
            d.y = pRec->position.y - mpos.y;
            d.z = pRec->position.z - mpos.z;
            FixVecScale(&scaled, (FixVector *)((BYTE *)pRec + 0x94),
                        FixVecDot(&d, (FixVector *)((BYTE *)pRec + 0x94)));
            d.x -= scaled.x;
            d.y -= scaled.y;
            d.z -= scaled.z;
            pRec->position.x = d.x + mpos.x;
            pRec->position.y = d.y + mpos.y;
            pRec->position.z = d.z + mpos.z;
            FixMatrix_GetPosition((FixVector *)((BYTE *)pRec + 0xa0), &pRec->matrix);
        }
    }
}

// Moving stage objects: the table walked by the per-frame update below, one
// 0x128-byte record per object (defined in StageObjects.cpp, which owns the
// GLOBAL annotations of the table and of the active count).
struct StageObjectEntry0x128 {
    int field_0x0;              // 0x00
    int *pObject;               // 0x04  head of the object's node chain
    BYTE rest[0x120];           // 0x08
};
struct MovingObjects {
    StageObjectEntry0x128 entries[40];
    BYTE meshCount;
    BYTE field_0x2e41[0x103];
    int carDistance[40 * 8];
    BYTE count;
};
extern MovingObjects g_movingObjects;

// 16.16 -> 12-bit angle of a turn rate (its negated twin is 0x511398 in Car.cpp);
// defined in StageTiming.cpp, which owns the GLOBAL annotation.
extern const double g_unk0x00511380;

// The original converts the 16.16 value on the FPU (fild / fmul / fistp) and
// keeps the low word of the truncated result.
#define FIX_ANGLE(v) ((short)(__int64)((double)(v) * g_unk0x00511380))

// The eight ground-probe vectors of an object: its reference axes at 0x104 (x),
// 0x108 (y) and 0x10c (z) with the sign pattern FUN_004702f0 selects with its
// octant index (y >= 0 -> +4, x < 0 -> +2, z < 0 -> +1).
#define FILL_PROBE_TABLE(pObj)                                                          \
    {                                                                                   \
        int probeX = *(int *)((BYTE *)(pObj) + 0x104);                                  \
        int probeY = *(int *)((BYTE *)(pObj) + 0x108);                                  \
        int probeZ = *(int *)((BYTE *)(pObj) + 0x10c);                                  \
                                                                                        \
        g_unk0x00589458[0].x = probeX;                                                  \
        g_unk0x00589458[0].y = -probeY;                                                 \
        g_unk0x00589458[0].z = probeZ;                                                  \
        g_unk0x00589458[1].x = probeX;                                                  \
        g_unk0x00589458[1].y = -probeY;                                                 \
        g_unk0x00589458[1].z = -probeZ;                                                 \
        g_unk0x00589458[2].x = -probeX;                                                 \
        g_unk0x00589458[2].y = -probeY;                                                 \
        g_unk0x00589458[2].z = probeZ;                                                  \
        g_unk0x00589458[3].x = -probeX;                                                 \
        g_unk0x00589458[3].y = -probeY;                                                 \
        g_unk0x00589458[3].z = -probeZ;                                                 \
        g_unk0x00589458[4].x = probeX;                                                  \
        g_unk0x00589458[4].y = probeY;                                                  \
        g_unk0x00589458[4].z = probeZ;                                                  \
        g_unk0x00589458[5].x = probeX;                                                  \
        g_unk0x00589458[5].y = probeY;                                                  \
        g_unk0x00589458[5].z = -probeZ;                                                 \
        g_unk0x00589458[6].x = -probeX;                                                 \
        g_unk0x00589458[6].y = probeY;                                                  \
        g_unk0x00589458[6].z = probeZ;                                                  \
        g_unk0x00589458[7].x = -probeX;                                                 \
        g_unk0x00589458[7].y = probeY;                                                  \
        g_unk0x00589458[7].z = -probeZ;                                                 \
    }

// Per-frame update of the moving stage objects. Each active object of
// g_movingObjects.entries (count in g_movingObjects.count) gets its result matrix refreshed
// from its source matrix and then:
//   mode 1 - the turn rate at 0xe0 is applied to the object basis, the basis is
//            re-orthonormalised and the object is dropped onto the ground;
//   mode 2 - gravity is integrated into the velocity at 0xec and the position,
//            the basis is reflected against the ground and the object matrix is
//            placed at the car-relative offset 0xf8;
// after which the ground-probe octant table is rebuilt and every node of the
// object's chain is flagged as moved. Any other mode only refreshes the matrix.
// match 62.2% (auditado W165): BUG CORREGIDO - el modo 2 leia 'world' sin inicializar; el original
// calcula world = RotateVector(pObj+0xf8) + GetPosition(pMatrix) al principio del bloque (se ve en el
// despacho: 'dec eax; jne' manda el modo 2 a esa secuencia). Al anadirlo la secuencia de llamadas
// coincide exactamente con la del original (27), pero su planificacion baja el % (63.3 -> 62.2): el
// resto de los diffs son reparto de registros/slots y el orden de los bloques.
// inlines (18 shrd / 96 shld / 8 idiv) y las mismas 6 conversiones fild/fmul/fistp.
// FUNCTION: CMR2 0x00470580
void FUN_00470580(void)
{
    FixBasis basis;
    FixVector vecD4;
    FixVector vecE0;
    FixVector world;
    FixVector pos;
    FixVector step;
    FixVector grav;
    FixVector v;
    FixAngles angles;
    FixMatrix *pMatrix;
    BYTE *pObj;
    int *pNode;
    int len;
    int len2;
    int dot;
    int height;
    int i;

    if (g_movingObjects.count == 0)
        return;

    pObj = (BYTE *)g_movingObjects.entries;

    for (i = 0; i < (int)(g_movingObjects.count & 0xff); i++) {
        pMatrix = *(FixMatrix **)(pObj + 0xc8);
        vecD4 = *(FixVector *)(pObj + 0xd4);

        *(FixMatrix *)(pObj + 0x48) = *(FixMatrix *)(pObj + 8);

        switch (*(int *)(pObj + 0xcc)) {
        case 1:
            // Turn rate (scaled first) becomes this step's rotation of the object basis.
            vecE0 = *(FixVector *)(pObj + 0xe0);
            FixVecScale(&vecE0, &vecE0, 0x11999);
            *(FixVector *)(pObj + 0xe0) = vecE0;

            FixMatrix_GetRight(&basis.right, pMatrix);
            FixMatrix_GetUp(&basis.up, pMatrix);
            FixMatrix_GetForward(&basis.forward, pMatrix);

            angles.x = FIX_ANGLE(FixMul(vecE0.x, g_physicsTimeStep));
            angles.y = FIX_ANGLE(FixMul(vecE0.y, g_physicsTimeStep));
            angles.z = FIX_ANGLE(FixMul(vecE0.z, g_physicsTimeStep));
            FixBasis_Rotate(&basis, (unsigned short *)&angles);

            // Rebuilds an orthonormal basis out of up/right unless the object is
            // already upright (up . velocity rate > 0).
            dot = FixVecDot(&vecD4, &basis.up);
            if (dot <= 0) {
                FixVecScale(&pos, &vecD4, dot);
                pos.x = basis.up.x - pos.x;
                pos.y = basis.up.y - pos.y;
                pos.z = basis.up.z - pos.z;
                len = FixVecLength(&pos);
                if (len == 0) {
                    basis.up.x = 0;
                    basis.up.y = 0;
                    basis.up.z = 0;
                } else {
                    FixVecScaleRecip(&basis.up, &pos, len);
                }

                dot = FixVecDot(&basis.up, &basis.right);
                FixVecScale(&pos, &basis.up, dot);
                pos.x = basis.right.x - pos.x;
                pos.y = basis.right.y - pos.y;
                pos.z = basis.right.z - pos.z;
                len = FixVecLength(&pos);
                if (len == 0) {
                    basis.right.x = 0;
                    basis.right.y = 0;
                    basis.right.z = 0;
                } else {
                    FixVecScaleRecip(&basis.right, &pos, len);
                }

                FixVecCross(&pos, &basis.right, &basis.up);
                len = FixVecLength(&pos);
                if (len == 0) {
                    basis.forward.x = 0;
                    basis.forward.y = 0;
                    basis.forward.z = 0;
                } else {
                    FixVecScaleRecip(&basis.forward, &pos, len);
                }
                *(int *)(pObj + 0xcc) = 0;
            }

            FILL_PROBE_TABLE(pObj)
            FixMatrix_RotateVector(&world, (FixVector *)(pObj + 0xf8), pMatrix);
            FixMatrix_GetPosition(&pos, pMatrix);
            world.x += pos.x;
            world.y += pos.y;
            world.z += pos.z;

            pMatrix->position.y += FUN_004702f0(pObj, &world);
            FixMatrix_SetRight(&basis.right, pMatrix);
            FixMatrix_SetUp(&basis.up, pMatrix);
            FixMatrix_SetForward(&basis.forward, pMatrix);

            for (pNode = *(int **)(pObj + 4); pNode != NULL; pNode = *(int **)((BYTE *)pNode + 8))
                *(int *)((BYTE *)pNode + 0x174) = 1;

            if (*(int *)(pObj + 0xcc) == 0)
                *(FixMatrix *)(pObj + 0x48) = *(FixMatrix *)(pObj + 8);
            break;

        case 2:
            FixMatrix_GetRight(&basis.right, pMatrix);
            FixMatrix_GetUp(&basis.up, pMatrix);
            FixMatrix_GetForward(&basis.forward, pMatrix);
            FixMatrix_RotateVector(&world, (FixVector *)(pObj + 0xf8), pMatrix);
            FixMatrix_GetPosition(&pos, pMatrix);
            world.x += pos.x;
            world.y += pos.y;
            world.z += pos.z;

            FILL_PROBE_TABLE(pObj)
            vecE0 = *(FixVector *)(pObj + 0xe0);
            angles.x = FIX_ANGLE(FixMul(vecE0.x, g_physicsTimeStep));
            angles.y = FIX_ANGLE(FixMul(vecE0.y, g_physicsTimeStep));
            angles.z = FIX_ANGLE(FixMul(vecE0.z, g_physicsTimeStep));
            FixBasis_Rotate(&basis, (unsigned short *)&angles);

            // Gravity of the object (zeroed while its "on the ground" flag at
            // 0x120 is set) integrated into the velocity at 0xec, then one step
            // of it is applied to the position.
            grav.x = 0;
            grav.y = 0;
            grav.z = 0;
            if (*(int *)(pObj + 0x120) == 0)
                grav.y = -0xa3d;
            if (CGameInfo::FUN_004063f0(1) != 0)
                FixVecScale(&grav, &grav, 0x8000);
            FixVecScale(&grav, &grav, g_physicsTimeStep);

            ((FixVector *)(pObj + 0xec))->x += grav.x;
            ((FixVector *)(pObj + 0xec))->y += grav.y;
            ((FixVector *)(pObj + 0xec))->z += grav.z;

            FixVecScale(&step, (FixVector *)(pObj + 0xec), g_physicsTimeStep);
            FixVecScale(&grav, &grav, g_physicsTimeStep / 2);
            step.x -= grav.x;
            step.y -= grav.y;
            step.z -= grav.z;
            world.x += step.x;
            world.y += step.y;
            world.z += step.z;

            if (*(int *)(pObj + 0x120) == 0) {
                FixMatrix_SetRight(&basis.right, pMatrix);
                FixMatrix_SetUp(&basis.up, pMatrix);
                FixMatrix_SetForward(&basis.forward, pMatrix);

                if (*(int *)(pObj + 4) < 0) {
                    height = FUN_004702f0(pObj, &world);
                    if (height > -0xccc) {
                        *(int *)(pObj + 0xcc) = 1;
                        world.y += height;
                    }
                }
            } else {
                // Airborne: the spin is damped twice and, while it is slow
                // enough, the basis is rebuilt from the fallen-over up axis.
                FixVecScale(&vecE0, &vecE0, 0xcccc);
                len = FixVecLength(&vecE0);
                FixVecScale(&vecE0, &vecE0, 0xcccc);
                len2 = FixVecLength(&vecE0);

                if (len < 0x28f && len2 < 0x28f) {
                    *(int *)(pObj + 0xe0) = 0;
                    *(int *)(pObj + 0xe4) = 0;
                    *(int *)(pObj + 0xe8) = 0;
                    *(int *)(pObj + 0xec) = 0;
                    *(int *)(pObj + 0xf0) = 0;
                    *(int *)(pObj + 0xf4) = 0;

                    if (FixVecDot(&basis.forward, &vecD4) >= 0) {
                        v = vecD4;
                    } else {
                        FixVecScaleRecip(&v, &vecD4, -0x10000);
                    }

                    pos.x = v.x - basis.forward.x;
                    pos.y = v.y - basis.forward.y;
                    pos.z = v.z - basis.forward.z;
                    len = FixVecLength(&pos);
                    if (len > 0x1999) {
                        FixVecScaleRecip(&pos, &pos, len);
                        FixVecScale(&pos, &pos, 0x1999);
                        pos.x += basis.forward.x;
                        pos.y += basis.forward.y;
                        pos.z += basis.forward.z;
                        len = FixVecLength(&pos);
                        if (len == 0) {
                            basis.forward.x = 0;
                            basis.forward.y = 0;
                            basis.forward.z = 0;
                        } else {
                            FixVecScaleRecip(&basis.forward, &pos, len);
                        }
                    } else {
                        basis.forward = v;
                        *(int *)(pObj + 0xcc) = 0;
                    }

                    dot = FixVecDot(&basis.right, &basis.forward);
                    FixVecScale(&pos, &basis.forward, dot);
                    pos.x = basis.right.x - pos.x;
                    pos.y = basis.right.y - pos.y;
                    pos.z = basis.right.z - pos.z;
                    len = FixVecLength(&pos);
                    if (len == 0) {
                        basis.right.x = 0;
                        basis.right.y = 0;
                        basis.right.z = 0;
                    } else {
                        FixVecScaleRecip(&basis.right, &pos, len);
                    }

                    FixVecCross(&pos, &basis.forward, &basis.right);
                    len = FixVecLength(&pos);
                    if (len == 0) {
                        basis.up.x = 0;
                        basis.up.y = 0;
                        basis.up.z = 0;
                    } else {
                        FixVecScaleRecip(&basis.up, &pos, len);
                    }
                }

                FixMatrix_SetRight(&basis.right, pMatrix);
                FixMatrix_SetUp(&basis.up, pMatrix);
                FixMatrix_SetForward(&basis.forward, pMatrix);
                world.y += FUN_004702f0(pObj, &world);
            }

            FixMatrix_RotateVector(&pos, (FixVector *)(pObj + 0xf8), pMatrix);
            pos.x = world.x - pos.x;
            pos.y = world.y - pos.y;
            pos.z = world.z - pos.z;
            FixMatrix_SetPosition(&pos, pMatrix);

            for (pNode = *(int **)(pObj + 4); pNode != NULL; pNode = *(int **)((BYTE *)pNode + 8))
                *(int *)((BYTE *)pNode + 0x174) = 1;

            if (*(int *)(pObj + 0xcc) == 0)
                *(FixMatrix *)(pObj + 0x48) = *(FixMatrix *)(pObj + 8);
            break;
        }

        pObj += 0x128;
    }
}
