#include <windows.h>
#include "Car.h"
#include "CarParts.h"
#include "FixedPoint.h"
#include "GameInfo.h"
#include <string.h>

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

// The tracked object is the one defined (and annotated) in StageTiming.cpp:
// both struct views are the same memory, so reference that symbol instead of
// keeping a second definition. The casts preserve this file's field layout.

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
    int len;

    collisionMode = 0;
    if (usePoint == 0) {
        offset.x = g_collisionTarget.x - g_collisionCar->position.x;
        offset.y = g_collisionTarget.y - g_collisionCar->position.y;
        offset.z = g_collisionTarget.z - g_collisionCar->position.z;
        if (FixVecDot(&offset, &g_collisionDirection) > 0)
            FixVecScale(&offset, &g_collisionDirection, 0x20000);
        else
            FixVecScale(&offset, &g_collisionDirection, -0x20000);
        FixMatrix_InverseRotateVector(&g_collisionCar->field_0x5dc, &offset, g_collisionCar->pWorld);
    } else {
        offset.x = pPoint->x - g_collisionCar->position.x;
        offset.y = pPoint->y - g_collisionCar->position.y;
        offset.z = pPoint->z - g_collisionCar->position.z;
        FixMatrix_InverseRotateVector(&g_collisionCar->field_0x5dc, &offset, g_collisionCar->pWorld);
    }

    component = g_collisionCar->field_0x5dc.x;
    absoluteComponent = component < 0 ? -component : component;
    if (absoluteComponent > g_collisionCar->halfExtents.x)
        g_collisionCar->field_0x5dc.x = component < 0 ? -g_collisionCar->halfExtents.x : g_collisionCar->halfExtents.x;
    component = g_collisionCar->field_0x5dc.y;
    absoluteComponent = component < 0 ? -component : component;
    if (absoluteComponent > g_collisionCar->halfExtents.y)
        g_collisionCar->field_0x5dc.y = component < 0 ? -g_collisionCar->halfExtents.y : g_collisionCar->halfExtents.y;
    component = g_collisionCar->field_0x5dc.z;
    absoluteComponent = component < 0 ? -component : component;
    if (absoluteComponent > g_collisionCar->halfExtents.z)
        g_collisionCar->field_0x5dc.z = component < 0 ? -g_collisionCar->halfExtents.z : g_collisionCar->halfExtents.z;

    g_collisionCar->velocity.x += pImpulse->x;
    g_collisionCar->velocity.y += pImpulse->y;
    g_collisionCar->velocity.z += pImpulse->z;
    if (CGameInfo::IsActiveCheatEnabled(4) != 0) {
        g_collisionCar->velocity.x += pImpulse->x;
        g_collisionCar->velocity.y += pImpulse->y;
        g_collisionCar->velocity.z += pImpulse->z;
    }
    if (g_collisionCar->field_0xc00 != 0)
        return;

    len = FixVecLength(pImpulse);
    if (g_collisionCar->field_0xb64 == 0) {
        if (len > 0x13333)
            collisionMode = 1;
        else if (len > 0xe666)
            collisionMode = 2;
    }

    FixVecScale(&scaledImpulse, pImpulse, 0x28000);
    len = FixMul(len, 0x28000);
    if (len > 0x34ccc) {
        FixVecScaleRecip(&scaledImpulse, &scaledImpulse, len);
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

    g_collisionCar->field_0xc00 = 1;
    g_collisionCar->field_0x96c = 0x10000;
    g_collisionCar->field_0xc04[0] = 1;

apply_torque:
    if ((torque.x < 0 ? -torque.x : torque.x) > 0x80000)
        torque.x = torque.x > 0 ? 0x80000 : -0x80000;
    if ((torque.y < 0 ? -torque.y : torque.y) > 0x80000)
        torque.y = torque.y > 0 ? 0x80000 : -0x80000;
    if ((torque.z < 0 ? -torque.z : torque.z) > 0x80000)
        torque.z = torque.z > 0 ? 0x80000 : -0x80000;

    FixVecScale(&torque, &torque, g_physicsScale);
    g_collisionCar->field_0x5d0.x += torque.x;
    g_collisionCar->field_0x5d0.y += torque.y;
    g_collisionCar->field_0x5d0.z += torque.z;
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
// FUNCTION: CMR2 0x00490570
int VehiclePhysics_SelectParallelCollisionSide(void)
{
    int d = FixMul(g_unk0x005918d0, g_collisionDirection.x) + FixMul(g_unk0x0059195c, g_collisionDirection.z);
    FixVector delta;

    if (d < 0)
        d = -d;
    if (d > g_unk0x005919b8)
        return 0;
    {
        delta.x = g_collisionTarget.x - g_collisionCar->positionPrev.x;
        delta.y = g_collisionTarget.y - g_collisionCar->positionPrev.y;
        delta.z = g_collisionTarget.z - g_collisionCar->positionPrev.z;
        g_collisionSelectBackSide = FixVecDot(&g_collisionDirection, &delta) >= 0;
        return 1;
    }
}

// Accepts the collision when the target is behind the face and the direction
// is nearly parallel to it.
// FUNCTION: CMR2 0x00490640
int VehiclePhysics_TestBehindFaceCollision(void)
{
    BYTE *pCar = (BYTE *)g_collisionCar;
    FixVector delta;
    int side;
    int d;

    delta.x = g_collisionTarget.x - *(int *)(pCar + 0x2e8);
    delta.y = g_collisionTarget.y - *(int *)(pCar + 0x2ec);
    delta.z = g_collisionTarget.z - *(int *)(pCar + 0x2f0);
    side = FixVecDot(&g_collisionDirection, &delta);
    if (side > 0)
        return 0;
    d = FixMul(g_unk0x005918d0, g_collisionDirection.x) + FixMul(g_unk0x0059195c, g_collisionDirection.z);
    if (d < 0)
        d = -d;
    if (d > g_unk0x005919b8)
        return 0;
    g_collisionSelectBackSide = side >= 0;
    return 1;
}

// Classifies face vertices by signed distance from the active collision plane.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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
        if (*pDistance >= 0)
            g_collisionPositiveCandidates[g_collisionPositiveCandidateCount++] = (BYTE)vertexIndex;
        else
            g_collisionNegativeCandidates[g_collisionNegativeCandidateCount++] = (BYTE)vertexIndex;

nextVertex:
        vertexOffset += sizeof(FixVector);
        vertexIndex++;
        pDistance++;
    } while ((int)pDistance < (int)(g_collisionVertexDistances + 4)); // 0x5919b4 in the original
}

