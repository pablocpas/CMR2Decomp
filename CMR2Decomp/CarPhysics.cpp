#include <windows.h>
#include "CarPhysics.h"
#include "Car.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "StageTiming.h"
#include "Graphics.h"
#include "Sprite.h"

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
void Scene_GetShadowColour(DWORD *pColour, int *pLevel);
void FUN_00495f50(int view, CarContact *pContact);
CarTransforms *FUN_0042ca70(int index);
FixMatrix *FUN_0042cab0(int index);
BYTE *FUN_0042cac0(int index);
void FUN_00497db0(Car *pCar);
void FUN_004ae140(BYTE *pColour);
void Graphics_SetTextureFactorAlpha(BYTE *pColour);
void FUN_004962c0(Car *pCar, CarContact *pContact);
int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri, unsigned short *pSurface, int defaultY);

#define CAR_CONTACT(i) ((CarContact *)g_unk0x00592734 + (i))

// Per-car pointers into the car's handling data (see FUN_00494bb0).
// GLOBAL: CMR2 0x00592278
BYTE *g_physSkidCount[8];
// GLOBAL: CMR2 0x00592298
FixVector *g_physGroundPoint;   // the car's 8 hull points
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
FixVector g_physPatchDir;
// GLOBAL: CMR2 0x00592474
FixVector g_physPatchSide;
// GLOBAL: CMR2 0x00592480
int g_physPatchLength;
// GLOBAL: CMR2 0x00592484
int g_physPatchWidth;
// GLOBAL: CMR2 0x00592488
FixMatrix *g_physWheels;
// Copy of the contact record being drawn, pulled towards the camera.
// GLOBAL: CMR2 0x00592490
CarContact g_physContactView;

// Scale of the body patch edges (the first two, then the last two) by the
// stage type of the car's timing record.
// GLOBAL: CMR2 0x00520150
int g_physPatchScaleA[14] = {0x10000, 0x1072b, 0xfa5e, 0x1076c, 0xf74b, 0xfd70, 0xefdf,
                             0x112f1, 0xb916,  0xe5a1, 0xe418,  0xebc6, 0xf1eb, 0xf1eb};
// GLOBAL: CMR2 0x00520188
int g_physPatchScaleB[14] = {0x10000, 0x10a3d, 0x10000, 0x10624, 0x10000, 0x108b4, 0xfe76,
                             0xfe76,  0xd8d4,  0x10f5c, 0x105e3, 0xfeb8,  0xfe76,  0xfe76};
// Pairs of opposite body patch corners.
// GLOBAL: CMR2 0x005201c0
BYTE g_physPatchEdges[5][2] = {{1, 2}, {0, 3}, {1, 0}, {2, 3}, {0, 0}};
// Reference axis of the skid trail and its length scale.
// GLOBAL: CMR2 0x005201c8
FixVector g_physTrailAxis = {0x10000, 0, 0};
// GLOBAL: CMR2 0x005201d4
int g_physTrailScale = 0x10000;

// Resets the contact records for a new stage and caches the pointers into
// every car's handling data. In the time trial modes only the first car
// (the player) is driven, the others are ghosts.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00494bb0
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
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00497db0
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
                t = FixMul(FixDiv(0x10000, 0x4000), travel);
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

