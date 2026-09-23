#include <windows.h>
#include "CarPhysics.h"
#include "Car.h"
#include "GameInfo.h"
#include "RallyData.h"

// Wheel / ground contact of the cars (0x494b50-0x498570). Every frame the
// car being simulated is loaded into the g_phys* globals (its transform, axes
// and wheel matrices); the contact patches, ground heights and grip of its
// wheels are kept in its CarContact record.

extern void *g_unk0x00592734;   // CarContact[g_unk0x00592738]
extern int g_unk0x00592738;

int FUN_00457e10(BYTE *pCar, int offset);
int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY);
int RallyData_FUN_00411060(void);
int FUN_0042cae0(Car *pCar, int param2);

#define CAR_CONTACT(i) ((CarContact *)g_unk0x00592734 + (i))

// Per-car pointers into the car's handling data (see FUN_00494bb0).
// GLOBAL: CMR2 0x00592278
BYTE *g_physSkidCount[8];
// GLOBAL: CMR2 0x00592298
int *g_physGroundPoint;
// GLOBAL: CMR2 0x0059229c
int *g_physWheelLength[8];
// GLOBAL: CMR2 0x005922bc
BYTE *g_physSkidRange[8];
// GLOBAL: CMR2 0x005922e0
FixVector g_physGroundPos;
// GLOBAL: CMR2 0x005922ec
CarTransforms *g_physBody;
// GLOBAL: CMR2 0x005922f0
FixVector g_physPos;
// GLOBAL: CMR2 0x005922fc
int *g_physSkidWidth[8];
// GLOBAL: CMR2 0x00592320
FixVector g_physRight;
// GLOBAL: CMR2 0x0059232c
FixVector g_physUp;
// GLOBAL: CMR2 0x00592338
FixVector g_physForward;
// GLOBAL: CMR2 0x00592344
int *g_physSkidOffset[8];
// GLOBAL: CMR2 0x00592468
FixVector g_physTrackDir;
// GLOBAL: CMR2 0x00592474
FixVector g_physTrackSide;
// GLOBAL: CMR2 0x00592480
int g_physTrackAngle;
// GLOBAL: CMR2 0x00592484
int g_physTrackSlope;
// GLOBAL: CMR2 0x00592488
FixMatrix *g_physWheels;

// Resets the contact records for a new stage and caches the pointers into
// every car's handling data. In the time trial modes only the first car
// (the player) is driven, the others are ghosts.
// TODO: CMR2 0x00494bb0 (implemented, match 42%)
void FUN_00494bb0(void)
{
    int i;
    int j;
    int data;

    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 || CGameInfo::FUN_00405d80() == 7) {
        for (i = 0; i < g_unk0x00592738; i++) {
            CAR_CONTACT(i)->field_0x294 = 1;
            CAR_CONTACT(i)->field_0x298 = 1;
            if (i == 0) {
                CAR_CONTACT(0)->field_0x294 = 0;
                if ((BYTE)RallyDataState() == 1)
                    CAR_CONTACT(0)->field_0x298 = 1;
            }
            CAR_CONTACT(i)->field_0x250 = 0x9999;
            for (j = 0; j < 4; j++)
                CAR_CONTACT(i)->wheelGrip[j] = 0x10000;
        }
    } else {
        for (i = 0; i < g_unk0x00592738; i++) {
            CAR_CONTACT(i)->field_0x294 = 0;
            CAR_CONTACT(i)->field_0x298 = 1;
            CAR_CONTACT(i)->field_0x250 = 0x9999;
            for (j = 0; j < 4; j++)
                CAR_CONTACT(i)->wheelGrip[j] = 0x10000;
        }
    }
    for (i = 0; i < g_unk0x00592738; i++) {
        data = FUN_00457e10((BYTE *)Car_Get(i), 1);
        g_physSkidCount[i] = (BYTE *)data;
        g_physSkidRange[i] = (BYTE *)(data + 1);
        g_physSkidOffset[i] = (int *)(data + 4);
        g_physWheelLength[i] = (int *)(data + 8);
        g_physSkidWidth[i] = (int *)(data + 0xc);
    }
}

// Whether skid point index (counted from the front, or from the back when
// field_0x2a0 is clear) falls in the car's visible skid range.
// FUNCTION: CMR2 0x00494d40
int FUN_00494d40(Car *pCar, CarContact *pContact, int index)
{
    if (pContact->field_0x29c != 0)
        index--;
    if (pContact->field_0x2a0 == 0)
        index = (*g_physSkidCount[pCar->field_0xb1a] - index) - 1;
    index -= *g_physSkidRange[pCar->field_0xb1a];
    if (index >= 0 && index < g_physSkidRange[pCar->field_0xb1a][1])
        return 1;
    return 0;
}

