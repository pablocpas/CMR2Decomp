#include <windows.h>
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
// GLOBAL: CMR2 0x00519c8c
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
        step = FixMul(g_pCurrentCar->field_0x424, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->field_0x424 = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->steer) < 0x28f) {
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
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->steer, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->steer, 0x2604));

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
            int s = FixMul(FixMul(g_pCurrentCar->steer, 0x1e0000), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->force0x648.x += localUp.x;
            g_pCurrentCar->force0x648.y += localUp.y;
            g_pCurrentCar->force0x648.z += localUp.z;
            g_pCurrentCar->force0x654.x += localUp.x;
            g_pCurrentCar->force0x654.y += localUp.y;
            g_pCurrentCar->force0x654.z += localUp.z;
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
                d = FixVecDot((FixVector *)&g_pCurrentCar->field_0x498, &localUp) - FixVecDot((FixVector *)&g_pCurrentCar->field_0x48c, &localUp);
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
        step = FixMul(g_pCurrentCar->field_0x424, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->field_0x424 = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->steer) < 0x28f) {
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
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->steer, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->steer, 0x2604));

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
            int s = FixMul(FixMul(g_pCurrentCar->steer, 0x1e0000), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->force0x648.x += localUp.x;
            g_pCurrentCar->force0x648.y += localUp.y;
            g_pCurrentCar->force0x648.z += localUp.z;
            g_pCurrentCar->force0x654.x += localUp.x;
            g_pCurrentCar->force0x654.y += localUp.y;
            g_pCurrentCar->force0x654.z += localUp.z;
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
