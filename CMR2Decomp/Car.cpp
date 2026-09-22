#include <windows.h>
#include "Graphics.h"
#include "Car.h"
#include "FileBuffer.h"
#include "Game.h"
#include "GameInfo.h"

Car *g_cars[64];
int g_carCount;
Car *g_carBuffer;
Car *g_pCurrentCar;
CarTransforms g_carTransforms[16];
FixMatrix g_carWheelTransforms[16][4];
short g_carOrderCount;
short g_carOrder[48];
int g_carViewScale[15][2];
SceneNode *g_viewNodes[8];

// GLOBAL: CMR2 0x00428790
BYTE g_unk0x00428790[1];

// Allocates one Car per slot and registers the 0x428790 callback.
// FUNCTION: CMR2 0x004287c0
void Car_AllocateTable(int count)
{
    Car *pBuffer;
    int i;

    pBuffer = (Car *)CFileBuffer::AllocateLockedBuffer(count * sizeof(Car));
    g_carBuffer = pBuffer;
    g_carCount = count;
    for (i = 0; i < count; i++) {
        g_cars[i] = pBuffer;
        pBuffer++;
    }
    CGame::RegisterCallback(g_unk0x00428790, NULL);
}

// FUNCTION: CMR2 0x0042b5f0
Car *Car_Get(int index)
{
    return g_cars[index];
}

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

#define FIX_NORMALIZE_INTO(out, v)                                                  \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            out.x = 0;                                                              \
            out.y = 0;                                                              \
            out.z = 0;                                                              \
        } else {                                                                    \
            FixVecScaleRecip(&out, &v, len);                                        \
        }                                                                           \
    }

#define ADD_POSITION(p, pos)    \
    (p)->x += (pos).x;          \
    (p)->y += (pos).y;          \
    (p)->z += (pos).z;          \
    (p)++;

// Recomputes the eight world-space corners of the car's box from its
// half extents and world matrix, then applies the suspension offsets.
// FUNCTION: CMR2 0x0043eef0
void Car_UpdateCorners(Car *pCar)
{
    int hx;
    int hy;
    int hz;
    FixMatrix *pM;
    int ax;
    int ay;
    int az;
    FixVector *p;

    hx = pCar->halfExtents.x;
    pM = pCar->pWorld;
    hy = pCar->halfExtents.y;
    hz = pCar->halfExtents.z;
    g_pCurrentCar = pCar;

    ax = FixMul(pM->right.x, hx);
    ay = FixMul(pM->up.x, hy);
    az = FixMul(pM->forward.x, hz);
    pCar->corners[4].x = az + ay + ax;
    pCar->corners[5].x = (ay - az) + ax;
    pCar->corners[7].x = (ay - az) - ax;
    pCar->corners[6].x = (az - ax) + ay;

    ax = FixMul(pM->right.y, hx);
    ay = FixMul(pM->up.y, hy);
    az = FixMul(pM->forward.y, hz);
    pCar->corners[4].y = az + ay + ax;
    pCar->corners[5].y = (ay - az) + ax;
    pCar->corners[7].y = (ay - az) - ax;
    pCar->corners[6].y = (az - ax) + ay;

    ax = FixMul(pM->right.z, hx);
    ay = FixMul(pM->up.z, hy);
    az = FixMul(pM->forward.z, hz);
    pCar->corners[4].z = az + ay + ax;
    pCar->corners[5].z = (ay - az) + ax;
    pCar->corners[7].z = (ay - az) - ax;
    pCar->corners[6].z = (az - ax) + ay;

    pCar->corners[0].x = -pCar->corners[7].x;
    pCar->corners[1].x = -pCar->corners[6].x;
    pCar->corners[2].x = -pCar->corners[5].x;
    pCar->corners[3].x = -pCar->corners[4].x;
    pCar->corners[0].y = -pCar->corners[7].y;
    pCar->corners[1].y = -pCar->corners[6].y;
    pCar->corners[2].y = -pCar->corners[5].y;
    pCar->corners[3].y = -pCar->corners[4].y;
    pCar->corners[0].z = -pCar->corners[7].z;
    pCar->corners[1].z = -pCar->corners[6].z;
    pCar->corners[2].z = -pCar->corners[5].z;
    pCar->corners[3].z = -pCar->corners[4].z;

    p = pCar->corners;
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)

    if (g_pCurrentCar->field_0xb64 == 0)
        Car_ApplyCornerOffsets();
}

// Shifts the four upper corners of g_pCurrentCar by the scaled body
// offset vectors (suspension travel).
// FUNCTION: CMR2 0x0043f280
void Car_ApplyCornerOffsets(void)
{
    FixVector c;
    FixVector b;
    FixVector a;

    FixVecScale(&a, &g_pCurrentCar->right, g_pCurrentCar->scale0x764);
    FixVecScale(&b, &g_pCurrentCar->forward, g_pCurrentCar->scale0x76c);
    FixVecScale(&c, &g_pCurrentCar->right, g_pCurrentCar->scale0x768);
    g_pCurrentCar->corners[4].x -= a.x;
    g_pCurrentCar->corners[4].y -= a.y;
    g_pCurrentCar->corners[4].z -= a.z;
    g_pCurrentCar->corners[4].x -= b.x;
    g_pCurrentCar->corners[4].y -= b.y;
    g_pCurrentCar->corners[4].z -= b.z;
    g_pCurrentCar->corners[5].x -= a.x;
    g_pCurrentCar->corners[5].y -= a.y;
    g_pCurrentCar->corners[5].z -= a.z;
    g_pCurrentCar->corners[5].x += b.x;
    g_pCurrentCar->corners[5].y += b.y;
    g_pCurrentCar->corners[5].z += b.z;
    g_pCurrentCar->corners[6].x += c.x;
    g_pCurrentCar->corners[6].y += c.y;
    g_pCurrentCar->corners[6].z += c.z;
    g_pCurrentCar->corners[6].x -= b.x;
    g_pCurrentCar->corners[6].y -= b.y;
    g_pCurrentCar->corners[6].z -= b.z;
    g_pCurrentCar->corners[7].x += c.x;
    g_pCurrentCar->corners[7].y += c.y;
    g_pCurrentCar->corners[7].z += c.z;
    g_pCurrentCar->corners[7].x += b.x;
    g_pCurrentCar->corners[7].y += b.y;
    g_pCurrentCar->corners[7].z += b.z;
}

// Scene node stored at byte offset `off + field` inside the car table.
#define CAR_NODE(off, field) (*(SceneNode **)((int)g_carBuffer + (off) + (field)))

