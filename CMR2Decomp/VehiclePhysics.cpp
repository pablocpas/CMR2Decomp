#include <windows.h>
#include "Car.h"
#include "FixedPoint.h"
#include "GameInfo.h"

// Defined as g_physicsScale in Car.cpp (0x00519c90).

// GLOBAL: CMR2 0x005918dc
Car *g_collisionCar;
// GLOBAL: CMR2 0x00591910
FixVector g_collisionDirection;
// GLOBAL: CMR2 0x00591ae0
FixVector g_collisionTarget;

struct CollisionFaceVertices {
    BYTE pad0x00[0x2c];
    int hasSecondaryVertices;
    FixVector primaryVertices[4];
    FixVector secondaryVertices[4];
};

struct VehicleMotionState {
    BYTE pad0x000[4];
    FixMatrix *pMatrix;
    BYTE pad0x008[0x118];
    FixVector position;
    FixVector previousPosition;
    FixVector velocity;
    FixVector correction;
    BYTE flags;
    BYTE pad0x151[7];
    unsigned short directionAngle;
};

struct VehicleMotionContext {
    BYTE pad0x000[0x750];
    FixMatrix *pMatrix;
};

// GLOBAL: CMR2 0x0059192c
int g_collisionSelectBackSide;
// GLOBAL: CMR2 0x00591934
BYTE g_collisionNegativeVertexCount;
// GLOBAL: CMR2 0x00591935
BYTE g_collisionPositiveVertexCount;
// GLOBAL: CMR2 0x00591944
BYTE g_collisionNegativeCandidateCount;
// GLOBAL: CMR2 0x00591945
BYTE g_collisionPositiveCandidateCount;
// GLOBAL: CMR2 0x00591984
CollisionFaceVertices *g_collisionFace;
// GLOBAL: CMR2 0x00591988
BYTE g_collisionNegativeCandidates[4];
// GLOBAL: CMR2 0x0059198c
BYTE g_collisionPositiveCandidates[4];
// GLOBAL: CMR2 0x005919a4
int g_collisionVertexDistances[4];
// GLOBAL: CMR2 0x005919b4
int g_collisionBestVertex;
// GLOBAL: CMR2 0x00591ac0
FixVector g_collisionLineStart;
// GLOBAL: CMR2 0x00591adc
int g_collisionDirectionDirty;

// TODO: the same object as g_unk0x00590c20 in StageTiming.cpp; the annotation
// lives there until both views of the struct are merged into one type.
VehicleMotionState *g_vehicleMotionState;
// TODO: the same object as g_unk0x00590d74 in StageTiming.cpp.
VehicleMotionContext *g_vehicleMotionContext;

#define COLLISION_VECTOR(offset) (*(FixVector *)((BYTE *)g_collisionCar + (offset)))
#define COLLISION_INT(offset) (*(int *)((BYTE *)g_collisionCar + (offset)))

inline int Collision_FixSqrt(int value)
{
    __asm {
        mov eax, value
        or eax, eax
        mov ebx, eax
        jnz sqrt_nonzero
        mov eax, 0
        jmp sqrt_done
    sqrt_nonzero:
        xor ecx, ecx
        cmp eax, 0x10000
        jb sqrt_l1
        shr eax, 16
        add cl, 16
    sqrt_l1:
        cmp eax, 0x100
        jb sqrt_l2
        shr eax, 8
        add cl, 8
    sqrt_l2:
        cmp eax, 0x10
        jb sqrt_l3
        shr eax, 4
        add cl, 4
    sqrt_l3:
        cmp eax, 4
        jb sqrt_l4
        shr eax, 2
        add cl, 2
    sqrt_l4:
        cmp eax, 2
        jb sqrt_l5
        inc ecx
    sqrt_l5:
        mov eax, ebx
        sub cl, 15
        test cl, 1
        jz sqrt_l6
        inc cl
    sqrt_l6:
        mov bl, cl
        add cl, 4
        jns sqrt_l7
        neg cl
        shl eax, cl
        jmp sqrt_l8
    sqrt_l7:
        shr eax, cl
    sqrt_l8:
        sar bl, 1
        mov ax, word ptr [eax * 2 + g_sqrtTable]
        or bl, bl
        mov cl, bl
        js sqrt_l9
        shl eax, cl
        jmp sqrt_done
    sqrt_l9:
        neg cl
        shr eax, cl
    sqrt_done:
    }
}

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

    usePoint = g_physicsScale;
    FixVecScale(&torque, &torque, usePoint);
    COLLISION_INT(0x5d0) += torque.x;
    COLLISION_INT(0x5d4) += torque.y;
    COLLISION_INT(0x5d8) += torque.z;
}