// Updates the vehicle-local motion vector and the resulting positional correction.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00482ac0
void Vehicle_UpdateMotion(FixVector *pInput)
{
    FixVector localInput;
    FixVector delta;
    FixVector rotatedDelta;
    FixVector direction;
    int projection;
    int length;

    FixMatrix_InverseRotateVector(&localInput, pInput, g_partCar->pWorld);
    FixVecScale(&g_partState->velocity, &localInput, 0x1999);

    if ((g_partState->flags & 0xf0) == 0) {
        if (g_partState->velocity.y < 0)
            g_partState->velocity.y = 0;

        direction.x = FixSin((unsigned short)g_partState->angle);
        direction.y = FixSin((unsigned short)g_partState->angle + 0x400);
        direction.z = 0;
        projection = FixVecDot(&g_partState->velocity, &direction);
        if (projection < 0) {
            length = FixVecLength(&g_partState->velocity);
            FixVecScale(&direction, &direction, projection);
            g_partState->velocity.x -= direction.x;
            g_partState->velocity.y -= direction.y;
            g_partState->velocity.z -= direction.z;

            FixVector *pVelocity = &g_partState->velocity;
            int normalizedLength = FixVecLength(pVelocity);
            if (normalizedLength == 0) {
                pVelocity->x = 0;
                pVelocity->y = 0;
                pVelocity->z = 0;
            } else {
                FixVecScaleRecip(pVelocity, pVelocity, normalizedLength);
            }

            length = FixMul(length, 0x50000);
            FixVecScale(&g_partState->velocity,
                        &g_partState->velocity, length);
        }
    }

    delta.x = g_partState->position.x - g_partState->previousPosition.x;
    delta.y = g_partState->position.y - g_partState->previousPosition.y;
    delta.z = g_partState->position.z - g_partState->previousPosition.z;
    FixMatrix_RotateVector(&rotatedDelta, &delta, g_partState->pWorld);
    rotatedDelta.x += g_partState->previousPosition.x;
    rotatedDelta.y += g_partState->previousPosition.y;
    rotatedDelta.z += g_partState->previousPosition.z;
    g_partState->correction.x = g_partState->position.x - rotatedDelta.x;
    g_partState->correction.y = g_partState->position.y - rotatedDelta.y;
    g_partState->correction.z = g_partState->position.z - rotatedDelta.z;
    g_partState->position = g_partState->previousPosition;
    g_partState->flags |= 4;
    g_partState->flags &= (BYTE)~2;
}