// Applies the stored transforms of every car to its scene nodes for the
// given view. Cars closer than 2.0 to the view get the transforms as they
// are; farther cars are pulled 1.5 units towards the view and their axes
// scaled by (1 - 1.5 / distance), which is stored in g_carViewScale.
// FUNCTION: CMR2 0x004292c0
void Car_ApplyViewTransforms(int viewIndex)
{
    FixVector viewPos;
    FixVector delta;
    FixVector bodyPos;
    FixVector axis;
    int pull;
    int n;
    int i;
    short *pOrder;
    int carIndex;
    int wheel;
    int offset;
    int wheelOffset;
    int scale;
    int len;
    CarTransforms *pT;
    FixMatrix *pWheel;

    pull = 0x18000;
    FixMatrix_GetPosition(&viewPos, &g_viewNodes[viewIndex]->current);
    i = g_carOrderCount - 1;
    if (i >= 0) {
        pOrder = &g_carOrder[i];
        n = i + 1;
        do {
            scale = 0x10000;
            carIndex = *pOrder;
            pT = &g_carTransforms[carIndex];
            i = carIndex;
            FixMatrix_GetPosition(&bodyPos, &pT->body);
            wheel = 4;
            offset = carIndex * sizeof(Car);
            pWheel = g_carWheelTransforms[carIndex];
            wheelOffset = offset + 0x738;
            do {
                FixMatrix_CopyRotation(pWheel, &CAR_NODE(wheelOffset, 0)->current);
                wheelOffset += 4;
                pWheel++;
                wheel--;
            } while (wheel != 0);
            delta.x = viewPos.x - bodyPos.x;
            delta.y = viewPos.y - bodyPos.y;
            delta.z = viewPos.z - bodyPos.z;
            len = FixVecLength(&delta);
            if (len > 0x20000) {
                if (len != 0) {
                    scale = 0x10000 - FixDiv(pull, len);
                    FixVecScaleRecip(&delta, &delta, len);
                    FixVecScale(&delta, &delta, pull);
                    bodyPos.x += delta.x;
                    bodyPos.y += delta.y;
                    bodyPos.z += delta.z;
                    FixMatrix_SetPosition(&bodyPos, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetRight(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetUp(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetForward(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_SetPosition(&bodyPos, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetRight(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetUp(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetForward(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &CAR_NODE(offset, 0x720)->current);
                    carIndex = i;
                }
            } else {
                FixMatrix_CopyRotation(&pT->body, &CAR_NODE(offset, 0x71c)->current);
                FixMatrix_CopyRotation(&pT->body2, &CAR_NODE(offset, 0x720)->current);
            }
            pOrder--;
            g_carViewScale[carIndex][viewIndex] = scale;
            n--;
        } while (n != 0);
    }
}

#define SCALE_NODE_AXES(pNode, s)                                            \
    (pNode)->current.right.x = FixMul((pNode)->local.right.x, s);            \
    (pNode)->current.right.y = FixMul((pNode)->local.right.y, s);            \
    (pNode)->current.right.z = FixMul((pNode)->local.right.z, s);            \
    (pNode)->current.up.x = FixMul((pNode)->local.up.x, s);                  \
    (pNode)->current.up.y = FixMul((pNode)->local.up.y, s);                  \
    (pNode)->current.up.z = FixMul((pNode)->local.up.z, s);                  \
    (pNode)->current.forward.x = FixMul((pNode)->local.forward.x, s);        \
    (pNode)->current.forward.y = FixMul((pNode)->local.forward.y, s);        \
    (pNode)->current.forward.z = FixMul((pNode)->local.forward.z, s);        \
    (pNode)->useParentWorld = 0;                                             \
    for (p = (pNode); p != NULL; p = p->pParent)                             \
        p->dirty = 1;

// Places the two view-dependent child nodes of every car (0x748 in front of
// the body towards the view, 0x74c behind it) along the car -> view
// direction, scaled with the distance.
// FUNCTION: CMR2 0x00429810
void Car_UpdateViewNodes(int viewIndex)
{
    int len;
    short *pOrder;
    int nearDist;
    int n;
    int s;
    int i;
    int farDist;
    FixVector local;
    FixVector viewPos;
    FixVector pos;
    FixVector carPos;
    FixVector delta;
    Car *pCar;
    SceneNode *p;

    nearDist = 0x1999;
    farDist = 0x8000;
    FixMatrix_GetPosition(&viewPos, &g_viewNodes[viewIndex]->current);
    i = g_carOrderCount - 1;
    if (i >= 0) {
        pOrder = &g_carOrder[i];
        n = i + 1;
        do {
            pCar = Car_Get(*pOrder);
            if (pCar->pViewNodeNear != NULL || pCar->pViewNodeFar != NULL) {
                FixMatrix_GetPosition(&carPos, &pCar->pNode0x720->current);
                delta.x = carPos.x - viewPos.x;
                delta.y = carPos.y - viewPos.y;
                delta.z = carPos.z - viewPos.z;
                if (FIX_ABS(delta.x) <= 0x800000 && FIX_ABS(delta.y) <= 0x800000 && FIX_ABS(delta.z) <= 0x800000) {
                    len = FixVecLength(&delta);
                    if (len == 0)
                        goto next;
                    FixVecScaleRecip(&delta, &delta, len);
                } else {
                    delta.x /= 128;
                    delta.y /= 128;
                    delta.z /= 128;
                    len = FixVecLength(&delta);
                    FixVecScaleRecip(&delta, &delta, len);
                    len <<= 7;
                }
                if (len != 0) {
                    FixMatrix_InverseRotateVector(&local, &delta, &pCar->pNode0x720->current);
                    if (pCar->pViewNodeNear != NULL) {
                        FixVecScale(&pos, &local, nearDist);
                        FixMatrix_SetPosition(&pos, &(pCar->pViewNodeNear)->current);
                        s = FixDiv(nearDist, len) + 0x10000;
                        SCALE_NODE_AXES(pCar->pViewNodeNear, s)
                    }
                    if (pCar->pViewNodeFar != NULL) {
                        FixVecScale(&pos, &local, -farDist);
                        FixMatrix_SetPosition(&pos, &(pCar->pViewNodeFar)->current);
                        s = 0x10000 - FixDiv(farDist, len);
                        SCALE_NODE_AXES(pCar->pViewNodeFar, s)
                    }
                }
            }
next:
            pOrder--;
            n--;
        } while (n != 0);
    }
}

// Physics time scale (16.16, 1.0 in the shipped data).
int g_physicsTimeStep = 0x10000;

// Updates the body axes of g_pCurrentCar: the up vector is pulled towards
// the wheel plane and the velocity, forward/right are re-orthogonalised,
// then a fraction of the velocity is turned into lateral force and damped.
// FUNCTION: CMR2 0x00436630
void Car_UpdateBodyAxes(void)
{
    int step;
    int damping;
    int maxStep;
    FixVector localUp;
    FixVector world;
    FixVector diff;

    if (g_pCurrentCar->field_0xb28 > 0 && g_pCurrentCar->field_0xb74 != 0) {
        damping = FixMul(0x624, g_physicsTimeStep);
        maxStep = FixMul(0x312, g_physicsTimeStep);
        step = FixMul(g_pCurrentCar->angularVelocity.y, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->angularVelocity.y = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            localUp.x = 0;
            localUp.y = 0;
            localUp.z = 0;
        } else {
            localUp.y = 0;
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.z = -g_sinTable[g_pCurrentCar->heading & 0xfff];
        }
        FixMatrix_RotateVector(&world, &localUp, g_pCurrentCar->pWorld);
        if (FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->right) >= 0)
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->speed, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->speed, 0x2604));

        localUp.x = g_pCurrentCar->right.x + world.x;
        localUp.y = g_pCurrentCar->right.y + world.y;
        localUp.z = g_pCurrentCar->right.z + world.z;
        FIX_NORMALIZE_INTO(localUp, localUp)
        diff.x = localUp.x - g_pCurrentCar->right.x;
        diff.y = localUp.y - g_pCurrentCar->right.y;
        diff.z = localUp.z - g_pCurrentCar->right.z;
        {
            FixVector *pUp;
            step = FixVecLength(&diff);
            if (step < maxStep) {
                pUp = &g_pCurrentCar->right;
                pUp->x = localUp.x;
                pUp->y = localUp.y;
                pUp->z = localUp.z;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                pUp = &g_pCurrentCar->right;
                localUp.x = localUp.x + pUp->x;
                localUp.y = localUp.y + pUp->y;
                localUp.z = localUp.z + pUp->z;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            int d = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, d);
            pForward = &g_pCurrentCar->forward;
            localUp.x = pForward->x - localUp.x;
            localUp.y = pForward->y - localUp.y;
            localUp.z = pForward->z - localUp.z;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            int d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->velocity);
            FixVecScale(&localUp, &g_pCurrentCar->up, d);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(g_pCurrentCar->speed, 0x1e0000), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->cornerForce[0].x += localUp.x;
            g_pCurrentCar->cornerForce[0].y += localUp.y;
            g_pCurrentCar->cornerForce[0].z += localUp.z;
            g_pCurrentCar->cornerForce[1].x += localUp.x;
            g_pCurrentCar->cornerForce[1].y += localUp.y;
            g_pCurrentCar->cornerForce[1].z += localUp.z;
        }

        localUp.x = g_pCurrentCar->velocity.x;
        localUp.y = g_pCurrentCar->velocity.y;
        localUp.y = 0;
        localUp.z = g_pCurrentCar->velocity.z;
        {
            int len = FixVecLength(&localUp);
            if (len > 0) {
                int d;
                FixVecScaleRecip(&localUp, &localUp, len);
                d = FixVecDot(&g_pCurrentCar->normal0x498, &localUp) - FixVecDot(&g_pCurrentCar->groundNormal, &localUp);
                if (d > 0x41) {
                    int s = FixMul(d, g_pCurrentCar->field_0x9c4);
                    if (s > 0x10000)
                        s = 0x10000;
                    s = 0x10000 - s;
                    g_pCurrentCar->velocity.x = FixMul(g_pCurrentCar->velocity.x, s);
                    g_pCurrentCar->velocity.y = FixMul(g_pCurrentCar->velocity.y, s);
                    g_pCurrentCar->velocity.z = FixMul(g_pCurrentCar->velocity.z, s);
                }
            }
        }
    }
}

// Same as Car_UpdateBodyAxes without the final velocity damping.
// FUNCTION: CMR2 0x0043fcc0
void Car_UpdateBodyAxesNoDamping(void)
{
    int step;
    int damping;
    int maxStep;
    FixVector localUp;
    FixVector world;
    FixVector diff;

    if (g_pCurrentCar->field_0xb28 > 0 && g_pCurrentCar->field_0xb74 != 0) {
        damping = FixMul(0x624, g_physicsTimeStep);
        maxStep = FixMul(0x312, g_physicsTimeStep);
        step = FixMul(g_pCurrentCar->angularVelocity.y, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->angularVelocity.y = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            localUp.x = 0;
            localUp.y = 0;
            localUp.z = 0;
        } else {
            localUp.y = 0;
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.z = -g_sinTable[g_pCurrentCar->heading & 0xfff];
        }
        FixMatrix_RotateVector(&world, &localUp, g_pCurrentCar->pWorld);
        if (FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->right) >= 0)
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->speed, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->speed, 0x2604));

        localUp.x = g_pCurrentCar->right.x + world.x;
        localUp.y = g_pCurrentCar->right.y + world.y;
        localUp.z = g_pCurrentCar->right.z + world.z;
        FIX_NORMALIZE_INTO(localUp, localUp)
        diff.x = localUp.x - g_pCurrentCar->right.x;
        diff.y = localUp.y - g_pCurrentCar->right.y;
        diff.z = localUp.z - g_pCurrentCar->right.z;
        {
            FixVector *pUp;
            step = FixVecLength(&diff);
            if (step < maxStep) {
                pUp = &g_pCurrentCar->right;
                pUp->x = localUp.x;
                pUp->y = localUp.y;
                pUp->z = localUp.z;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                pUp = &g_pCurrentCar->right;
                localUp.x = localUp.x + pUp->x;
                localUp.y = localUp.y + pUp->y;
                localUp.z = localUp.z + pUp->z;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            int d = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, d);
            pForward = &g_pCurrentCar->forward;
            localUp.x = pForward->x - localUp.x;
            localUp.y = pForward->y - localUp.y;
            localUp.z = pForward->z - localUp.z;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            int d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->velocity);
            FixVecScale(&localUp, &g_pCurrentCar->up, d);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(g_pCurrentCar->speed, 0x1e0000), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->cornerForce[0].x += localUp.x;
            g_pCurrentCar->cornerForce[0].y += localUp.y;
            g_pCurrentCar->cornerForce[0].z += localUp.z;
            g_pCurrentCar->cornerForce[1].x += localUp.x;
            g_pCurrentCar->cornerForce[1].y += localUp.y;
            g_pCurrentCar->cornerForce[1].z += localUp.z;
        }

    }
}