// Builds the contact patch under the body: a rectangle on the ground plane
// spanning the car's hull points (or taken from the car's box when the
// ground normal is unknown), or, for ghost cars, the hull points themselves
// rescaled for the stage.
// match 62%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004962c0
void FUN_004962c0(Car *pCar, CarContact *pContact)
{
    BYTE stage;
    FixVector *pNormal;
    FixVector d;
    FixVector mid;
    FixVector fwd;
    FixVector side;
    FixVector rel;
    int t;
    int u;
    int i;
    BYTE *pEdge;
    FixVector *pA;
    FixVector *pB;

    stage = *FUN_00456be0(pCar->field_0xb1a);
    g_physPatchWidth = 0;
    g_physPatchLength = 0;
    if (pContact->field_0x294 == 0 || pCar->field_0xc00 != 0) {
        if (pCar->field_0xb64 == 0) {
            pNormal = &g_physBody->groundNormal;
            t = FixVecDot(&g_physRight, pNormal);
            if (FIX_ABS(t) < 0xfd71) {
                FixVecScale(&d, pNormal, t);
                d.x = g_physRight.x - d.x;
                d.y = g_physRight.y - d.y;
                d.z = g_physRight.z - d.z;
                FIX_NORMALIZE_INTO(g_physPatchDir, d);
                FixVecCross(&g_physPatchSide, &g_physPatchDir, &g_physBody->groundNormal);
            } else {
                t = FixVecDot(&g_physUp, pNormal);
                FixVecScale(&d, pNormal, t);
                d.x = g_physUp.x - d.x;
                d.y = g_physUp.y - d.y;
                d.z = g_physUp.z - d.z;
                FIX_NORMALIZE_INTO(g_physPatchDir, d);
                FixVecCross(&g_physPatchSide, &g_physPatchDir, &g_physBody->groundNormal);
            }
            for (i = 0; i < 8; i++) {
                rel.x = g_physGroundPoint[i].x - g_physPos.x;
                rel.y = g_physGroundPoint[i].y - g_physPos.y;
                rel.z = g_physGroundPoint[i].z - g_physPos.z;
                t = FixVecDot(&g_physPatchDir, &rel);
                u = FixVecDot(&g_physPatchSide, &rel);
                if (t > 0 && t > g_physPatchLength)
                    g_physPatchLength = t;
                if (u > 0 && u > g_physPatchWidth)
                    g_physPatchWidth = u;
            }
            FixVecScale(&fwd, &g_physPatchDir, g_physPatchLength);
            FixVecScale(&side, &g_physPatchSide, g_physPatchWidth);
            pContact->points[1].x = side.x + fwd.x;
            pContact->points[0].x = fwd.x - side.x;
            pContact->points[1].y = side.y + fwd.y;
            pContact->points[0].y = fwd.y - side.y;
            pContact->points[1].z = side.z + fwd.z;
            pContact->points[0].z = fwd.z - side.z;
            FixVecScale(&fwd, &fwd, -0x10000);
            pContact->points[2].x = side.x + fwd.x;
            pContact->points[3].x = fwd.x - side.x;
            pContact->points[2].y = side.y + fwd.y;
            pContact->points[3].y = fwd.y - side.y;
            pContact->points[2].z = side.z + fwd.z;
            pContact->points[3].z = fwd.z - side.z;
            for (i = 0; i < 4; i++) {
                pContact->points[i].x += g_physGroundPos.x;
                pContact->points[i].y += g_physGroundPos.y;
                pContact->points[i].z += g_physGroundPos.z;
            }
            return;
        }
        g_physPatchDir = pCar->right;
        g_physPatchSide = pCar->forward;
        g_physPatchLength = pCar->halfExtents.x;
        g_physPatchWidth = pCar->halfExtents.z;
        rel.x = g_physGroundPos.x - pCar->position.x;
        rel.y = g_physGroundPos.y - pCar->position.y;
        rel.z = g_physGroundPos.z - pCar->position.z;
        FixVecScale(&d, &pCar->up, pCar->halfExtents.y);
        for (i = 0; i < 4; i++) {
            pContact->points[i].x = pCar->corners[i].x + rel.x + d.x;
            pContact->points[i].y = rel.y + d.y + pCar->corners[i].y;
            pContact->points[i].z = pCar->corners[i].z + rel.z + d.z;
        }
        d = pContact->points[2];
        pContact->points[2] = pContact->points[3];
        pContact->points[3] = d;
        return;
    }
    pContact->points[1] = g_physGroundPoint[0];
    pContact->points[0] = g_physGroundPoint[1];
    pContact->points[3] = g_physGroundPoint[3];
    pContact->points[2] = g_physGroundPoint[2];
    if (pCar->field_0xb64 == 0) {
        pContact->points[1].y = g_physBody->cornerHeight[0];
        pContact->points[0].y = g_physBody->cornerHeight[1];
        pContact->points[3].y = g_physBody->cornerHeight[3];
        pContact->points[2].y = g_physBody->cornerHeight[2];
    }
    if (stage == (char)pCar->field_0xb1b[0])
        return;
    for (pEdge = g_physPatchEdges[0]; pEdge < g_physPatchEdges[4]; pEdge += 2) {
        pA = &pContact->points[pEdge[0]];
        pB = &pContact->points[pEdge[1]];
        d.x = pA->x - pB->x;
        d.y = pA->y - pB->y;
        d.z = pA->z - pB->z;
        FixVecScale(&d, &d, 0x8000);
        mid.x = pB->x + d.x;
        mid.y = pB->y + d.y;
        mid.z = pB->z + d.z;
        if (pEdge < g_physPatchEdges[2])
            FixVecScale(&d, &d, g_physPatchScaleA[stage]);
        else
            FixVecScale(&d, &d, g_physPatchScaleB[stage]);
        pA->x = d.x + mid.x;
        pA->y = d.y + mid.y;
        pA->z = d.z + mid.z;
        pB->x = mid.x - d.x;
        pB->y = mid.y - d.y;
        pB->z = mid.z - d.z;
    }
}