void StageObject_QueueViewLensFlare(int *pObj, int *param2, int *param3, FixVector *pVerts);

// Selects a candidate face, transforms and classifies its vertices, then uses
// candidate and side counts to decide whether it participates in car collision.
// Byte-exact against the original (fastcmp EXACT).
// FUNCTION: CMR2 0x00490b90
int VehiclePhysics_ClassifyCandidateFace(int param_1)
{
    int result = 0;
    int found;

    if (param_1 != 0)
        found = VehiclePhysics_SelectParallelCollisionSide();
    else
        found = VehiclePhysics_TestBehindFaceCollision();
    if (found != 0) {
        StageObject_QueueViewLensFlare((int *)g_collisionFace, &g_collisionCar->right.x,
                     &g_collisionCar->position.x,
                     &g_collisionCar->corners[0]);
        Collision_ClassifyFaceVertices();
        result = 0;
        if (g_collisionPositiveCandidateCount > 0 && g_collisionNegativeCandidateCount > 0)
            result = 1;
        else if (g_collisionPositiveCandidateCount > 0 || g_collisionNegativeCandidateCount > 0) {
            if (g_collisionPositiveVertexCount != 4 || g_collisionSelectBackSide != 0) {
                if (g_collisionNegativeVertexCount == 4 && g_collisionSelectBackSide != 0)
                    result = 1;
            }
        }
    }
    return result;
}

void StageTiming_ClearPartFlagAndReselectIndex(void);

// Tests whether the length of the motion-state correction vector exceeds the
// current ground offset; when it does, latches the collision and reports it.
// FUNCTION: CMR2 0x00482f30
int VehiclePhysics_TestMotionCorrectionCollision(void)
{
    FixVector *pV = &g_partState->correction;

    if (FixVecLength(pV) >
        g_partCar->field_0x758 + g_partState->field_0x15c) {
        StageTiming_ClearPartFlagAndReselectIndex();
        return 1;
    }
    return 0;
}

void StageObject_ApplyRecursiveFrameDelta(BYTE index, char other, int *pDelta, int flag);
int Collision_TestSectorEdgeEndpoints(char type);
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);

extern int *g_unk0x0059190c;
extern FixVector g_unk0x005918e0;
extern int g_unk0x00591930;
extern FixVector g_unk0x00591938;
extern BYTE g_unk0x0059199c;
extern int g_unk0x005919a0;
extern FixVector g_unk0x00591ad0;
extern int g_unk0x00591948;
extern int g_unk0x0051fb00[52];