// Plane normal x/z of the collision face and its accepted distance.
// GLOBAL: CMR2 0x005918d0
int g_unk0x005918d0;
// GLOBAL: CMR2 0x0059195c
int g_unk0x0059195c;
// GLOBAL: CMR2 0x005919b8
int g_unk0x005919b8;

// Accepts the collision direction when it is nearly parallel to the face and
// picks the side of the car the target is on.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490570
int FUN_00490570(void)
{
    int d = FixMul(g_unk0x0059195c, g_collisionDirection.z) + FixMul(g_unk0x005918d0, g_collisionDirection.x);
    BYTE *pCar = (BYTE *)g_collisionCar;

    if (d < 0)
        d = -d;
    if (g_unk0x005919b8 < d)
        return 0;
    g_collisionSelectBackSide =
        FixMul(g_collisionTarget.z - *(int *)(pCar + 0x2f0), g_collisionDirection.z) +
        FixMul(g_collisionTarget.x - *(int *)(pCar + 0x2e8), g_collisionDirection.x) +
        FixMul(g_collisionTarget.y - *(int *)(pCar + 0x2ec), g_collisionDirection.y) >= 0;
    return 1;
}

// Accepts the collision when the target is behind the face and the direction
// is nearly parallel to it.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490640
int FUN_00490640(void)
{
    BYTE *pCar = (BYTE *)g_collisionCar;
    int side;
    int d;

    side = FixMul(g_collisionTarget.z - *(int *)(pCar + 0x2f0), g_collisionDirection.z) +
           FixMul(g_collisionTarget.x - *(int *)(pCar + 0x2e8), g_collisionDirection.x) +
           FixMul(g_collisionTarget.y - *(int *)(pCar + 0x2ec), g_collisionDirection.y);
    if (side > 0)
        return 0;
    d = FixMul(g_unk0x0059195c, g_collisionDirection.z) + FixMul(g_unk0x005918d0, g_collisionDirection.x);
    if (d < 0)
        d = -d;
    if (g_unk0x005919b8 < d)
        return 0;
    g_collisionSelectBackSide = side >= 0;
    return 1;
}

// Classifies face vertices by signed distance from the active collision plane.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490720
void Collision_ClassifyFaceVertices(void)
{
    FixVector newDirection;
    FixVector sideDirection;
    FixVector targetDelta;
    FixVector startDelta;
    FixVector secondaryDelta;
    int length;
    int sideAtTarget;
    int sideAtStart;
    int secondaryDistance;
    int vertexOffset;
    int vertexIndex;
    int *pDistance;

    if (g_collisionDirectionDirty != 0) {
        g_collisionDirectionDirty = 0;
        newDirection.x = g_collisionLineStart.z - g_collisionTarget.z;
        newDirection.y = 0;
        newDirection.z = g_collisionTarget.x - g_collisionLineStart.x;

        length = Collision_FixSqrt(FixMul(newDirection.x, newDirection.x) +
                                   FixMul(newDirection.z, newDirection.z));
        if (FixVecDot(&newDirection, &g_collisionDirection) <= 0) {
            length = -length;
            FixVecScaleRecip(&newDirection, &newDirection, length);
        } else {
            FixVecScaleRecip(&newDirection, &newDirection, length);
        }
        g_collisionDirection = newDirection;
    }

    sideDirection.x = -g_collisionDirection.z;
    sideDirection.y = 0;
    sideDirection.z = g_collisionDirection.x;

    g_collisionNegativeCandidateCount = 0;
    g_collisionPositiveCandidateCount = 0;
    g_collisionNegativeVertexCount = 0;
    g_collisionPositiveVertexCount = 0;
    vertexIndex = 0;
    vertexOffset = 0;
    pDistance = g_collisionVertexDistances;

    do {
        FixVector *pPrimary = (FixVector *)(vertexOffset + (int)g_collisionFace + 0x30);
        targetDelta.x = g_collisionTarget.x - pPrimary->x;
        targetDelta.y = g_collisionTarget.y - pPrimary->y;
        targetDelta.z = g_collisionTarget.z - pPrimary->z;
        startDelta.x = g_collisionLineStart.x - pPrimary->x;
        startDelta.y = g_collisionLineStart.y - pPrimary->y;
        startDelta.z = g_collisionLineStart.z - pPrimary->z;

        sideAtTarget = FixVecDot(&targetDelta, &sideDirection);
        sideAtStart = FixVecDot(&startDelta, &sideDirection);
        *pDistance = FixVecDot(&targetDelta, &g_collisionDirection);
        if (*pDistance >= 0)
            g_collisionPositiveVertexCount++;
        else
            g_collisionNegativeVertexCount++;

        if ((sideAtTarget != 0) && (sideAtStart != 0) &&
            ((sideAtTarget <= 0) || (sideAtStart >= 0)) &&
            ((sideAtTarget >= 0) || (sideAtStart <= 0)))
            goto nextVertex;

        if (*pDistance >= 0) {
            if (g_collisionSelectBackSide == 0)
                goto checkSecondary;
            goto addVertex;
        } else {
            if (g_collisionSelectBackSide == 0)
                goto addVertex;
checkSecondary:
            if (g_collisionFace->hasSecondaryVertices == 0)
                goto addVertex;

            {
                FixVector *pSecondary = (FixVector *)(vertexOffset + (int)g_collisionFace + 0x60);
                secondaryDelta.x = g_collisionTarget.x - pSecondary->x;
                secondaryDelta.y = g_collisionTarget.y - pSecondary->y;
                secondaryDelta.z = g_collisionTarget.z - pSecondary->z;
            }
            secondaryDistance = FixVecDot(&secondaryDelta, &g_collisionDirection);
            if (*pDistance > 0) {
                if (secondaryDistance <= *pDistance)
                    goto addVertex;
            } else if ((*pDistance >= 0) || (secondaryDistance >= *pDistance))
                goto addVertex;
            goto nextVertex;
        }

addVertex:
        if (*pDistance >= 0) {
            BYTE candidateIndex = g_collisionPositiveCandidateCount;
            BYTE vertex = (BYTE)vertexIndex;
            *(volatile BYTE *)&g_collisionPositiveCandidateCount = (BYTE)(candidateIndex + 1);
            ((volatile BYTE *)g_collisionPositiveCandidates)[candidateIndex] = vertex;
        } else {
            g_collisionNegativeCandidates[g_collisionNegativeCandidateCount++] = (BYTE)vertexIndex;
        }

nextVertex:
        vertexOffset += sizeof(FixVector);
        vertexIndex++;
        pDistance++;
    } while ((int)pDistance < (int)&g_collisionBestVertex);
}