#define FIX_MIDPOINT(out, a, b) \
    out.x = (a).x - (b).x;      \
    out.y = (a).y - (b).y;      \
    out.z = (a).z - (b).z;      \
    FixVecScale(&out, &out, 0x8000); \
    out.x += (b).x;             \
    out.y += (b).y;             \
    out.z += (b).z

#define SHADOW_VERTEX(v, p, c) \
    v.x = (p).x;               \
    v.y = (p).y;               \
    v.z = (p).z;               \
    *(DWORD *)v.colour = *(DWORD *)(c)

#define SHADOW_TRIANGLE(p0, c0, p1, c1, p2, c2) \
    SHADOW_VERTEX(v0, p0, c0);                  \
    SHADOW_VERTEX(v1, p1, c1);                  \
    SHADOW_VERTEX(v2, p2, c2);                  \
    Quad2D_QueueFixedTriangle(0, &v0, &v1, &v2, NULL, (Quad2D *)10)

// Octagon around the body patch (its corners cut at 5% / 95% of each edge)
// and the solid core, 60% of its size.
#define SHADOW_OCTAGON(pBody, pRing, pCore, centre)                        \
    for (i = 0; i < 4; i++) {                                              \
        d.x = pBody[(i + 1) % 4].x - pBody[i].x;                           \
        d.y = pBody[(i + 1) % 4].y - pBody[i].y;                           \
        d.z = pBody[(i + 1) % 4].z - pBody[i].z;                           \
        FixVecScale(&e, &d, 0xf333);                                       \
        FixVecScale(&d, &d, 0xccc);                                        \
        pRing[i * 2 + 1].x = pBody[i].x + d.x;                             \
        pRing[i * 2 + 1].y = d.y + pBody[i].y;                             \
        pRing[i * 2 + 1].z = d.z + pBody[i].z;                             \
        pRing[(i * 2 + 2) % 8].x = pBody[i].x + e.x;                       \
        pRing[(i * 2 + 2) % 8].y = pBody[i].y + e.y;                       \
        pRing[(i * 2 + 2) % 8].z = pBody[i].z + e.z;                       \
    }                                                                      \
    for (i = 0; i < 8; i++) {                                              \
        d.x = pRing[i].x - centre.x;                                       \
        d.y = pRing[i].y - centre.y;                                       \
        d.z = pRing[i].z - centre.z;                                       \
        FixVecScale(&d, &d, 0x9999);                                       \
        pCore[i].x = centre.x + d.x;                                       \
        pCore[i].y = centre.y + d.y;                                       \
        pCore[i].z = centre.z + d.z;                                       \
    }