// Copies the body axes and position of g_pCurrentCar into its body matrix
// and rebuilds it from the wheel plane.
// FUNCTION: CMR2 0x00432c20
void Car_StoreBodyMatrix(void)
{
    g_pCurrentCar->pBodyMatrix->position = g_pCurrentCar->position;
    g_pCurrentCar->pBodyMatrix->right = g_pCurrentCar->right;
    g_pCurrentCar->pBodyMatrix->up = g_pCurrentCar->up;
    g_pCurrentCar->pBodyMatrix->forward = g_pCurrentCar->forward;
    Car_UpdateBodyMatrix();
}

// int field of g_pCurrentCar at byte offset off
#define CAR_INT(off) (*(int *)((int)g_pCurrentCar + (off)))

#define FIX_NORMALIZE_FLIP(v)                                                       \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            v.x = 0;                                                                \
            v.y = 0;                                                                \
            v.z = 0;                                                                \
        } else {                                                                    \
            FixVecScaleRecip(&v, &v, len);                                          \
            if (v.y < 0) {                                                          \
                v.y = -v.y;                                                         \
                v.x = -v.x;                                                         \
                v.z = -v.z;                                                         \
            }                                                                       \
        }                                                                           \
    }

// Tilts the body matrix of g_pCurrentCar to the plane through the wheel
// contact points (suspension compressions), keeping the right vector, and
// lifts its position by the mean compression.
// FUNCTION: CMR2 0x00432cc0
void Car_UpdateBodyMatrix(void)
{
    FixVector a;
    FixVector n1;
    FixVector n2;
    FixVector b;
    FixVector world;
    FixVector forward;
    FixVector up;
    FixVector right;
    int rearAvg;
    int frontAvg;
    int comp[4];
    FixVector p2;
    FixVector p1;
    FixVector p0;
    int i;

    for (i = 0; i < 4; i++) {
        comp[i] = g_pCurrentCar->wheel0x988[i] + g_pCurrentCar->wheel0x9d8[i] + g_pCurrentCar->wheel0x9a8[i];
        if (CGameInfo::FUN_004063f0(6) != 0) {
            if (comp[i] < -0x5999)
                comp[i] = -0x5999;
        } else if (comp[i] < -0x1999) {
            comp[i] = -0x1999;
        }
    }
    rearAvg = FixMul(0x8000, comp[3] + comp[2]);
    frontAvg = FixMul(0x8000, comp[1] + comp[0]);

    p0 = g_pCurrentCar->wheelPos[0];
    p1 = g_pCurrentCar->wheelPos[1];
    p2 = g_pCurrentCar->wheelPos[2];
    a.x = p1.x - p0.x;
    a.y = comp[1] - comp[0];
    a.z = p1.z - p0.z;
    b.x = p2.x - p0.x;
    b.y = rearAvg - comp[0];
    b.z = -p0.z;
    FixVecCross(&n1, &a, &b);
    FIX_NORMALIZE_FLIP(n1)

    p2 = g_pCurrentCar->wheelPos[2];
    p1 = g_pCurrentCar->wheelPos[3];
    p0 = g_pCurrentCar->wheelPos[0];
    a.x = p1.x - p2.x;
    a.y = comp[3] - comp[2];
    a.z = p1.z - p2.z;
    b.x = p0.x - p2.x;
    b.y = frontAvg - comp[2];
    b.z = -p2.z;
    FixVecCross(&n2, &a, &b);
    FIX_NORMALIZE_FLIP(n2)

    a.x = n2.x + n1.x;
    a.y = n2.y + n1.y;
    a.z = n2.z + n1.z;
    FIX_NORMALIZE_INTO(a, a)
    FixMatrix_RotateVector(&world, &a, g_pCurrentCar->pWorld);
    FixMatrix_GetRight(&right, g_pCurrentCar->pBodyMatrix);
    FixMatrix_GetUp(&up, g_pCurrentCar->pBodyMatrix);
    FixMatrix_GetForward(&forward, g_pCurrentCar->pBodyMatrix);
    up.x = world.x;
    up.y = world.y;
    up.z = world.z;
    {
        int d = FixVecDot(&right, &world);
        right.x = right.x - FixMul(world.x, d);
        right.y = right.y - FixMul(world.y, d);
        right.z = right.z - FixMul(world.z, d);
    }
    FIX_NORMALIZE_INTO(right, right)
    FixVecCross(&forward, &right, &world);
    FIX_NORMALIZE_INTO(forward, forward)
    {
        int lift = FixMul(frontAvg + rearAvg, 0x8000);
        a.x = FixMul(g_pCurrentCar->up.x, lift) + g_pCurrentCar->position.x;
        a.y = FixMul(g_pCurrentCar->up.y, lift) + g_pCurrentCar->position.y;
        a.z = FixMul(g_pCurrentCar->up.z, lift) + g_pCurrentCar->position.z;
    }
    FixMatrix_SetRight(&right, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetUp(&up, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetForward(&forward, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetPosition(&a, g_pCurrentCar->pBodyMatrix);
}

// Normalises a body axis of g_pCurrentCar in place through a pointer.
#define CAR_NORMALIZE_AXIS(field)                                                   \
    {                                                                               \
        FixVector *pAxis = &g_pCurrentCar->field;                                   \
        FixVector *pSrc = pAxis;                                                    \
        int len = FixVecLength(pSrc);                                               \
        if (len == 0) {                                                             \
            pAxis->x = 0;                                                           \
            pAxis->y = 0;                                                           \
            pAxis->z = 0;                                                           \
        } else {                                                                    \
            FixVecScaleRecip(pAxis, pSrc, len);                                     \
        }                                                                           \
    }

// Relaxes the body up and forward vectors of g_pCurrentCar towards their
// targets (0x390/0x39c) with a rate that grows with the distance, then
// re-orthogonalises the basis and writes it to the world matrix.
// bFast selects the faster blend rates.
// FUNCTION: CMR2 0x00431220
void Car_RelaxBodyAxes(int bFast)
{
    int rateMin;
    int rateRange;
    int gain;
    FixVector diff;
    FixVector proj;
    int len;
    int t;
    int d;

    if (bFast != 0) {
        rateMin = 0x8000;
        rateRange = 0x8000;
    } else {
        rateMin = 0x1999;
        rateRange = 0x4ccc;
    }
    gain = FixMul(0xc0000, g_physicsTimeStep);

    if (FIX_ABS(g_pCurrentCar->field_0x920) > 0xfae1) {
        diff.x = g_pCurrentCar->up.x - g_pCurrentCar->targetUp.x;
        diff.y = g_pCurrentCar->up.y - g_pCurrentCar->targetUp.y;
        diff.z = g_pCurrentCar->up.z - g_pCurrentCar->targetUp.z;
        len = FixVecLength(&diff);
        if (len > 0) {
            t = FixMul(len, gain);
            if (t > 0x10000)
                t = 0x10000;
            FixVecScale(&diff, &diff, FixMul(t, rateRange) + rateMin);
            g_pCurrentCar->up.x = g_pCurrentCar->targetUp.x + diff.x;
            g_pCurrentCar->up.y = g_pCurrentCar->targetUp.y + diff.y;
            g_pCurrentCar->up.z = g_pCurrentCar->targetUp.z + diff.z;
            CAR_NORMALIZE_AXIS(up)
        }
    }

    if (FIX_ABS(g_pCurrentCar->field_0x924) > 0x3333 && FIX_ABS(g_pCurrentCar->field_0x91c) < 0x1999) {
        diff.x = g_pCurrentCar->forward.x - g_pCurrentCar->targetForward.x;
        diff.y = g_pCurrentCar->forward.y - g_pCurrentCar->targetForward.y;
        diff.z = g_pCurrentCar->forward.z - g_pCurrentCar->targetForward.z;
        d = FixVecDot(&diff, &g_pCurrentCar->up);
        FixVecScale(&proj, &g_pCurrentCar->up, d);
        diff.x = diff.x - proj.x;
        diff.y = diff.y - proj.y;
        diff.z = diff.z - proj.z;
        len = FixVecLength(&diff);
        if (len > 0) {
            t = FixMul(len, gain);
            if (t > 0x10000)
                t = 0x10000;
            FixVecScale(&diff, &diff, (FixMul(t, rateRange) - 0x10000) + rateMin);
            g_pCurrentCar->forward.x = g_pCurrentCar->forward.x + diff.x;
            g_pCurrentCar->forward.y = g_pCurrentCar->forward.y + diff.y;
            g_pCurrentCar->forward.z = g_pCurrentCar->forward.z + diff.z;
            CAR_NORMALIZE_AXIS(forward)
        }
    }

    d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->forward);
    FixVecScale(&diff, &g_pCurrentCar->up, d);
    g_pCurrentCar->forward.x = g_pCurrentCar->forward.x - diff.x;
    g_pCurrentCar->forward.y = g_pCurrentCar->forward.y - diff.y;
    g_pCurrentCar->forward.z = g_pCurrentCar->forward.z - diff.z;
    CAR_NORMALIZE_AXIS(forward)
    FixVecCross(&g_pCurrentCar->right, &g_pCurrentCar->up, &g_pCurrentCar->forward);
    CAR_NORMALIZE_AXIS(right)
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
}

// Velocity of each box corner of g_pCurrentCar: v + w x r, with the
// angular velocity in body space and the corners as +-half extents.
// FUNCTION: CMR2 0x00434f50
void Car_UpdateCornerVelocities(void)
{
    FixVector *pCv;
    FixMatrix *pM;
    FixVector *pVel;
    FixVector *pW;
    int wyhz;
    int wzhy;
    int wzhx;
    int wxhz;
    int wxhy;
    int wyhx;
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int c0;
    int c1;
    int c2;
    int c3;

    pM = g_pCurrentCar->pWorld;
    pW = &g_pCurrentCar->angularVelocity;
    pVel = &g_pCurrentCar->velocity;
    pCv = g_pCurrentCar->cornerVelocity;
    wyhz = FixMul(pW->y, g_pCurrentCar->halfExtents.z);
    wzhy = FixMul(pW->z, g_pCurrentCar->halfExtents.y);
    wzhx = FixMul(pW->z, g_pCurrentCar->halfExtents.x);
    wxhz = FixMul(pW->x, g_pCurrentCar->halfExtents.z);
    wxhy = FixMul(pW->x, g_pCurrentCar->halfExtents.y);
    wyhx = FixMul(pW->y, g_pCurrentCar->halfExtents.x);

    a = FixMul(pM->right.x, wyhz);
    b = FixMul(pM->right.x, wzhy);
    c = FixMul(pM->up.x, wzhx);
    d = FixMul(pM->up.x, wxhz);
    e = FixMul(pM->forward.x, wxhy);
    f = FixMul(pM->forward.x, wyhx);
    pCv[0].x = (((c - f) - e) - d) + b + a;
    pCv[1].x = (((d - f) - e) - a) + c + b;
    pCv[2].x = (((f - e) - d) - c) + b + a;
    pCv[3].x = (((f - e) - c) - a) + d + b;

    a = FixMul(pM->right.y, wyhz);
    b = FixMul(pM->right.y, wzhy);
    c = FixMul(pM->up.y, wzhx);
    d = FixMul(pM->up.y, wxhz);
    e = FixMul(pM->forward.y, wxhy);
    f = FixMul(pM->forward.y, wyhx);
    pCv[0].y = (((c - f) - e) - d) + b + a;
    pCv[1].y = (((d - f) - e) - a) + c + b;
    pCv[2].y = (((f - e) - d) - c) + b + a;
    pCv[3].y = (((f - e) - c) - a) + d + b;

    a = FixMul(pM->right.z, wyhz);
    b = FixMul(pM->right.z, wzhy);
    c = FixMul(pM->up.z, wzhx);
    d = FixMul(pM->up.z, wxhz);
    e = FixMul(pM->forward.z, wxhy);
    f = FixMul(pM->forward.z, wyhx);
    c0 = (((c - f) - e) - d) + b + a;
    c1 = (((d - f) - e) - a) + c + b;
    pCv[1].z = c1;
    pCv[0].z = c0;
    c2 = (((f - e) - d) - c) + b + a;
    c3 = (((f - e) - c) - a) + d + b;
    pCv[3].z = c3;
    pCv[2].z = c2;

    pCv[4].x = -pCv[3].x;
    pCv[4].y = -pCv[3].y;
    pCv[4].z = -c3;
    pCv[5].x = -pCv[2].x;
    pCv[5].y = -pCv[2].y;
    pCv[5].z = -c2;
    pCv[6].x = -pCv[1].x;
    pCv[6].y = -pCv[1].y;
    pCv[6].z = -c1;
    pCv[7].x = -pCv[0].x;
    pCv[7].y = -pCv[0].y;
    pCv[7].z = -c0;

    pCv[0].x = pCv[0].x + pVel->x;
    pCv[0].y = pCv[0].y + pVel->y;
    pCv[0].z = pCv[0].z + pVel->z;
    pCv[1].x = pCv[1].x + pVel->x;
    pCv[1].y = pCv[1].y + pVel->y;
    pCv[1].z = pCv[1].z + pVel->z;
    pCv[2].x = pCv[2].x + pVel->x;
    pCv[2].y = pCv[2].y + pVel->y;
    pCv[2].z = pCv[2].z + pVel->z;
    pCv[3].x = pCv[3].x + pVel->x;
    pCv[3].y = pCv[3].y + pVel->y;
    pCv[3].z = pCv[3].z + pVel->z;
    pCv[4].x = pCv[4].x + pVel->x;
    pCv[4].y = pCv[4].y + pVel->y;
    pCv[4].z = pCv[4].z + pVel->z;
    pCv[5].x = pCv[5].x + pVel->x;
    pCv[5].y = pCv[5].y + pVel->y;
    pCv[5].z = pCv[5].z + pVel->z;
    pCv[6].x = pCv[6].x + pVel->x;
    pCv[6].y = pCv[6].y + pVel->y;
    pCv[6].z = pCv[6].z + pVel->z;
    pCv[7].x = pCv[7].x + pVel->x;
    pCv[7].y = pCv[7].y + pVel->y;
    pCv[7].z = pCv[7].z + pVel->z;

}

// Corner i of the box with its y replaced by the ground height under it.
#define GROUND_CORNER(v, i)                                                         \
    v.x = g_pCurrentCar->corners[i].x;                                              \
    v.y = g_pCurrentCar->cornerHeight[i];                                           \
    v.z = g_pCurrentCar->corners[i].z;

// Picks the box face closest to horizontal and rebuilds the ground normal
// from the two triangles formed by its four corners projected onto the ground.
// FUNCTION: CMR2 0x0042de20
void Car_UpdateGroundNormal(void)
{
    int absRight = FIX_ABS(g_pCurrentCar->right.y);
    int absUp = FIX_ABS(g_pCurrentCar->up.y);
    int absForward = FIX_ABS(g_pCurrentCar->forward.y);
    FixVector a;
    FixVector b;
    FixVector c;
    FixVector d;
    FixVector ab;
    FixVector cb;
    FixVector cd;
    FixVector ad;
    FixVector n;

    if (absUp >= absRight && absUp >= absForward) {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[1].x;
        c.x = g_pCurrentCar->corners[2].x;
        d.x = g_pCurrentCar->corners[3].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[1];
        c.y = g_pCurrentCar->cornerHeight[2];
        d.y = g_pCurrentCar->cornerHeight[3];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[1].z;
        c.z = g_pCurrentCar->corners[2].z;
        d.z = g_pCurrentCar->corners[3].z;
    } else if (absRight >= absUp && absRight >= absForward) {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[1].x;
        c.x = g_pCurrentCar->corners[4].x;
        d.x = g_pCurrentCar->corners[5].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[1];
        c.y = g_pCurrentCar->cornerHeight[4];
        d.y = g_pCurrentCar->cornerHeight[5];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[1].z;
        c.z = g_pCurrentCar->corners[4].z;
        d.z = g_pCurrentCar->corners[5].z;
    } else {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[2].x;
        c.x = g_pCurrentCar->corners[4].x;
        d.x = g_pCurrentCar->corners[6].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[2];
        c.y = g_pCurrentCar->cornerHeight[4];
        d.y = g_pCurrentCar->cornerHeight[6];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[2].z;
        c.z = g_pCurrentCar->corners[4].z;
        d.z = g_pCurrentCar->corners[6].z;
    }

    ab.x = a.x - b.x;
    ab.y = a.y - b.y;
    ab.z = a.z - b.z;
    cb.x = c.x - b.x;
    cb.y = c.y - b.y;
    cb.z = c.z - b.z;
    FixVecCross(&n, &cb, &ab);
    if (n.y < 0) {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    {
        FixVector *pOut = &g_pCurrentCar->groundNormal;
        int len = FixVecLength(&n);
        if (len == 0) {
            pOut->x = 0;
            pOut->y = 0;
            pOut->z = 0;
        } else {
            FixVecScaleRecip(pOut, &n, len);
        }
    }

    ad.x = a.x - d.x;
    ad.y = a.y - d.y;
    ad.z = a.z - d.z;
    cd.x = c.x - d.x;
    cd.y = c.y - d.y;
    cd.z = c.z - d.z;
    FixVecCross(&n, &cd, &ad);
    if (n.y < 0) {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    FIX_NORMALIZE_INTO(n, n);

    {
        FixVector *pOut = &g_pCurrentCar->groundNormal;
        n.x += pOut->x;
        n.y += pOut->y;
        n.z += pOut->z;
        {
            int len = FixVecLength(&n);
            if (len == 0) {
                pOut->x = 0;
                pOut->y = 0;
                pOut->z = 0;
            } else {
                FixVecScaleRecip(pOut, &n, len);
            }
        }
    }
}

FixVector g_leanDamping;
FixVector g_leanAccel;
FixVector g_leanDelta;
FixVector g_carAccel;
FixBasis g_leanBasis;

int g_physicsScale = 0x10000;

// Integrates the body lean (the chassis pitching/rolling against its own
// acceleration) and rebuilds the body matrix from the two lean angles.
// FUNCTION: CMR2 0x00443250
void Car_UpdateBodyLean(void)
{
    int len;
    int maxStep;
    int angleZ;
    int angleX;
    int sinZ;
    int cosZ;
    int sinX;
    int cosX;
    int z;

    g_carAccel.x = g_pCurrentCar->velocityNext.x - g_pCurrentCar->velocity.x;
    g_carAccel.y = g_pCurrentCar->velocityNext.y - g_pCurrentCar->velocity.y;
    g_carAccel.z = g_pCurrentCar->velocityNext.z - g_pCurrentCar->velocity.z;
    FixMatrix_InverseRotateVector(&g_leanAccel, &g_carAccel, g_pCurrentCar->pWorld);
    if (FixVecLength(&g_leanAccel) == 0 || g_pCurrentCar->field_0xb60 != 0) {
        g_leanAccel.x = 0;
        g_leanAccel.y = 0;
        g_leanAccel.z = 0;
    }
    FixVecScale(&g_leanAccel, &g_leanAccel, g_physicsScale);
    g_leanAccel.z = g_leanAccel.z * 2;

    len = FixVecLength(&g_pCurrentCar->lean);
    if (len > 0) {
        int damping = -FixMul(len, g_pCurrentCar->field_0x9bc);
        FixVecScaleRecip(&g_leanDamping, &g_pCurrentCar->lean, len);
        FixVecScale(&g_leanDamping, &g_leanDamping, damping);
    } else {
        g_leanDamping.x = 0;
        g_leanDamping.y = 0;
        g_leanDamping.z = 0;
    }

    g_leanDelta.x = g_leanDamping.x + g_leanAccel.x;
    g_leanDelta.y = g_leanDamping.y + g_leanAccel.y;
    g_leanDelta.z = g_leanDamping.z + g_leanAccel.z;
    FixVecScale(&g_leanDelta, &g_leanDelta, FixMul(g_physicsTimeStep, 0x13333));
    g_leanDelta.y = 0;
    len = FixVecLength(&g_leanDelta);
    maxStep = FixMul(0x312, g_physicsTimeStep);
    if (len > maxStep) {
        FixVecScaleRecip(&g_leanDelta, &g_leanDelta, len);
        FixVecScale(&g_leanDelta, &g_leanDelta, FixMul(0x312, g_physicsTimeStep));
    }

    g_pCurrentCar->lean.x += g_leanDelta.x;
    g_pCurrentCar->lean.y += g_leanDelta.y;
    g_pCurrentCar->lean.z += g_leanDelta.z;
    {
        FixVector *pLean = &g_pCurrentCar->lean;
        g_leanAccel = *pLean;
    }

    if (FIX_ABS(g_leanAccel.x) > 0x170a) {
        g_leanAccel.x = g_leanAccel.x > 0 ? 0x170a : -0x170a;
    }
    z = g_leanAccel.z;
    if (FIX_ABS(z) > 0x170a) {
        if (z > 0) {
            z = 0x170a;
        } else {
            z = -0x170a;
        }
        g_leanAccel.z = z;
    }
    if (z < 0) {
        z = -z;
    }

    angleZ = FixAtan2(z, FixMul(0x10000, 0x10000));
    if (g_leanAccel.z < 0) {
        angleZ = -angleZ;
    }
    cosZ = g_sinTable[(angleZ + 0x400) & 0xfff];
    sinZ = g_sinTable[angleZ & 0xfff];

    angleX = FixAtan2(FIX_ABS(g_leanAccel.x), FixMul(0x10000, 0x10000));
    if (g_leanAccel.x < 0) {
        angleX = -angleX;
    }
    sinX = g_sinTable[angleX & 0xfff];
    cosX = g_sinTable[(angleX + 0x400) & 0xfff];

    g_leanBasis.right.z = 0;
    g_leanBasis.right.x = cosX;
    g_leanBasis.right.y = -sinX;
    g_leanBasis.up.x = FixMul(cosZ, sinX);
    g_leanBasis.up.y = FixMul(cosZ, cosX);
    g_leanBasis.up.z = sinZ;
    g_leanBasis.forward.x = FixMul(-sinZ, sinX);
    g_leanBasis.forward.y = FixMul(-sinZ, cosX);
    g_leanBasis.forward.z = cosZ;

    g_pCurrentCar->pBodyMatrix->right = g_leanBasis.right;
    g_pCurrentCar->pBodyMatrix->up = g_leanBasis.up;
    g_pCurrentCar->pBodyMatrix->forward = g_leanBasis.forward;
}

// Evens out the two wheels of an axle once they drift too far apart.
// FUNCTION: CMR2 0x0043af70
void Car_BalanceWheelPairs(void)
{
    int diff = g_pCurrentCar->wheelLoad[0] - g_pCurrentCar->wheelLoad[1];
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 0x1999) {
        int sum = g_pCurrentCar->wheelLoad[1] + g_pCurrentCar->wheelLoad[0];
        int avg = FixMul(sum, 0x8000);
        g_pCurrentCar->wheelLoad[1] = avg;
        g_pCurrentCar->wheelLoad[0] = avg;
    }
    diff = g_pCurrentCar->wheelLoad[2] - g_pCurrentCar->wheelLoad[3];
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 0x1999) {
        int sum = g_pCurrentCar->wheelLoad[3] + g_pCurrentCar->wheelLoad[2];
        int avg = FixMul(sum, 0x8000);
        g_pCurrentCar->wheelLoad[3] = avg;
        g_pCurrentCar->wheelLoad[2] = avg;
    }
}

// arccos as a 12-bit angle: 4096 entries for a dot product in [-1, 1]
// GLOBAL: CMR2 0x006e6ef4
short g_acosTable[4096];

inline short FixAcos(int x)
{
    int neg = 0;
    double t;

    if (x < 0) {
        x = -x;
        neg = 1;
    }
    if (x > 0x10000) {
        return g_acosTable[4095];
    }
    t = (double)x * CGraphics::m_oneOver65536 * -4095.0;
    if (neg) {
        return -g_acosTable[-(__int64)t];
    }
    return g_acosTable[-(__int64)t];
}

// Builds the sideways (friction) force at every corner that is sliding against
// its contact normal, sharing the given grip between the sliding corners.
// FUNCTION: CMR2 0x0043a920
void Car_ApplyCornerFriction(int grip)
{
    int sliding[8];
    int strength[8];
    FixVector dir;
    int count = 0;
    int cornerCount;
    int i;

    if (g_pCurrentCar->field_0xc00 == 0) {
        cornerCount = 4;
    } else {
        cornerCount = 8;
        if (g_pCurrentCar->field_0xb34 > 0) {
            grip = FixDiv(g_pCurrentCar->field_0x75c, 0x80000);
        }
    }

    i = 0;
    if (cornerCount > 0) {
        do {
            sliding[i] = 0;
            if ((g_pCurrentCar->field_0xc00 == 0 && g_pCurrentCar->field_0xbac[i] != 0) ||
                (g_pCurrentCar->field_0xc00 != 0 && g_pCurrentCar->cornerFlags[i] == 0)) {
                int d = FixVecDot(&g_pCurrentCar->cornerAxis[i], &g_pCurrentCar->cornerNormal[i]);
                if (d >= 0x10000) {
                    strength[i] = 0;
                } else {
                    strength[i] = (0x400 - FixAcos(d)) * 0x1680;
                }
                if (strength[i] > 0xa0000 &&
                    g_pCurrentCar->cornerAxis[i].y < g_pCurrentCar->cornerNormal[i].y) {
                    int along = FixVecDot(&g_pCurrentCar->cornerNormal[i], &g_pCurrentCar->cornerVelocity[i]);
                    FixVecScale(&dir, &g_pCurrentCar->cornerNormal[i], along);
                    dir.x = g_pCurrentCar->cornerVelocity[i].x - dir.x;
                    dir.y = g_pCurrentCar->cornerVelocity[i].y - dir.y;
                    dir.z = g_pCurrentCar->cornerVelocity[i].z - dir.z;
                    if (FixVecDot(&dir, &g_pCurrentCar->cornerAxis[i]) < 0) {
                        sliding[i] = 1;
                        count++;
                    }
                }
            }
            i++;
        } while (i < cornerCount);

        if (count > 0) {
            int share = FixDiv(0x140000, count << 16);
            i = 0;
            do {
                if (sliding[i] != 0) {
                    int along;
                    int len;
                    int s;

                    along = FixVecDot(&g_pCurrentCar->cornerAxis[i], &g_pCurrentCar->cornerNormal[i]);
                    FixVecScale(&dir, &g_pCurrentCar->cornerNormal[i], along);
                    dir.x = g_pCurrentCar->cornerAxis[i].x - dir.x;
                    dir.y = g_pCurrentCar->cornerAxis[i].y - dir.y;
                    dir.z = g_pCurrentCar->cornerAxis[i].z - dir.z;
                    len = FixVecLength(&dir);
                    if (len == 0) {
                        dir.x = 0;
                        dir.y = 0;
                        dir.z = 0;
                    } else {
                        FixVecScaleRecip(&dir, &dir, len);
                    }
                    along = FixVecDot(&dir, &g_pCurrentCar->cornerVelocity[i]);
                    if (FIX_ABS(along) > 0x10000) {
                        if (along < 0) {
                            dir.x = -dir.x;
                            dir.y = -dir.y;
                            dir.z = -dir.z;
                        }
                    } else {
                        FixVecScale(&dir, &dir, along);
                    }
                    strength[i] = strength[i] - 0xa0000;
                    s = FixMul(strength[i], 0x51e);
                    if (s > 0x10000) {
                        s = 0x10000;
                    }
                    s = -FixMul(s, FixMul(grip, share));
                    FixVecScale(&dir, &dir, s);
                    g_pCurrentCar->cornerForce[i].x += dir.x;
                    g_pCurrentCar->cornerForce[i].y += dir.y;
                    g_pCurrentCar->cornerForce[i].z += dir.z;
                }
                i++;
            } while (i < cornerCount);
        }
    }
}

int FUN_00469bc0(void *pCar, int index);

// Normalises the per-wheel slip, turns it into wheel torque and, on the cars
// that use it, feeds the torque back towards half the drive torque.
// FUNCTION: CMR2 0x0043c640
void Car_UpdateWheelTorques(void)
{
    int quarter = g_pCurrentCar->field_0x75c / 4;
    int maxSlip = 0;
    int *pSlip = g_pCurrentCar->field_0x998;
    int i = 4;

    do {
        int v = *pSlip;
        if (FIX_ABS(v) > maxSlip) {
            maxSlip = FIX_ABS(v);
        }
        pSlip++;
        i--;
    } while (i != 0);

    if (maxSlip > 0x4000) {
        int scale = FixDiv(0x4000, maxSlip);
        for (i = 0; i < 4; i++) {
            g_pCurrentCar->field_0x998[i] = FixMul(g_pCurrentCar->field_0x998[i], scale);
        }
    }

    for (i = 0; i < 4; i++) {
        int load = g_pCurrentCar->field_0x808[i] + FixMul(g_pCurrentCar->field_0x998[i], 0x40000);
        g_pCurrentCar->field_0x8b8[i] = quarter - FixMul(g_pCurrentCar->field_0x8d8, load);
    }

    if (FUN_00469bc0(g_pCurrentCar, 3) != 0) {
        int rate = FixMul(g_pCurrentCar->speed, 0x2000);
        int half;

        if (rate > 0x10000) {
            rate = 0x10000;
        }
        half = g_pCurrentCar->field_0x75c / 2;
        for (i = 0; i < 4; i++) {
            int target = i < 2 ? half : 0;
            g_pCurrentCar->field_0x8b8[i] += FixMul(rate, target - g_pCurrentCar->field_0x8b8[i]);
        }
    }
}

// GLOBAL: CMR2 0x0053cc1c
BYTE *g_pCarSetup;
// GLOBAL: CMR2 0x00519c88
int g_dragBase = 0x10000;

// One physics step of the rigid body: sums the corner forces into a force and
// a torque, integrates linear and angular velocity, moves the body and rebuilds
// its world matrix.
// FUNCTION: CMR2 0x00440840
void Car_Integrate(void)
{
    FixVector force;
    FixVector tmp;
    FixVector angStep;
    FixVector torque;
    FixVector posStep;
    FixVector cross;
    FixVector r;
    FixVector accel;
    int locked = 0;
    int off;
    int halfStep;
    int drag;

    if (g_pCurrentCar->angularVelocity.x == 0 && g_pCurrentCar->angularVelocity.z == 0) {
        locked = 1;
    }
    force = g_pCurrentCar->baseForce;
    torque.x = 0;
    torque.y = 0;
    torque.z = 0;

    if (locked != 0) {
        off = 0x66c;
        do {
            FixVector *pForce = (FixVector *)((BYTE *)g_pCurrentCar + off);
            tmp = *pForce;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            torque.y = torque.y +
                       FixMul(FixVecDot(pForce, &g_pCurrentCar->forward),
                              *(int *)((BYTE *)g_pCurrentCar + off - 0x438)) * 2;
            off -= 0x18;
        } while (off >= 0x648);
    } else {
        off = 0x24;
        do {
            FixVector *pForce = (FixVector *)((BYTE *)g_pCurrentCar + off + 0x648);
            tmp = *pForce;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            r.x = *(int *)((BYTE *)g_pCurrentCar + off + 0x270) - g_pCurrentCar->position.x;
            r.y = *(int *)((BYTE *)g_pCurrentCar + off + 0x274) - g_pCurrentCar->position.y;
            r.z = *(int *)((BYTE *)g_pCurrentCar + off + 0x278) - g_pCurrentCar->position.z;
            FixVecCross(&cross, &tmp, &r);
            torque.x = torque.x + cross.x;
            torque.y = torque.y + cross.y;
            torque.z = torque.z + cross.z;
            off -= 0xc;
        } while (off >= 0);
        FixMatrix_InverseRotateVector(&tmp, &torque, g_pCurrentCar->pWorld);
        torque = tmp;
    }

    torque.y = FixMul(torque.y, 0xb333);
    FixVecScale(&force, &force, 0xd999);
    drag = FixMul(FIX_ABS(g_pCurrentCar->speed), *(int *)(g_pCarSetup + 0x408) + g_dragBase);
    FixVecScale(&tmp, &g_pCurrentCar->velocity, drag);
    force.x = force.x - tmp.x;
    force.y = force.y - tmp.y;
    force.z = force.z - tmp.z;

    FixVecScale(&accel, &force, g_pCurrentCar->field_0x760);
    accel.x = accel.x + g_pCurrentCar->field_0x5c4.x;
    accel.y = accel.y + g_pCurrentCar->field_0x5c4.y;
    accel.z = accel.z + g_pCurrentCar->field_0x5c4.z;
    FixVecScale(&accel, &accel, 0x3333);
    g_pCurrentCar->velocityNext = g_pCurrentCar->velocity;
    FixVecScale(&accel, &accel, g_physicsTimeStep);
    g_pCurrentCar->velocity.x = g_pCurrentCar->velocity.x + accel.x;
    g_pCurrentCar->velocity.y = g_pCurrentCar->velocity.y + accel.y;
    g_pCurrentCar->velocity.z = g_pCurrentCar->velocity.z + accel.z;
    g_pCurrentCar->speed = FixVecLength(&g_pCurrentCar->velocity);

    FixVecScale(&posStep, &g_pCurrentCar->velocity, g_physicsTimeStep);
    halfStep = g_physicsTimeStep / 2;
    FixVecScale(&accel, &accel, halfStep);
    posStep.x = posStep.x - accel.x;
    posStep.y = posStep.y - accel.y;
    posStep.z = posStep.z - accel.z;

    if (locked != 0) {
        int angAccelY = -FixMul(torque.y, g_pCurrentCar->inertia.y);

        angAccelY = angAccelY - FixMul(g_pCurrentCar->field_0x5d0.y,
                                       FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c));
        angAccelY = FixMul(angAccelY, g_physicsTimeStep);
        g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y + angAccelY;
        angStep.y = FixMul(g_pCurrentCar->angularVelocity.y, g_physicsTimeStep);
        halfStep = g_physicsTimeStep / 2;
        angAccelY = FixMul(angAccelY, halfStep);
        angStep.y = angStep.y - angAccelY;
    } else {
        FixVector angAccel;

        angAccel.x = -FixMul(torque.x, g_pCurrentCar->inertia.x);
        angAccel.y = -FixMul(torque.y, g_pCurrentCar->inertia.y);
        angAccel.z = -FixMul(torque.z, g_pCurrentCar->inertia.z);
        angAccel.x = angAccel.x - FixMul(g_pCurrentCar->field_0x5d0.x,
                                         FixMul(g_pCurrentCar->inertia.x, g_pCurrentCar->field_0x75c));
        angAccel.y = angAccel.y - FixMul(g_pCurrentCar->field_0x5d0.y,
                                         FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c));
        angAccel.z = angAccel.z - FixMul(g_pCurrentCar->field_0x5d0.z,
                                         FixMul(g_pCurrentCar->inertia.z, g_pCurrentCar->field_0x75c));
        FixVecScale(&angAccel, &angAccel, g_physicsTimeStep);
        g_pCurrentCar->angularVelocity.x = g_pCurrentCar->angularVelocity.x + angAccel.x;
        g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y + angAccel.y;
        g_pCurrentCar->angularVelocity.z = g_pCurrentCar->angularVelocity.z + angAccel.z;
        FixVecScale(&angStep, &g_pCurrentCar->angularVelocity, g_physicsTimeStep);
        halfStep = g_physicsTimeStep / 2;
        FixVecScale(&angAccel, &angAccel, halfStep);
        angStep.x = angStep.x - angAccel.x;
        angStep.y = angStep.y - angAccel.y;
        angStep.z = angStep.z - angAccel.z;
    }

    if (g_pCurrentCar->field_0xb34 > 0) {
        int upDot = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->up);
        if (FIX_ABS(angStep.x) < 6 && FIX_ABS(upDot) > 0xcccc) {
            angStep.x = 0;
        }
        if ((g_pCurrentCar->flag0x1d0[0] == 0 && g_pCurrentCar->flag0x1d0[1] == 0) ||
            FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            if (FIX_ABS(angStep.y) < 6) {
                angStep.y = 0;
            }
        }
        if (FIX_ABS(angStep.z) < 6) {
            angStep.z = 0;
        }
        if (g_pCurrentCar->speed < 0xccc) {
            posStep.x = 0;
            posStep.y = 0;
            posStep.z = 0;
        }
    }

    if (locked != 0) {
        angStep.x = 0;
        angStep.z = 0;
    }
    g_pCurrentCar->position.x = g_pCurrentCar->position.x + posStep.x;
    g_pCurrentCar->position.y = g_pCurrentCar->position.y + posStep.y;
    g_pCurrentCar->position.z = g_pCurrentCar->position.z + posStep.z;
    g_pCurrentCar->field_0xb60 = 0;
    if (angStep.x == 0 && angStep.y == 0 && angStep.z == 0) {
        if (g_pCurrentCar->speed < 0x51e) {
            g_pCurrentCar->field_0xb60 = 1;
        }
    } else if (locked == 0) {
        FixBasis_Integrate(&g_pCurrentCar->right, &angStep);
    } else {
        off = 0;
        do {
            FixVector *pAxis = (FixVector *)((BYTE *)g_pCurrentCar + off + 0x360);
            tmp.x = FixMul(angStep.y, pAxis->z);
            tmp.y = 0;
            tmp.z = -FixMul(angStep.y, pAxis->x);
            pAxis->x = pAxis->x + tmp.x;
            pAxis->y = pAxis->y + tmp.y;
            pAxis->z = pAxis->z + tmp.z;
            off += 0x18;
        } while (off < 0x24);
    }

    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    Car_UpdateCorners(g_pCurrentCar);
}