// Resolves the car's contact with the sector edge tracked in g_unk0x0059190c:
// picks the contact vertex of the face tracked in g_collisionFace, slides the
// car, its eight corners and the face's four vertices out of the surface,
// applies the impact impulse and spawns the impact debris.
// FUNCTION: CMR2 0x0048fb80
int Collision_ResolveSectorEdgeContact(char type, int param)
{
    FixVector debrisAxes[3];
    FixVector perp;
    FixVector delta;
    FixVector velDiff;
    FixVector scaled;
    FixVector slide;
    int best;
    int count;
    int found;
    int index;
    int len;
    int tangentLen;
    int i;
    int result;
    int dot;
    int *pDistance;

    result = 0;
    if ((*(BYTE *)((BYTE *)g_unk0x0059190c + 0x2d) & 1) != 0)
        found = VehiclePhysics_SelectParallelCollisionSide();
    else
        found = VehiclePhysics_TestBehindFaceCollision();
    if (found == 0)
        goto finish;

    if ((*(BYTE *)((BYTE *)g_unk0x0059190c + 0x2d) & 1) == 0)
        result = Collision_TestSectorEdgeEndpoints(type);

    StageObject_QueueViewLensFlare((int *)g_collisionFace, &g_collisionCar->right.x,
                 &g_collisionCar->position.x,
                 &g_collisionCar->corners[0]);
    Collision_ClassifyFaceVertices();

    if (g_collisionPositiveCandidateCount != 0 && g_collisionNegativeCandidateCount != 0) {
        if (g_collisionSelectBackSide == 0) {
            best = 0xd8f00000;
            count = g_collisionPositiveCandidateCount;
            for (i = 0; i < count; i++) {
                index = g_collisionPositiveCandidates[i];
                if (g_collisionVertexDistances[index] > best) {
                    best = g_collisionVertexDistances[index];
                    g_collisionBestVertex = index;
                    g_unk0x00591948 = (int)g_unk0x0059190c;
                }
            }
        } else {
            best = 0x27100000;
            count = g_collisionNegativeCandidateCount;
            for (i = 0; i < count; i++) {
                index = g_collisionNegativeCandidates[i];
                if (g_collisionVertexDistances[index] < best) {
                    best = g_collisionVertexDistances[index];
                    g_collisionBestVertex = index;
                    g_unk0x00591948 = (int)g_unk0x0059190c;
                }
            }
        }
    } else {
        if (g_collisionPositiveCandidateCount == 0 && g_collisionNegativeCandidateCount == 0)
            goto finish;
        if (g_collisionPositiveVertexCount == 4 && g_collisionSelectBackSide == 0) {
            best = 0xd8f00000;
            i = 0;
            pDistance = g_collisionVertexDistances;
            do {
                if (*pDistance > best) {
                    best = *pDistance;
                    g_collisionBestVertex = i;
                    g_unk0x00591948 = (int)g_unk0x0059190c;
                }
                i++;
                pDistance++;
            } while ((int)pDistance < (int)(g_collisionVertexDistances + 4)); // 0x5919b4 in the original
        } else if (g_collisionNegativeVertexCount == 4 && g_collisionSelectBackSide != 0) {
            best = 0x27100000;
            i = 0;
            pDistance = g_collisionVertexDistances;
            do {
                if (*pDistance < best) {
                    best = *pDistance;
                    g_collisionBestVertex = i;
                    g_unk0x00591948 = (int)g_unk0x0059190c;
                }
                i++;
                pDistance++;
            } while ((int)pDistance < (int)(g_collisionVertexDistances + 4)); // 0x5919b4 in the original
        } else {
            goto finish;
        }
    }

    if (best == 0)
        goto noSlide;
    if (g_collisionSelectBackSide == 0) {
        if (best <= 0)
            goto noSlide;
    } else if (best >= 0) {
        goto noSlide;
    }
    {
        FixVecScale(&scaled, &g_collisionDirection, best);
        g_collisionCar->position.x += scaled.x;
        g_collisionCar->position.y += scaled.y;
        g_collisionCar->position.z += scaled.z;
        for (i = 0; i < 8; i++) {
            g_collisionCar->corners[i].x += scaled.x;
            g_collisionCar->corners[i].y += scaled.y;
            g_collisionCar->corners[i].z += scaled.z;
        }
        for (i = 0; i < 4; i++) {
            g_collisionFace->primaryVertices[i].x += scaled.x;
            g_collisionFace->primaryVertices[i].y += scaled.y;
            g_collisionFace->primaryVertices[i].z += scaled.z;
        }
        StageObject_ApplyRecursiveFrameDelta(*(BYTE *)((BYTE *)g_collisionCar + 0xb1a), -1, (int *)&scaled, 0);
    }

noSlide:
    dot = FixVecDot(&g_collisionCar->velocity, &g_collisionDirection);
    if (dot != 0) {
        if (g_collisionSelectBackSide == 0) {
            if (dot >= 0)
                goto skipReflect;
        } else if (dot <= 0) {
            goto skipReflect;
        }

        FixVecScale(&slide, &g_collisionDirection, dot);
        velDiff.x = g_collisionCar->velocity.x - slide.x;
        velDiff.y = g_collisionCar->velocity.y - slide.y;
        velDiff.z = g_collisionCar->velocity.z - slide.z;
        FixVecScale(&velDiff, &velDiff,
                    -FixMul(g_physicsTimeStep, g_unk0x0051fb00[26 + type]));
        FixVecScale(&perp, &slide, -g_unk0x0051fb00[type]);
        slide.x = perp.x - slide.x + velDiff.x;
        slide.y = perp.y - slide.y + velDiff.y;
        slide.z = perp.z - slide.z + velDiff.z;
        CarPhysics_ApplyImpulse(&slide,
                                &(&g_collisionFace->primaryVertices[0])[g_collisionBestVertex], 0);
        len = FixVecLength(&slide);
        if (g_unk0x00591930 != 0 && g_unk0x005919a0 >= len)
            goto skipReflect;
        g_unk0x005919a0 = len;
        g_unk0x005918e0 = slide;
        g_unk0x00591ad0 = g_collisionTarget;
        g_unk0x00591938 = g_collisionDirection;
        g_unk0x00591930 = 1;
        g_unk0x0059199c = 0;
    }

skipReflect:
    dot = FixVecDot(&g_collisionCar->velocity, &g_collisionDirection);
    FixVecScale(&debrisAxes[0], &g_collisionDirection, dot);
    debrisAxes[0].x = g_collisionCar->velocity.x - debrisAxes[0].x;
    debrisAxes[0].y = g_collisionCar->velocity.y - debrisAxes[0].y;
    debrisAxes[0].z = g_collisionCar->velocity.z - debrisAxes[0].z;
    if (type != 2 && type != 0xe && type != 0xf && type != 0x12)
        return 1;

    len = FixVecLength(&debrisAxes[0]);
    if (len <= 0x8000)
        return 1;
    tangentLen = len;
    FixVecScaleRecip(&debrisAxes[0], &debrisAxes[0], -len);
    FixVecCross(&debrisAxes[1], &debrisAxes[0], &g_collisionDirection);
    len = FixVecLength(&debrisAxes[1]);
    if (len == 0) {
        debrisAxes[1].x = 0;
        debrisAxes[1].y = 0;
        debrisAxes[1].z = 0;
    } else {
        FixVecScaleRecip(&debrisAxes[1], &debrisAxes[1], len);
    }

    debrisAxes[2] = g_collisionDirection;
    delta.x = (&g_collisionFace->primaryVertices[0])[g_collisionBestVertex].x -
              g_collisionCar->position.x;
    delta.y = (&g_collisionFace->primaryVertices[0])[g_collisionBestVertex].y -
              g_collisionCar->position.y;
    delta.z = (&g_collisionFace->primaryVertices[0])[g_collisionBestVertex].z -
              g_collisionCar->position.z;
    delta.y = 0;
    FixVecScale(&delta, &delta, 0xcccc);
    if (tangentLen > 0x10000)
        tangentLen = 0x10000;
    if (g_collisionCar->field_0xb70 == 0)
        Car_SpawnDebris(tangentLen, &delta, g_collisionCar, &debrisAxes[0], 0x90000, 0x6666);
    return 1;
finish:
    return result;
}