// Draws the shadow of the car: a soft octagon under the body, a patch under
// every wheel on the ground and, for the driven car, a smear along its skid
// trail. Each is a solid core fading out to a transparent rim.
// match 33%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00494db0
void FUN_00494db0(Car *pCar, int view)
{
    Quad2DInputVertex v0;
    Quad2DInputVertex v1;
    Quad2DInputVertex v2;
    FixVector ring[18];
    FixVector outline[18];
    FixVector e;
    FixVector centre;
    FixVector d;
    FixVector *pSrc;
    BYTE colour[4];
    BYTE clear[4];
    int level;
    int alpha;
    int count;
    BOOL all;
    int i;
    int j;
    int k;
    int t;
    int total;
    short n;

    colour[0] = 0;
    colour[1] = 0;
    colour[2] = 0;
    colour[3] = 0x32;
    count = *g_physSkidCount[pCar->field_0xb1a];
    all = TRUE;
    if (*(int *)((BYTE *)FUN_00469680(pCar->field_0xb1a) + 0x4bc) != 0) {
        all = FALSE;
        count -= g_physSkidRange[pCar->field_0xb1a][1];
    }
    Scene_GetShadowColour((DWORD *)colour, &level);
    colour[3] = 0x32;
    *(DWORD *)clear = *(DWORD *)colour;
    clear[3] = 0;
    g_physContactView = *CAR_CONTACT(pCar->field_0xb1a);
    FUN_00495f50(view, &g_physContactView);
    v0.u = 0;
    v0.v = 0;
    v1.u = 0;
    v1.v = 0;
    v2.u = 0;
    v2.v = 0;

    // Wheels.
    if (g_physContactView.field_0x298 != 0) {
        for (i = 0; i < 4; i++) {
            if (*(int *)((BYTE *)pCar->pWheelNodes[i] + 8) == RallyData_FUN_00411060() ||
                g_physContactView.wheelGrip[i] == 0)
                continue;
            alpha = FixMul(0xe60000, g_physContactView.wheelGrip[i]);
            pSrc = CAR_CONTACT(pCar->field_0xb1a)->wheelCorners[i];
            FIX_MIDPOINT(d, pSrc[0], pSrc[2]);
            FIX_MIDPOINT(centre, pSrc[1], pSrc[3]);
            CAR_CONTACT(pCar->field_0xb1a)->wheelFrontMid[i].x = d.x;
            CAR_CONTACT(pCar->field_0xb1a)->wheelFrontMid[i].y = d.y;
            CAR_CONTACT(pCar->field_0xb1a)->wheelFrontMid[i].z = d.z;
            CAR_CONTACT(pCar->field_0xb1a)->wheelRearMid[i].x = centre.x;
            CAR_CONTACT(pCar->field_0xb1a)->wheelRearMid[i].y = centre.y;
            CAR_CONTACT(pCar->field_0xb1a)->wheelRearMid[i].z = centre.z;
            FIX_MIDPOINT(d, g_physContactView.wheelCorners[i][0], g_physContactView.wheelCorners[i][2]);
            FIX_MIDPOINT(centre, g_physContactView.wheelCorners[i][1], g_physContactView.wheelCorners[i][3]);
            colour[3] = (BYTE)(alpha >> 16);
            SHADOW_TRIANGLE(g_physContactView.wheelCorners[i][0], clear, g_physContactView.wheelCorners[i][1], clear, d, colour);
            SHADOW_TRIANGLE(d, colour, g_physContactView.wheelCorners[i][1], clear, centre, colour);
            SHADOW_TRIANGLE(d, colour, centre, colour, g_physContactView.wheelCorners[i][2], clear);
            SHADOW_TRIANGLE(centre, colour, g_physContactView.wheelCorners[i][3], clear, g_physContactView.wheelCorners[i][2], clear);
        }
    }

    FIX_MIDPOINT(centre, g_physContactView.points[0], g_physContactView.points[2]);

    // Skid trail: the trailing edge of the body followed by the skid points.
    if (!(g_pGraphics->field913_0x3bc & 0x20) && g_physContactView.field_0x294 == 0) {
        k = g_physContactView.trailEdge;
        outline[0] = g_physContactView.points[k];
        outline[1] = g_physContactView.points[(k + 1) % 4];
        n = 2;
        for (i = 0; i < *g_physSkidCount[pCar->field_0xb1a] + 1; i++) {
            if (all || !FUN_00494d40(pCar, &g_physContactView, i)) {
                outline[n] = g_physContactView.points[4 + i];
                n++;
            }
        }
        outline[count + 3] = g_physContactView.points[(k + 3) % 4];
        outline[count + 4] = g_physContactView.points[(k + 2) % 4];
        t = 0xfd70 - FixMul(0x10000 - g_physContactView.field_0x250, 0x23d7);
        total = (short)(count + 4);
        for (i = 0; i < total + 1; i++) {
            d.x = outline[i].x - centre.x;
            d.y = outline[i].y - centre.y;
            d.z = outline[i].z - centre.z;
            FixVecScale(&d, &d, t);
            ring[i].x = centre.x + d.x;
            ring[i].y = d.y + centre.y;
            ring[i].z = d.z + centre.z;
        }
        colour[3] = FixMulShift32(level, pCar->field_0xa70);
        for (i = 1; i <= total; i++) {
            j = i % total;
            SHADOW_TRIANGLE(ring[i - 1], colour, centre, colour, ring[j], colour);
            SHADOW_TRIANGLE(outline[i - 1], clear, ring[i - 1], colour, outline[j], clear);
            SHADOW_TRIANGLE(outline[j], clear, ring[i - 1], colour, ring[j], colour);
        }
    }

    // Body: fades out as the car leaves the ground.
    if (g_physContactView.field_0x294 == 0) {
        SHADOW_OCTAGON(g_physContactView.points, ring, outline, centre);
        t = pCar->corners[0].y - pCar->cornerHeight[0];
        for (i = 1; i < 8; i++) {
            if (pCar->corners[i].y - pCar->cornerHeight[i] < t)
                t = pCar->corners[i].y - pCar->cornerHeight[i];
        }
        if (t < 1) {
            t = 0x10000;
        } else {
            t = FixMul(t, 0x20000);
            if (t > 0x10000)
                t = 0x10000;
            t = 0x10000 - t;
        }
        colour[3] = FixMulShift32(t, 0xff0000);
    } else {
        SHADOW_OCTAGON(g_physContactView.points, ring, outline, centre);
        colour[3] = 0xff;
    }
    for (i = 1; i <= 8; i++) {
        j = i % 8;
        SHADOW_TRIANGLE(outline[j], colour, outline[i - 1], colour, centre, colour);
        SHADOW_TRIANGLE(ring[i - 1], clear, outline[i - 1], colour, ring[j], clear);
        SHADOW_TRIANGLE(outline[i - 1], colour, outline[j], colour, ring[j], clear);
    }
}