// Builds the force each wheel puts on the ground: first the drive/brake torque
// per wheel (with the handbrake and the traction limits), then the tyre force
// itself, combining the rolling and the lateral slip. Only the right hand
// wheels are solved; the left hand ones are mirrored from them.
// Incomplete: the friction-ellipse section of the second loop (the grip limit
// built from wheelParams[i] plus the FixAtan2/FixSqrt combination, and the
// difficulty-assist scaling around CGameInfo::FUN_00405d90) is still missing,
// so this only covers about two thirds of the original.
// TODO: CMR2 0x00441500 (implemented, match 46%)
void Car_UpdateWheelForces(void)
{
    FixVector dirRear;
    FixVector dirFront;
    FixVector axis[4];
    FixVector dir;
    FixVector force;
    int dot[4];
    int brakeA;
    int brakeB;
    int scale;
    int loadScale;
    int torqueScale;
    int torqueLimit;
    int i;

    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0 &&
        g_pCurrentCar->field_0xb1e != 0) {
        brakeA = FixMul(0x1999, g_pCurrentCar->field_0x830);
        brakeB = FixMul(0x1999, 0x10000 - g_pCurrentCar->field_0x830);
    } else {
        brakeA = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x3fc)),
                        g_pCurrentCar->field_0x830);
        brakeB = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x400)),
                        0x10000 - g_pCurrentCar->field_0x830);
    }
    if (g_pCurrentCar->field_0x1d8 != 0) {
        brakeB = brakeB + g_pCurrentCar->field_0x848;
    }

    dirRear = g_pCurrentCar->wheelDirRear;
    axis[0] = g_pCurrentCar->wheelAxisRear;
    dirFront = g_pCurrentCar->wheelDirFront;
    {
        FixVector cross;
        int len;

        FixVecCross(&cross, &g_pCurrentCar->wheelDirFront, &g_pCurrentCar->up);
        len = FixVecLength(&cross);
        if (len == 0) {
            axis[1].x = 0;
            axis[1].y = 0;
            axis[1].z = 0;
        } else {
            FixVecScaleRecip(&axis[1], &cross, len);
        }
    }

    scale = FixMul(g_pCurrentCar->field_0x8b4, g_physicsScale);
    loadScale = FixMul(g_physicsScale, 0xf000);
    torqueScale = FixMul(g_physicsTimeStep, 0x11113);
    torqueLimit = FIX_ABS(FixMul(g_pCurrentCar->field_0x794,
                                 g_pCurrentCar->field_0x7dc[g_pCurrentCar->field_0xb1e]));

    i = 3;
    do {
        int load;
        int slip;
        int target;
        int brake;
        int limited;

        dir = i > 1 ? dirFront : dirRear;
        limited = *(int *)(g_pCarSetup + 0x3ec + i * 4);
        load = g_pCurrentCar->wheelLoad[i];
        if (load >= 1) {
            brake = i > 1 ? brakeB : brakeA;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - brake;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - limited;
        } else if (load < 0) {
            brake = i > 1 ? brakeB : brakeA;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + brake;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limited;
        }

        slip = FixVecDot(&dir, &g_pCurrentCar->cornerVelocity[i]);
        load = g_pCurrentCar->wheelLoad[i];
        dot[i] = slip;
        target = slip * 4 - load;
        if (i <= 1 || g_pCurrentCar->field_0x1d8 == 0 ||
            (slip > 0 && target < 0) || (slip < 0 && target > 0)) {
            int grip;
            int limit = FixMul(target, loadScale) * 4;

            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                grip = g_pCurrentCar->field_0xa5c[i];
            } else {
                int slide = FIX_ABS(FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]));
                int room = FixMul(FIX_ABS(target), 0x4000) - slide;

                if (room < 0) {
                    room = 0;
                }
                room = FixMul(room, 0xccc) + 0x51e;
                if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0) {
                    room = room + FixMul(*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24), 0xe666);
                }
                grip = FixMul(g_pCurrentCar->field_0xa5c[i], room);
            }
            if (grip < limit || (grip = -grip, limit < grip)) {
                limit = grip;
            }
            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                limit = limit / 4;
            } else {
                limit = limit / 8;
            }
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limit;
        } else {
            g_pCurrentCar->wheelTorque[i] = 0;
            load = g_pCurrentCar->wheelLoad[i];
            if (load >= 1) {
                brake = i > 1 ? brakeB : brakeA;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - brake;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - limited;
            } else if (load < 0) {
                brake = i > 1 ? brakeB : brakeA;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + brake;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limited;
            }
        }

        load = g_pCurrentCar->wheelLoad[i];
        g_pCurrentCar->wheelLoad[i] = g_pCurrentCar->wheelLoad[i] +
                                      FixMul(g_pCurrentCar->wheelTorque[i], torqueScale);
        if (torqueLimit < g_pCurrentCar->wheelLoad[i] ||
            (torqueLimit = -torqueLimit, g_pCurrentCar->wheelLoad[i] < torqueLimit)) {
            g_pCurrentCar->wheelLoad[i] = torqueLimit;
        }
        torqueLimit = FIX_ABS(torqueLimit);
        if (g_pCurrentCar->field_0xb1e == 0 || g_pCurrentCar->field_0xb84 != 0 ||
            g_pCurrentCar->field_0x7b4 == (i > 1 ? 0x10000 : 0x0)) {
            if ((load >= 1 && g_pCurrentCar->wheelLoad[i] < 0) ||
                (load < 0 && g_pCurrentCar->wheelLoad[i] > 0)) {
                g_pCurrentCar->wheelLoad[i] = 0;
            }
        } else {
            int gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e];

            if ((gear > 0 && g_pCurrentCar->wheelLoad[i] < 0) ||
                (gear < 0 && g_pCurrentCar->wheelLoad[i] > 0)) {
                g_pCurrentCar->wheelLoad[i] = 0;
            }
        }
        i -= 2;
    } while (i >= 0);

    g_pCurrentCar->wheelTorque[0] = g_pCurrentCar->wheelTorque[1];
    g_pCurrentCar->wheelTorque[2] = g_pCurrentCar->wheelTorque[3];
    g_pCurrentCar->wheelLoad[0] = g_pCurrentCar->wheelLoad[1];
    g_pCurrentCar->wheelLoad[2] = g_pCurrentCar->wheelLoad[3];

    i = 3;
    do {
        int susp;
        int lateral;
        int fLong;
        int fLat;
        int len;
        int combined;

        dir = i > 1 ? dirFront : dirRear;
        axis[i] = i > 1 ? axis[1] : axis[0];
        susp = dot[i] * 4 - g_pCurrentCar->wheelLoad[i];
        g_pCurrentCar->field_0x870[i] = susp;
        fLong = -FixMul(FixMul(susp, 0x4000), scale);
        lateral = FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]);
        g_pCurrentCar->field_0x880[i] = lateral;
        fLat = -FixMul(lateral, scale);

        if (FIX_ABS(fLong) <= 0x10000 || FIX_ABS(fLat) >= FIX_ABS(fLong)) {
            if (FIX_ABS(fLat) > 0x10000 && FIX_ABS(fLong) <= FIX_ABS(fLat)) {
                int recip = FixDiv(0x10000, FIX_ABS(fLat));
                int nLong = FixMul(fLong, recip);
                int nLat = FixMul(fLat, recip);

                force.x = FixMul(axis[i].x, nLat) + FixMul(dir.x, nLong);
                force.y = FixMul(axis[i].y, nLat) + FixMul(dir.y, nLong);
                force.z = FixMul(axis[i].z, nLat) + FixMul(dir.z, nLong);
                len = FixVecLength(&force);
                if (len == 0) {
                    g_pCurrentCar->cornerForce[i].x = 0;
                    g_pCurrentCar->cornerForce[i].y = 0;
                    g_pCurrentCar->cornerForce[i].z = 0;
                } else {
                    FixVecScaleRecip(&g_pCurrentCar->cornerForce[i], &force, len);
                }
                combined = FixMul(FixVecLength(&force), FIX_ABS(fLat));
            } else {
                force.x = FixMul(axis[i].x, fLat) + FixMul(dir.x, fLong);
                force.y = FixMul(axis[i].y, fLat) + FixMul(dir.y, fLong);
                force.z = FixMul(axis[i].z, fLat) + FixMul(dir.z, fLong);
                len = FixVecLength(&force);
                if (len == 0) {
                    g_pCurrentCar->cornerForce[i].x = 0;
                    g_pCurrentCar->cornerForce[i].y = 0;
                    g_pCurrentCar->cornerForce[i].z = 0;
                } else {
                    FixVecScaleRecip(&g_pCurrentCar->cornerForce[i], &force, len);
                }
                combined = FixVecLength(&force);
            }
        } else {
            int recip = FixDiv(0x10000, FIX_ABS(fLong));
            int nLong = FixMul(fLong, recip);
            int nLat = FixMul(fLat, recip);

            force.x = FixMul(axis[i].x, nLat) + FixMul(dir.x, nLong);
            force.y = FixMul(axis[i].y, nLat) + FixMul(dir.y, nLong);
            force.z = FixMul(axis[i].z, nLat) + FixMul(dir.z, nLong);
            len = FixVecLength(&force);
            if (len == 0) {
                g_pCurrentCar->cornerForce[i].x = 0;
                g_pCurrentCar->cornerForce[i].y = 0;
                g_pCurrentCar->cornerForce[i].z = 0;
            } else {
                FixVecScaleRecip(&g_pCurrentCar->cornerForce[i], &force, len);
            }
            combined = FixMul(FixVecLength(&force), FIX_ABS(fLong));
        }

        {
            int grip = FixMul(g_pCurrentCar->field_0xa4c[i], g_pCurrentCar->field_0xa5c[i]);

            if (combined < grip) {
                grip = combined;
            }
            g_pCurrentCar->cornerForce[i].x = FixMul(g_pCurrentCar->cornerForce[i].x, grip);
            g_pCurrentCar->cornerForce[i].y = FixMul(g_pCurrentCar->cornerForce[i].y, grip);
            g_pCurrentCar->cornerForce[i].z = FixMul(g_pCurrentCar->cornerForce[i].z, grip);
        }

        if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0) {
            int along = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &dir);

            if (along != 0) {
                int drag = -FixMul(along / 4,
                                   FixMul(g_pCurrentCar->field_0x8b4,
                                          *(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24)));

                g_pCurrentCar->cornerForce[i].x = g_pCurrentCar->cornerForce[i].x + FixMul(dir.x, drag);
                g_pCurrentCar->cornerForce[i].y = g_pCurrentCar->cornerForce[i].y + FixMul(dir.y, drag);
                g_pCurrentCar->cornerForce[i].z = g_pCurrentCar->cornerForce[i].z + FixMul(dir.z, drag);
            }
        }
        i -= 2;
    } while (i >= 0);

    g_pCurrentCar->field_0x870[0] = g_pCurrentCar->field_0x870[1];
    g_pCurrentCar->field_0x870[2] = g_pCurrentCar->field_0x870[3];
    g_pCurrentCar->cornerForce[0] = g_pCurrentCar->cornerForce[1];
    g_pCurrentCar->cornerForce[2] = g_pCurrentCar->cornerForce[3];
}
