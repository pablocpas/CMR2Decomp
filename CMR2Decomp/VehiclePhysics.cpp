#include <windows.h>
#include "Car.h"
#include "FixedPoint.h"
#include "GameInfo.h"

// GLOBAL: CMR2 0x00519c90
int g_collisionAngularScale = 0x10000;

// GLOBAL: CMR2 0x005918dc
Car *g_collisionCar;
// GLOBAL: CMR2 0x00591910
FixVector g_collisionDirection;
// GLOBAL: CMR2 0x00591ae0
FixVector g_collisionTarget;

#define COLLISION_VECTOR(offset) (*(FixVector *)((BYTE *)g_collisionCar + (offset)))
#define COLLISION_INT(offset) (*(int *)((BYTE *)g_collisionCar + (offset)))

// Applies a world-space impulse and its resulting torque to the active car.
// FUNCTION: CMR2 0x0048f470
void CarPhysics_ApplyImpulse(FixVector *pImpulse, FixVector *pPoint, int usePoint)
{
    char collisionMode;
    FixVector offset;
    FixVector torque;
    FixVector scaledImpulse;
    FixVector localOffset;
    FixVector localImpulse;
    int component;
    int absoluteComponent;
    int limit;

    collisionMode = 0;
    if (usePoint == 0) {
        offset.x = g_collisionTarget.x - g_collisionCar->position.x;
        offset.y = g_collisionTarget.y - g_collisionCar->position.y;
        offset.z = g_collisionTarget.z - g_collisionCar->position.z;
        if (FixVecDot(&offset, &g_collisionDirection) > 0)
            FixVecScale(&offset, &g_collisionDirection, 0x20000);
        else
            FixVecScale(&offset, &g_collisionDirection, -0x20000);
    } else {
        offset.x = pPoint->x - g_collisionCar->position.x;
        offset.y = pPoint->y - g_collisionCar->position.y;
        offset.z = pPoint->z - g_collisionCar->position.z;
    }

    FixMatrix_InverseRotateVector(&COLLISION_VECTOR(0x5dc), &offset, g_collisionCar->pWorld);

    component = COLLISION_INT(0x5dc);
    absoluteComponent = component;
    if (absoluteComponent < 0)
        absoluteComponent = -absoluteComponent;
    limit = g_collisionCar->halfExtents.x;
    if (absoluteComponent > limit) {
        if (component < 0)
            limit = -limit;
        COLLISION_INT(0x5dc) = limit;
    }
    component = COLLISION_INT(0x5e0);
    absoluteComponent = component;
    if (absoluteComponent < 0)
        absoluteComponent = -absoluteComponent;
    limit = g_collisionCar->halfExtents.y;
    if (absoluteComponent > limit) {
        if (component < 0)
            limit = -limit;
        COLLISION_INT(0x5e0) = limit;
    }
    component = COLLISION_INT(0x5e4);
    absoluteComponent = component;
    if (absoluteComponent < 0)
        absoluteComponent = -absoluteComponent;
    limit = g_collisionCar->halfExtents.z;
    if (absoluteComponent > limit) {
        if (component < 0)
            limit = -limit;
        COLLISION_INT(0x5e4) = limit;
    }

    g_collisionCar->velocity.x += pImpulse->x;
    g_collisionCar->velocity.y += pImpulse->y;
    g_collisionCar->velocity.z += pImpulse->z;
    if (CGameInfo::FUN_004063f0(4) != 0) {
        g_collisionCar->velocity.x += pImpulse->x;
        g_collisionCar->velocity.y += pImpulse->y;
        g_collisionCar->velocity.z += pImpulse->z;
    }
    if (COLLISION_INT(0xc00) != 0)
        return;

    usePoint = FixVecLength(pImpulse);
    if (g_collisionCar->field_0xb64 == 0) {
        if (usePoint > 0x13333)
            collisionMode = 1;
        else if (usePoint > 0xe666)
            collisionMode = 2;
    }

    FixVecScale(&scaledImpulse, pImpulse, 0x28000);
    usePoint = FixMul(usePoint, 0x28000);
    if (usePoint > 0x34ccc) {
        FixVecScaleRecip(&scaledImpulse, &scaledImpulse, usePoint);
        FixVecScale(&scaledImpulse, &scaledImpulse, 0x34ccc);
    }
    FixMatrix_InverseRotateVector(&localImpulse, &scaledImpulse, g_collisionCar->pWorld);

    offset = *pPoint;
    if (collisionMode == 1) {
        offset.y -= 0x40000;
        offset.x -= g_collisionCar->position.x;
        offset.y -= g_collisionCar->position.y;
        offset.z -= g_collisionCar->position.z;
        FixMatrix_InverseRotateVector(&localOffset, &offset, g_collisionCar->pWorld);
        FixVecCross(&torque, &localImpulse, &localOffset);
        FixVecScale(&torque, &torque, 0x40000);
        g_collisionCar->velocity.y += 0x4000;
    } else {
        if (collisionMode == 2)
            offset.y -= 0x40000;
        offset.x -= g_collisionCar->position.x;
        offset.y -= g_collisionCar->position.y;
        offset.z -= g_collisionCar->position.z;
        FixMatrix_InverseRotateVector(&localOffset, &offset, g_collisionCar->pWorld);
        FixVecCross(&torque, &localImpulse, &localOffset);
        if (collisionMode != 2)
            goto apply_torque;
    }

    COLLISION_INT(0xc00) = 1;
    COLLISION_INT(0x96c) = 0x10000;
    COLLISION_INT(0xc04) = 1;

apply_torque:
    if ((torque.x < 0 ? -torque.x : torque.x) > 0x80000)
        torque.x = torque.x > 0 ? 0x80000 : -0x80000;
    if ((torque.y < 0 ? -torque.y : torque.y) > 0x80000)
        torque.y = torque.y > 0 ? 0x80000 : -0x80000;
    if ((torque.z < 0 ? -torque.z : torque.z) > 0x80000)
        torque.z = torque.z > 0 ? 0x80000 : -0x80000;

    usePoint = g_collisionAngularScale;
    FixVecScale(&torque, &torque, usePoint);
    COLLISION_INT(0x5d0) += torque.x;
    COLLISION_INT(0x5d4) += torque.y;
    COLLISION_INT(0x5d8) += torque.z;
}