// Pulls the shadow points one unit towards the camera so they do not sink
// into the ground: the body and skid points, and the wheel patches when drawn.
// FUNCTION: CMR2 0x00495f50
void FUN_00495f50(int view, CarContact *pContact)
{
    FixVector cam;
    FixVector d;
    FixVector *p;
    int len;
    int i;

    FixMatrix_GetPosition(&cam, &g_viewNodes[view]->current);
    for (i = 0; i < pContact->pointCount; i++) {
        p = &pContact->points[i];
        d.x = cam.x - p->x;
        d.y = cam.y - p->y;
        d.z = cam.z - p->z;
        len = FixVecLength(&d);
        if (len > 0x20000 && len != 0) {
            FixVecScaleRecip(&d, &d, len);
            FixVecScale(&d, &d, 0x10000);
            // The original adds the components out of order here (it picks the
            // base register of the point from the component touched last).
            p->x += d.x;
            p->z += d.z;
            p->y += d.y;
        }
    }
    if (pContact->field_0x298 != 0) {
        p = pContact->wheelCorners[0];
        for (i = 16; i != 0; i--) {
            d.x = cam.x - p->x;
            d.y = cam.y - p->y;
            d.z = cam.z - p->z;
            len = FixVecLength(&d);
            if (len > 0x20000 && len != 0) {
                FixVecScaleRecip(&d, &d, len);
                FixVecScale(&d, &d, 0x10000);
                p->x += d.x;
                p->y += d.y;
                p->z += d.z;
            }
            p++;
        }
    }
}