// Updates the vehicle-local motion vector and the resulting positional correction.
// FUNCTION: CMR2 0x00482ac0
void Vehicle_UpdateMotion(FixVector *pInput)
{
    FixVector localInput;
    FixVector delta;
    FixVector rotatedDelta;
    FixVector direction;
    int projection;
    int length;

    FixMatrix_InverseRotateVector(&localInput, pInput, g_vehicleMotionContext->pMatrix);
    FixVecScale(&g_vehicleMotionState->velocity, &localInput, 0x1999);

    if ((g_vehicleMotionState->flags & 0xf0) == 0) {
        if (g_vehicleMotionState->velocity.y < 0)
            g_vehicleMotionState->velocity.y = 0;

        direction.x = FixSin(g_vehicleMotionState->directionAngle);
        direction.y = FixSin(g_vehicleMotionState->directionAngle + 0x400);
        direction.z = 0;
        projection = FixVecDot(&g_vehicleMotionState->velocity, &direction);
        if (projection < 0) {
            length = FixVecLength(&g_vehicleMotionState->velocity);
            FixVecScale(&direction, &direction, projection);
            g_vehicleMotionState->velocity.x -= direction.x;
            g_vehicleMotionState->velocity.y -= direction.y;
            g_vehicleMotionState->velocity.z -= direction.z;

            FixVector *pVelocity = &g_vehicleMotionState->velocity;
            int normalizedLength = FixVecLength(pVelocity);
            if (normalizedLength == 0) {
                pVelocity->x = 0;
                pVelocity->y = 0;
                pVelocity->z = 0;
            } else {
                FixVecScaleRecip(pVelocity, pVelocity, normalizedLength);
            }

            length = FixMul(length, 0x50000);
            FixVecScale(&g_vehicleMotionState->velocity,
                        &g_vehicleMotionState->velocity, length);
        }
    }

    delta.x = g_vehicleMotionState->position.x - g_vehicleMotionState->previousPosition.x;
    delta.y = g_vehicleMotionState->position.y - g_vehicleMotionState->previousPosition.y;
    delta.z = g_vehicleMotionState->position.z - g_vehicleMotionState->previousPosition.z;
    FixMatrix_RotateVector(&rotatedDelta, &delta, g_vehicleMotionState->pMatrix);
    rotatedDelta.x += g_vehicleMotionState->previousPosition.x;
    rotatedDelta.y += g_vehicleMotionState->previousPosition.y;
    rotatedDelta.z += g_vehicleMotionState->previousPosition.z;
    g_vehicleMotionState->correction.x = g_vehicleMotionState->position.x - rotatedDelta.x;
    g_vehicleMotionState->correction.y = g_vehicleMotionState->position.y - rotatedDelta.y;
    g_vehicleMotionState->correction.z = g_vehicleMotionState->position.z - rotatedDelta.z;
    g_vehicleMotionState->position = g_vehicleMotionState->previousPosition;
    g_vehicleMotionState->flags |= 4;
    g_vehicleMotionState->flags &= (BYTE)~2;
}