// Updates the contact patch of every wheel: its position under the car
// (dropped onto the ground), the grip left by the suspension travel and the
// four corners of the patch, flattened onto the ground plane.
// TODO: CMR2 0x00497db0 (implemented, match 59%)
void FUN_00497db0(Car *pCar)
{
    CarContact *pContact;
    FixVector offset;
    FixVector down;
    FixVector local;
    FixVector pos;
    FixVector fwd;
    FixVector side;
    FixVector d;
    FixVector *pCorner;
    int blend;
    int wheel;
    int travel;
    int length;
    int t;
    int i;
    short surfaceClass;

    pContact = CAR_CONTACT(pCar->field_0xb1a);
    if (pCar->field_0xb64 == 0) {
        blend = FixVecDot(&g_physUp, &g_physBody->groundNormal) - 0xcccc;
        if (blend < 0)
            blend = 0;
        blend = FixMul(blend, 0x50000);
        if (blend > 0x10000)
            blend = 0x10000;
    } else {
        blend = 0x10000;
    }
    FixVecScale(&offset, &g_physUp, -0x5999);
    if (CGameInfo::FUN_004063f0(6))
        FixVecScale(&offset, &offset, 0x28000);
    for (wheel = 0; wheel < 4; wheel++) {
        if (*(int *)((BYTE *)pCar->pWheelNodes[wheel] + 8) == RallyData_FUN_00411060())
            continue;
        FixMatrix_GetPosition(&local, &g_physWheels[wheel]);
        FixMatrix_RotateVector(&pos, &local, &g_physBody->body);
        pos.x += g_physPos.x + offset.x;
        pos.y += g_physPos.y + offset.y;
        pos.z += g_physPos.z + offset.z;
        if (pCar->field_0xb64 == 0) {
            down.x = 0;
            down.y = 0x10000;
            down.z = 0;
            t = Track_GetGroundHeightSurface(&pos, &down, &pContact->wheelTriangle[wheel], &surfaceClass,
                                             (unsigned short *)&surfaceClass, pContact->wheelGroundY[wheel]);
            pContact->wheelGroundY[wheel] = t;
            travel = pos.y - t;
            pos.y = t;
            if (travel >= 1) {
                t = FixMul(travel, 0x40000);
                if (t > 0x10000)
                    t = 0x10000;
                pContact->wheelGrip[wheel] = 0x10000 - t;
            } else {
                pContact->wheelGrip[wheel] = 0x10000;
            }
        } else {
            pContact->wheelGrip[wheel] = 0x10000;
        }
        pContact->wheelGrip[wheel] = FixMul(blend, pContact->wheelGrip[wheel]);
        FixMatrix_GetForward(&local, &g_physWheels[wheel]);
        FixMatrix_RotateVector(&fwd, &local, &g_physBody->body);
        FixVecCross(&side, &g_physUp, &fwd);
        FixVecScale(&side, &side, 0x5999);
        length = *g_physWheelLength[pCar->field_0xb1a];
        FixVecScale(&fwd, &fwd, length);
        if (FUN_0042cae0(pCar, 1))
            FixVecScale(&fwd, &fwd, 0x9999);
        pCorner = pContact->wheelCorners[wheel];
        pCorner[0].x = side.x + pos.x;
        pCorner[0].y = side.y + pos.y;
        pCorner[0].z = side.z + pos.z;
        pCorner[2].x = pos.x - side.x;
        pCorner[2].y = pos.y - side.y;
        pCorner[2].z = pos.z - side.z;
        pCorner[1].x = pCorner[0].x - fwd.x;
        pCorner[1].y = pCorner[0].y - fwd.y;
        pCorner[1].z = pCorner[0].z - fwd.z;
        pCorner[0].x += fwd.x;
        pCorner[0].y += fwd.y;
        pCorner[0].z += fwd.z;
        pCorner[3].x = pCorner[2].x - fwd.x;
        pCorner[3].y = pCorner[2].y - fwd.y;
        pCorner[3].z = pCorner[2].z - fwd.z;
        pCorner[2].x = fwd.x + pCorner[2].x;
        pCorner[2].y = fwd.y + pCorner[2].y;
        pCorner[2].z = pCorner[2].z + fwd.z;
        if (pCar->field_0xb64 == 0) {
            for (i = 0; i < 4; i++) {
                d.x = pCorner[i].x - pos.x;
                d.y = pCorner[i].y - pos.y;
                d.z = pCorner[i].z - pos.z;
                t = FixVecDot(&down, &d);
                FixVecScale(&side, &down, t);
                d.x -= side.x;
                d.y -= side.y;
                d.z -= side.z;
                pCorner[i].x = d.x + pos.x;
                pCorner[i].y = d.y + pos.y;
                pCorner[i].z = d.z + pos.z;
            }
        }
    }
}