// Loads the car into the g_phys* globals, drops it onto the ground and
// updates its contact record: the wheel and body patches and, for a driven
// car, the skid trail laid out behind the body patch along the car's skid
// profile, bent round when the car is sliding sideways. Every point is then
// eased towards the ground height under it.
// match 62%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00496e00
void FUN_00496e00(Car *pCar)
{
    CarContact *pContact;
    int *pProfile;
    FixVector flat;
    FixVector p;
    FixVector v;
    FixVector w;
    FixVector *pNormal;
    int h;
    int scale;
    int cosA;
    int fwd;
    int side;
    int grip;
    int bend;
    int base;
    int lateral;
    int along;
    int firstAlong;
    int firstLat;
    int acc;
    int limit;
    int len;
    int a;
    int c;
    int k;
    int first;
    int extra;
    short angle;
    int skip;

    pContact = CAR_CONTACT(pCar->field_0xb1a);
    firstAlong = 0;
    firstLat = 0;
    acc = 0;
    side = 0;
    first = 0;
    pProfile = g_physSkidWidth[pCar->field_0xb1a];
    g_physBody = FUN_0042ca70(pCar->field_0xb1a);
    g_physWheels = FUN_0042cab0(pCar->field_0xb1a);
    FixMatrix_GetRight(&g_physRight, &g_physBody->body);
    FixMatrix_GetUp(&g_physUp, &g_physBody->body);
    FixMatrix_GetForward(&g_physForward, &g_physBody->body);
    FixMatrix_GetPosition(&g_physPos, &g_physBody->body);
    g_physGroundPoint = (FixVector *)FUN_0042cac0(pCar->field_0xb1a);
    if (pCar->field_0xb64 != 0) {
        h = FixMul(g_physBody->groundNormal.y, g_physBody->cornerHeight[0] - g_physPos.y);
    } else {
        g_physGroundPos = *g_physGroundPoint;
        g_physGroundPos.x -= g_physPos.x;
        g_physGroundPos.y = g_physBody->cornerHeight[0] - g_physPos.y;
        g_physGroundPos.z -= g_physPos.z;
        pNormal = &g_physBody->groundNormal;
        h = FixVecDot(pNormal, &g_physGroundPos);
    }
    FixVecScale(&g_physGroundPos, &g_physBody->groundNormal, h);
    g_physGroundPos.x += g_physPos.x;
    g_physGroundPos.y += g_physPos.y;
    g_physGroundPos.z += g_physPos.z;
    if (pContact->field_0x298 != 0)
        FUN_00497db0(pCar);
    FUN_004962c0(pCar, pContact);
    if (pContact->field_0x294 != 0 || (g_pGraphics->field913_0x3bc & 0x20)) {
        pContact->pointCount = 4;
    } else {
        // Trail direction relative to the car, flattened onto the ground.
        scale = FixDiv(g_physPatchLength, *pProfile);
        flat = g_physPatchDir;
        flat.y = 0;
        FIX_NORMALIZE_INTO(flat, flat);
        flat.y = 0;
        cosA = FixMul(flat.z, g_physTrailAxis.z) + FixMul(flat.x, g_physTrailAxis.x);
        grip = FixVecDot(&g_physUp, &g_physBody->groundNormal) - 0xcccc;
        if (grip < 0) {
            grip = 0;
        } else {
            grip = FixMul(grip, FixDiv(0x10000, 0x3334));
            if (grip > 0x10000)
                grip = 0x10000;
        }
        fwd = FixMul(cosA, g_physTrailScale);
        side = FixMul(FixMul(flat.x, g_physTrailAxis.z) + FixMul(-flat.z, g_physTrailAxis.x), g_physTrailScale);
        bend = FixMul(grip, side);
        if (fwd < 0)
            a = pProfile[*g_physSkidCount[pCar->field_0xb1a] * 2 - 1];
        else
            a = pProfile[1];
        base = FixMul(a + *g_physSkidOffset[pCar->field_0xb1a], fwd);
        lateral = g_physPatchWidth;
        if (side < 0)
            lateral = -g_physPatchWidth;
        lateral += FixMul(*g_physSkidOffset[pCar->field_0xb1a], side);
        if (fwd < 0) {
            if (side < 0) {
                pContact->trailEdge = 1;
                skip = 1;
            } else {
                pContact->trailEdge = 0;
                skip = 0;
            }
        } else if (side < 0) {
            pContact->trailEdge = 2;
            skip = 0;
        } else {
            pContact->trailEdge = 3;
            skip = 1;
        }
        pContact->field_0x29c = skip;
        if (side < 0)
            pContact->field_0x2a0 = 0;
        else
            pContact->field_0x2a0 = 1;
        for (c = 0; c < *g_physSkidCount[pCar->field_0xb1a]; c++) {
            a = pProfile[c * 2];
            h = FixMul(pProfile[c * 2 + 1], bend) + lateral;
            if (c != 0 && c != *g_physSkidCount[pCar->field_0xb1a] - 1) {
                if (fwd < 0)
                    grip = pProfile[c * 2 + 2] - a;
                else
                    grip = pProfile[c * 2 - 2] - a;
                a += FixMul(grip, FIX_ABS(cosA));
            }
            along = FixMul(a + base, scale);
            if (fwd < 0 ? c == *g_physSkidCount[pCar->field_0xb1a] - 1 : c == 0) {
                firstAlong = along;
                firstLat = h;
            }
            FixVecScale(&p, &g_physPatchDir, along);
            FixVecScale(&v, &g_physPatchSide, h);
            p.y += v.y;
            p.x += v.x;
            p.z += v.z;
            if (side < 0)
                k = *g_physSkidCount[pCar->field_0xb1a] - c + 3;
            else
                k = c + 4;
            if (skip)
                k++;
            if ((c == 0 && side >= 0) || (c == *g_physSkidCount[pCar->field_0xb1a] - 1 && side < 0))
                first = k;
            pContact->points[k].x = p.x + g_physGroundPos.x;
            pContact->points[k].y = p.y + g_physGroundPos.y;
            pContact->points[k].z = p.z + g_physGroundPos.z;
        }
        // Closing point, just outside the body patch.
        if (side < 0)
            extra = FixMul(g_physPatchWidth, 0x20000);
        else
            extra = -FixMul(g_physPatchWidth, 0x20000);
        firstLat += extra;
        FixVecScale(&v, &g_physPatchSide, firstLat);
        p.y = FixMul(g_physPatchDir.y, firstAlong) + v.y;
        p.x = FixMul(g_physPatchDir.x, firstAlong) + v.x;
        p.z = FixMul(g_physPatchDir.z, firstAlong) + v.z;
        if (skip == 0) {
            pContact->points[*g_physSkidCount[pCar->field_0xb1a] + 4].x = p.x + g_physGroundPos.x;
            pContact->points[*g_physSkidCount[pCar->field_0xb1a] + 4].y = g_physGroundPos.y + p.y;
            pContact->points[*g_physSkidCount[pCar->field_0xb1a] + 4].z = g_physGroundPos.z + p.z;
        } else {
            pContact->points[4].x = p.x + g_physGroundPos.x;
            pContact->points[4].y = p.y + g_physGroundPos.y;
            pContact->points[4].z = p.z + g_physGroundPos.z;
        }
        pContact->pointCount = *g_physSkidCount[pCar->field_0xb1a] + 5;
    }
    if (pContact->field_0x294 != 0)
        return;
    if (!(g_pGraphics->field913_0x3bc & 0x20)) {
        // Bend the trail: no point may turn away from the trail direction
        // faster than 1.5 degrees per point.
        if (side < 0)
            c = *g_physSkidCount[pCar->field_0xb1a] - 1;
        else
            c = 0;
        while (c < *g_physSkidCount[pCar->field_0xb1a] && c >= 0) {
            k = c + (short)first;
            v.x = pContact->points[k % pContact->pointCount].x - g_physGroundPos.x;
            v.y = pContact->points[k % pContact->pointCount].y - g_physGroundPos.y;
            v.z = pContact->points[k % pContact->pointCount].z - g_physGroundPos.z;
            len = FixVecLength(&v);
            if (len > 0)
                FixVecScaleRecip(&v, &v, len);
            angle = FixAcos(FixVecDot(&g_physPatchDir, &v));
            limit = (0x400 - angle) * 0x1680;
            if (side < 0 ? c != *g_physSkidCount[pCar->field_0xb1a] - 1 : c != 0) {
                acc += 0x18000;
                if (limit < acc) {
                    if (side < 0)
                        FixVecScale(&w, &g_physPatchSide, FixSin((__int64)((double)-acc * (1.0 / 5760.0))));
                    else
                        FixVecScale(&w, &g_physPatchSide, FixSin((__int64)((double)acc * (1.0 / 5760.0))));
                    a = FixCos((__int64)((double)acc * -(1.0 / 5760.0)));
                    p.x = FixMul(g_physPatchDir.x, a) + w.x;
                    p.y = FixMul(g_physPatchDir.y, a) + w.y;
                    p.z = FixMul(g_physPatchDir.z, a) + w.z;
                    FIX_NORMALIZE_INTO(v, p);
                    FixVecScale(&v, &v, len);
                    pContact->points[((short)first + c) % pContact->pointCount].x = g_physGroundPos.x + v.x;
                    pContact->points[((short)first + c) % pContact->pointCount].y = g_physGroundPos.y + v.y;
                    pContact->points[((short)first + c) % pContact->pointCount].z = g_physGroundPos.z + v.z;
                    limit = acc;
                }
            }
            if (side < 0)
                c--;
            else
                c++;
            acc = limit;
        }
    }
    for (c = 0; c < pContact->pointCount; c++) {
        h = Track_GetGroundHeight(&pContact->points[c], &v, &pContact->pointTriangle[c], (unsigned short *)&first,
                                  pContact->pointGroundY[c]);
        pContact->pointGroundY[c] = h;
        h -= pContact->points[c].y;
        if (FIX_ABS(h) > 0x8000)
            h = h < 1 ? -0x8000 : 0x8000;
        pContact->points[c].y += h;
    }
}

