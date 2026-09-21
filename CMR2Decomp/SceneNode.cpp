#include <windows.h>
#include "SceneNode.h"

unsigned short g_sqrtTable[4096];
int g_sinTable[4096];

// Rodrigues rotation scratch values (shared globals in the original)
// GLOBAL: CMR2 0x0067f250
int g_rotSin;
// GLOBAL: CMR2 0x0067f248
int g_rotCos;
// GLOBAL: CMR2 0x0068336c
int g_rotOneMinusCos;
// GLOBAL: CMR2 0x0067f254
int g_rotAxisXX;
// GLOBAL: CMR2 0x0067f258
int g_rotAxisYY;
// GLOBAL: CMR2 0x0067f25c
int g_rotAxisZZ;
// GLOBAL: CMR2 0x0067f238
int g_rotAxisXY;
// GLOBAL: CMR2 0x0067f244
int g_rotAxisXZ;
// GLOBAL: CMR2 0x0067f24c
int g_rotAxisYZ;

// Rotates vectors A and B about the unit axis K by angle (Rodrigues) and
// renormalises them. The original expands this three times over locals, so
// it is a macro rather than a function.
#define ROTATE_ABOUT_AXIS(kx, ky, kz, ax, ay, az, bx, by, bz, angle)                  \
    {                                                                                 \
        int r00, r01, r02, r10, r11, r12, r20, r21, r22;                              \
        FixVector v;                                                                  \
        int len;                                                                      \
                                                                                      \
        g_rotSin = FixSin(-(angle));                                                  \
        g_rotCos = FixCos(angle);                                                     \
        g_rotOneMinusCos = 0x10000 - g_rotCos;                                        \
        g_rotAxisXX = FixMul(kx, kx);                                                 \
        g_rotAxisYY = FixMul(ky, ky);                                                 \
        g_rotAxisZZ = FixMul(kz, kz);                                                 \
        g_rotAxisXY = FixMul(FixMul(kx, ky), g_rotOneMinusCos);                       \
        g_rotAxisXZ = FixMul(FixMul(kx, kz), g_rotOneMinusCos);                       \
        g_rotAxisYZ = FixMul(FixMul(kz, ky), g_rotOneMinusCos);                       \
                                                                                      \
        r00 = FixMul(g_rotCos, 0x10000 - g_rotAxisXX) + g_rotAxisXX;                  \
        r01 = g_rotAxisXY - FixMul(kz, g_rotSin);                                     \
        r02 = FixMul(ky, g_rotSin) + g_rotAxisXZ;                                     \
        r10 = FixMul(kz, g_rotSin) + g_rotAxisXY;                                     \
        r11 = FixMul(g_rotCos, 0x10000 - g_rotAxisYY) + g_rotAxisYY;                  \
        r12 = g_rotAxisYZ - FixMul(kx, g_rotSin);                                     \
        r20 = g_rotAxisXZ - FixMul(ky, g_rotSin);                                     \
        r21 = FixMul(kx, g_rotSin) + g_rotAxisYZ;                                     \
        r22 = FixMul(g_rotCos, 0x10000 - g_rotAxisZZ) + g_rotAxisZZ;                  \
                                                                                      \
        v.x = FixMul(r20, az) + FixMul(r10, ay) + FixMul(r00, ax);                    \
        v.y = FixMul(r21, az) + FixMul(r11, ay) + FixMul(r01, ax);                    \
        v.z = FixMul(r22, az) + FixMul(r12, ay) + FixMul(r02, ax);                    \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            ax = 0;                                                                   \
            ay = 0;                                                                   \
            az = 0;                                                                   \
        } else {                                                                      \
            FixVecScaleRecip((FixVector *)&ax, &v, len);                              \
        }                                                                             \
                                                                                      \
        v.x = FixMul(r20, bz) + FixMul(r10, by) + FixMul(r00, bx);                    \
        v.y = FixMul(r21, bz) + FixMul(r11, by) + FixMul(r01, bx);                    \
        v.z = FixMul(r22, bz) + FixMul(r12, by) + FixMul(r02, bx);                    \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            bx = 0;                                                                   \
            by = 0;                                                                   \
            bz = 0;                                                                   \
        } else {                                                                      \
            FixVecScaleRecip((FixVector *)&bx, &v, len);                              \
        }                                                                             \
    }

// FUNCTION: CMR2 0x004ac820
void SceneNode_Rotate(SceneNode *pNode, FixVector *pTranslation, unsigned short *pAngles)
{
    int rx, ry, rz;
    int ux, uy, uz;
    int fx, fy, fz;
    SceneNode *p;

    ry = pNode->right.y;
    rx = pNode->right.x;
    rz = pNode->right.z;
    uy = pNode->up.y;
    ux = pNode->up.x;
    fy = pNode->forward.y;
    fx = pNode->forward.x;
    uz = pNode->up.z;
    fz = pNode->forward.z;

    if (pAngles[1] != 0)
        ROTATE_ABOUT_AXIS(ux, uy, uz, rx, ry, rz, fx, fy, fz, pAngles[1])
    if (pAngles[2] != 0)
        ROTATE_ABOUT_AXIS(fx, fy, fz, rx, ry, rz, ux, uy, uz, pAngles[2])
    if (pAngles[0] != 0)
        ROTATE_ABOUT_AXIS(rx, ry, rz, ux, uy, uz, fx, fy, fz, pAngles[0])

    pNode->right.y = ry;
    pNode->right.x = rx;
    pNode->right.z = rz;
    pNode->up.y = uy;
    pNode->up.x = ux;
    pNode->up.z = uz;
    pNode->forward.y = fy;
    pNode->forward.x = fx;
    pNode->forward.z = fz;
    pNode->position.x += pTranslation->x;
    pNode->position.y += pTranslation->y;
    pNode->position.z += pTranslation->z;
    for (p = pNode; p != NULL; p = p->pParent)
        p->dirty = 1;
}