// Points the skid trail reference axis along v (flattened, reversed) and
// sets the trail length from how far v leans out of the horizontal.
// FUNCTION: CMR2 0x00498370
void FUN_00498370(FixVector *v)
{
    int len;

    g_physTrailAxis = *v;
    g_physTrailAxis.y = 0;
    len = FixVecLength(&g_physTrailAxis);
    if (len > 0)
        FixVecScaleRecip(&g_physTrailAxis, &g_physTrailAxis, -len);
    g_physTrailScale = FixMul(0x10000 - v->y, 0x20000);
}

// Sets the shadow level of a car and the matching blend colours.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004984b0
void FUN_004984b0(int car, int level)
{
    BYTE colour[4];
    BYTE factor[4];
    BYTE alpha;

    colour[3] = 0x32;
    factor[3] = 0x32;
    colour[0] = 0;
    colour[1] = 0;
    colour[2] = 0;
    factor[0] = 0;
    factor[1] = 0;
    factor[2] = 0;
    CAR_CONTACT(car)->field_0x250 = level;
    colour[3] = FixMulShift32(0x4b0000, CAR_CONTACT(car)->field_0x250);
    FUN_004ae140(colour);
    alpha = FixMulShift32(0x4b0000, CAR_CONTACT(car)->field_0x250) + 100;
    factor[3] = alpha;
    if (alpha > 0xff)
        factor[3] = 0xff;
    Graphics_SetTextureFactorAlpha(factor);
}
