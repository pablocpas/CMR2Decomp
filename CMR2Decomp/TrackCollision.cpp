#include <windows.h>
#include "FixedPoint.h"
#include "Car.h"
#include "Mesh.h"
#include <stdio.h>
#include "Frontend.h"
#include "GenericFileLoader.h"
#include "StageTiming.h"
#include "GameInfo.h"

// Ground queries against the stage collision mesh loaded by FUN_00490d50:
// triangles (three vertex indices plus a surface byte, 8 bytes each), their
// vertices, and a five-level quadtree whose leaves list the triangles of a cell.

// Pointers into the stage collision block (see FUN_00490d50).
extern int g_unk0x00591af0;     // triangles: unsigned short v0, v1, v2; BYTE pad, surface
extern int g_unk0x00591af8;     // grid origin: int x, z
extern int g_unk0x00591b00[5];  // quadtree levels: { short count; int first; } per node
extern int g_unk0x00591b14;     // vertices (FixVector)
extern int g_unk0x00591b18;     // triangle index lists (short)
extern int g_unk0x00591b1c;     // grid depth in cells (short)
extern int g_unk0x00591b20;     // grid width in cells (short)

short FUN_00478a10(short index);

// Scratch values of the point-in-triangle edge test.
// GLOBAL: CMR2 0x00591af4
int g_trackEdgeSide;
// GLOBAL: CMR2 0x00591b28
int g_trackEdgeDX;
// GLOBAL: CMR2 0x00591b2c
int g_trackEdgeDZ;
// GLOBAL: CMR2 0x00591c50
int g_trackEdgeNX;
// GLOBAL: CMR2 0x00591c54
int g_trackEdgeNZ;
// GLOBAL: CMR2 0x00591c58
FixVector g_trackTriangle[3];

// Copies the three vertices of a collision triangle.
// FUNCTION: CMR2 0x00490e00
int Track_GetTriangle(FixVector *pOut, short tri)
{
#define TRACK_VERTEX(n, c) \
    *(int *)(g_unk0x00591b14 + *(unsigned short *)(tri * 8 + g_unk0x00591af0 + (n) * 2) * 12 + (c) * 4)
    pOut[0].x = TRACK_VERTEX(0, 0);
    pOut[0].y = TRACK_VERTEX(0, 1);
    pOut[0].z = TRACK_VERTEX(0, 2);
    pOut[1].x = TRACK_VERTEX(1, 0);
    pOut[1].y = TRACK_VERTEX(1, 1);
    pOut[1].z = TRACK_VERTEX(1, 2);
    pOut[2].x = TRACK_VERTEX(2, 0);
    pOut[2].y = TRACK_VERTEX(2, 1);
    pOut[2].z = TRACK_VERTEX(2, 2);
#undef TRACK_VERTEX
    return 1;
}

#define TRACK_EDGE_TEST(a, b)                                                                  \
    g_trackEdgeNX = pTri[b].z - pTri[a].z;                                                     \
    g_trackEdgeNZ = -(pTri[b].x - pTri[a].x);                                                  \
    g_trackEdgeDX = pPoint->x - pTri[a].x;                                                     \
    g_trackEdgeDZ = pPoint->z - pTri[a].z;                                                     \
    g_trackEdgeSide = FixMul(g_trackEdgeDX, g_trackEdgeNX) + FixMul(g_trackEdgeDZ, g_trackEdgeNZ)

// Whether the point lies inside the triangle when seen from above (the X/Z
// bounding box first, then the three edges). pTri may be NULL to load it.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490f20
int Track_PointInTriangle(FixVector *pPoint, short tri, FixVector *pTri)
{
    int v;

    if (pTri == NULL) {
        pTri = g_trackTriangle;
        if (!Track_GetTriangle(g_trackTriangle, tri))
            return 0;
    }
    v = pPoint->x;
    if ((v >= pTri[0].x || v >= pTri[1].x || v >= pTri[2].x) && (v <= pTri[0].x || v <= pTri[1].x || v <= pTri[2].x)) {
        v = pPoint->z;
        if ((v >= pTri[0].z || v >= pTri[1].z || v >= pTri[2].z) &&
            (v <= pTri[0].z || v <= pTri[1].z || v <= pTri[2].z)) {
            TRACK_EDGE_TEST(0, 1);
            if (g_trackEdgeSide >= 0) {
                TRACK_EDGE_TEST(1, 2);
                if (g_trackEdgeSide >= 0) {
                    TRACK_EDGE_TEST(2, 0);
                    if (g_trackEdgeSide >= 0)
                        return 1;
                }
            }
        }
    }
    return 0;
}

// Height of the point on the triangle's plane; pNormal receives the plane
// normal and pSurface the triangle's surface type.
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004910f0
int Track_GetHeight(FixVector *pPoint, short tri, int defaultY, FixVector *pNormal, unsigned short *pSurface)
{
    FixVector t[3];
    FixVector e1;
    FixVector e2;
    FixVector n1;
    FixVector n2;
    int len;

    if (!Track_GetTriangle(t, tri))
        return 0;
    *pSurface = *(BYTE *)(g_unk0x00591af0 + 6 + tri * 8) & 0x7f;
    e1.x = t[1].x - t[0].x;
    e1.y = t[1].y - t[0].y;
    e1.z = t[1].z - t[0].z;
    e2.x = t[2].x - t[0].x;
    e2.y = t[2].y - t[0].y;
    e2.z = t[2].z - t[0].z;
    FIX_NORMALIZE_INTO(n1, e1);
    FIX_NORMALIZE_INTO(n2, e2);
    pNormal->x = FixMul(n1.y, n2.z) - FixMul(n1.z, n2.y);
    pNormal->y = FixMul(n1.z, n2.x) - FixMul(n1.x, n2.z);
    pNormal->z = FixMul(n1.x, n2.y) - FixMul(n1.y, n2.x);
    len = FixVecLength(pNormal);
    if (len == 0) {
        pNormal->x = 0;
        pNormal->y = 0;
        pNormal->z = 0;
    } else {
        FixVecScaleRecip(pNormal, pNormal, len);
    }
    if (pNormal->y != 0)
        return FixDiv(FixMul(pNormal->z, t[0].z) + FixMul(pNormal->x, t[0].x) + FixMul(pNormal->y, t[0].y) -
                          FixMul(pPoint->z, pNormal->z) - FixMul(pPoint->x, pNormal->x),
                      pNormal->y);
    return defaultY;
}

// Of the listed triangles under the point, the one whose centre height is
// closest to y.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004916a0
int Track_FindNearestTriangle(FixVector *pPoint, short *pOut, int y, short count, short *pList)
{
    FixVector t[3];
    BOOL first;
    int best;
    int bestIndex;
    int i;
    int h;
    int d;

    best = 0;
    bestIndex = 0;
    i = 0;
    first = TRUE;
    if (count > 0) {
        do {
            if (Track_GetTriangle(t, pList[i]) && Track_PointInTriangle(pPoint, pList[i], t)) {
                h = (t[2].y + t[1].y + t[0].y) / 3;
                d = y - h;
                if (d < 0)
                    d = h - y;
                if (first) {
                    first = FALSE;
                    bestIndex = i;
                    best = d;
                } else if (d < best) {
                    bestIndex = i;
                    best = d;
                }
            }
            i++;
        } while (i < count);
        if (!first) {
            *pOut = pList[bestIndex];
            return 1;
        }
    }
    return 0;
}

// Finds the triangle under the point by walking down the quadtree from the
// top-level grid cell.
// match 39%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00491550
int Track_FindTriangle(FixVector *pPoint, short *pOut, int y)
{
    int dx;
    int dz;
    int mask;
    int shift;
    short row;
    short col;
    short level;
    int node;

    dx = pPoint->x - ((int *)g_unk0x00591af8)[0];
    dz = pPoint->z - ((int *)g_unk0x00591af8)[1];
    row = (short)(dz >> 24);
    col = (short)(dx >> 24);
    level = 0;
    node = *(short *)g_unk0x00591b20 * row + col;
    shift = 24;
    mask = 0xffffff;
    if (col < 0 || col >= *(short *)g_unk0x00591b20 || row < 0 || row >= *(short *)g_unk0x00591b1c)
        return 0;
    while (*(short *)(g_unk0x00591b00[level] + node * 8) == -1) {
        if (level >= 5)
            return 0;
        dx &= mask;
        dz &= mask;
        mask >>= 2;
        shift -= 2;
        node = (short)(dx >> shift) + *(int *)(g_unk0x00591b00[level] + node * 8 + 4) + (short)(dz >> shift) * 4;
        level++;
    }
    return Track_FindNearestTriangle(pPoint, pOut, y, *(short *)(g_unk0x00591b00[level] + node * 8),
                                     (short *)(g_unk0x00591b18 + *(int *)(g_unk0x00591b00[level] + node * 8 + 4) * 2));
}

// GLOBAL: CMR2 0x005918d8
BYTE g_unk0x005918d8;
// GLOBAL: CMR2 0x0072d250
int g_finCount;
// GLOBAL: CMR2 0x0072d254
BYTE *g_finData;
// GLOBAL: CMR2 0x0051fc38
char g_strFinFormat[] = "%sfin.dat";

BYTE *FUN_0041f900(void);

// Loads the stage's fin.dat table (0x30-byte records).
// FUNCTION: CMR2 0x00490c30
void FUN_00490c30(void)
{
    DWORD size = 0;

    g_unk0x005918d8 = 0;
    sprintf(CFrontend::m_stringDest, g_strFinFormat, FUN_0041f900());
    g_finData = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                     CFrontend::m_stringDest, 0, &size, 0);
    g_finCount = size / 0x30;
}

// Ground height under the point, reusing the cached triangle *pTri when the
// point is still over it; defaultY when the point is off the mesh.
// FUNCTION: CMR2 0x00490c90
int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri, unsigned short *pSurface, int defaultY)
{
    if (*pTri < 0)
        *pTri = -1;
    if (*pTri == -1) {
        if (Track_FindTriangle(pPoint, pTri, defaultY))
            return Track_GetHeight(pPoint, *pTri, defaultY, pNormal, pSurface);
        return defaultY;
    }
    if (Track_PointInTriangle(pPoint, *pTri, NULL))
        return Track_GetHeight(pPoint, *pTri, defaultY, pNormal, pSurface);
    if (Track_FindTriangle(pPoint, pTri, defaultY))
        return Track_GetHeight(pPoint, *pTri, defaultY, pNormal, pSurface);
    *pTri = -1;
    return defaultY;
}

// Same, also returning the surface class of the ground.
// FUNCTION: CMR2 0x004932b0
int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY)
{
    int height;

    height = Track_GetGroundHeight(pPoint, pNormal, pTri, pSurface, defaultY);
    *pSurfaceClass = FUN_00478a10(*pSurface);
    return height;
}

// GLOBAL: CMR2 0x0059226c
Car *g_pAutoGearCar;
// GLOBAL: CMR2 0x00592270
BYTE *g_pAutoGearSetup;

// Requests the adjacent automatic gear while the driver is shifting.  On an
// up-shift, a little extra hysteresis is derived from the difference between
// the first and third body corners.
// FUNCTION: CMR2 0x00493890
unsigned int FUN_00493890(void)
{
    unsigned int result;
    char gear;

    result = g_pAutoGearCar->field_0x1d4[0];
    if (g_pAutoGearCar->field_0x1d4[0] == 1) {
        gear = g_pAutoGearCar->field_0xb1e;
        result = (BYTE)gear;
        if (gear < 6) {
            int amount;
            int x;
            int z;
            int difference;

            g_pAutoGearCar->field_0xb20 = gear + 1;
            g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
            result = *(unsigned int *)(g_pAutoGearSetup + 0x278);
            if ((int)result > 0xb333) {
                amount = FixMul((int)result - 0xb333, 0x3553f);
                if (amount > 0x10000)
                    amount = 0x10000;
                else if (amount < 0)
                    amount = 0;

                x = g_pAutoGearCar->corners[0].x;
                z = g_pAutoGearCar->corners[0].z;
                difference = FIX_ABS(x) - FIX_ABS(z);
                if (difference < 0)
                    difference = FIX_ABS(z) - FIX_ABS(x);
                result = difference / 0x401;
                if ((difference % 0x401) * 0x40 < FixMul(amount, 0x4ccc)) {
                    result = (BYTE)g_pAutoGearCar->field_0xb20;
                    if (g_pAutoGearCar->field_0xb20 < 6) {
                        g_pAutoGearCar->field_0xb20++;
                        result = (BYTE)g_pAutoGearCar->field_0xb20;
                    }
                }
            }
            g_pAutoGearCar->field_0xb84 = 1;
            return result;
        }
        if (gear == 7) {
            g_pAutoGearCar->field_0xb20 = 0;
            g_pAutoGearCar->field_0xb84 = 1;
            g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
            return (unsigned int)g_pAutoGearSetup;
        }
    } else if (g_pAutoGearCar->field_0x1d4[0] == 0xff) {
        gear = g_pAutoGearCar->field_0xb1e;
        result = (BYTE)gear;
        if (gear < 7) {
            g_pAutoGearCar->field_0xb20 = gear - 1;
            g_pAutoGearCar->field_0xb84 = 1;
            g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
            result = (unsigned int)g_pAutoGearCar;
            if (g_pAutoGearCar->field_0xb20 < 0)
                g_pAutoGearCar->field_0xb20 = 7;
        }
    }
    return result;
}

// Selects the automatic gearbox's next gear from engine speed and road load.
// It also chooses reverse when the car stops against the driving direction.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00493b30
void Car_UpdateAutomaticGear(void)
{
    int i;
    int gear;
    int selected;
    int best;
    int candidate;
    int engine;
    int load;
    int chance;
    int difference;
    int dot;
    BOOL wheelAvailable = FALSE;

    if (g_pAutoGearCar->field_0xb9c == 0)
        return;

    for (i = 0; i < 4; i++) {
        if (g_pAutoGearCar->cornerFlags[i] == 0) {
            wheelAvailable = TRUE;
            break;
        }
    }
    if (g_pAutoGearCar->field_0xb21 > 0)
        g_pAutoGearCar->field_0xb21--;

    if (g_pAutoGearCar->field_0xb94 == 0 && wheelAvailable) {
        best = (int)0xd8f00000;
        selected = 0;
        gear = g_pAutoGearCar->field_0xb1e;
        engine = FixMul(g_pAutoGearCar->field_0x7a4, g_pAutoGearCar->field_0x7dc[gear]);
        for (i = 1; i <= 6; i++) {
            candidate = FixMul(engine, g_pAutoGearCar->field_0x7bc[i]);
            if (candidate > best &&
                (candidate < FixMul(g_pAutoGearCar->field_0x794, 0xfae1) || i == 6)) {
                best = candidate;
                selected = i;
            }
        }
        if (selected != gear) {
            load = FIX_ABS(g_pAutoGearCar->field_0x870[0]);
            if ((load < 0x8000 || selected < gear || g_pAutoGearCar->flag0x1d0[2] == 0) &&
                (g_pAutoGearCar->field_0xb21 == 0 ||
                 ((g_pAutoGearCar->field_0xb22 != 2 || gear <= selected) &&
                  (g_pAutoGearCar->field_0xb22 != 1 || selected <= gear)))) {
                g_pAutoGearCar->field_0xb84 = 1;
                if (gear < selected)
                    g_pAutoGearCar->field_0xb22 = 2;
                else
                    g_pAutoGearCar->field_0xb22 = 1;
                g_pAutoGearCar->field_0xb21 = 10;

                if (gear < selected && *(int *)(g_pAutoGearSetup + 0x278) > 0xb333) {
                    chance = FixMul(*(int *)(g_pAutoGearSetup + 0x278) - 0xb333, 0x3553f);
                    if (chance < 0)
                        chance = 0;
                    else if (chance > 0x10000)
                        chance = 0x10000;
                    difference = FIX_ABS(FIX_ABS(g_pAutoGearCar->corners[0].x) -
                                         FIX_ABS(g_pAutoGearCar->corners[0].z));
                    if ((difference % 0x401) * 0x40 < FixMul(chance, 0x4ccc) && selected < 6)
                        selected++;
                }
                g_pAutoGearCar->field_0xb20 = (char)selected;
                g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
            }
        }
    }

    dot = FixMul(g_pAutoGearCar->right.x, g_pAutoGearCar->velocity.x) +
          FixMul(g_pAutoGearCar->right.y, g_pAutoGearCar->velocity.y) +
          FixMul(g_pAutoGearCar->right.z, g_pAutoGearCar->velocity.z);
    if (g_pAutoGearCar->flag0x1d0[3] != 0 && g_pAutoGearCar->field_0xb1e == 1 &&
        g_pAutoGearCar->flag0x1d0[2] == 0 && g_pAutoGearCar->field_0x7a4 < 0x1999 && dot < 0x1999) {
        g_pAutoGearCar->field_0xb20 = 7;
        g_pAutoGearCar->field_0xb84 = 1;
        g_pAutoGearCar->field_0xb94 = 1;
        g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
        return;
    }
    if (g_pAutoGearCar->flag0x1d0[2] != 0 && g_pAutoGearCar->flag0x1d0[3] == 0 &&
        g_pAutoGearCar->field_0xb1e == 7) {
        g_pAutoGearCar->field_0xb20 = 1;
        g_pAutoGearCar->field_0xb84 = 1;
        g_pAutoGearCar->field_0xb94 = 0;
        g_pAutoGearCar->field_0xb24 = *(char *)(g_pAutoGearSetup + 0x468);
    }
}

void FUN_0046f4c0(int *pOut);
void FUN_0046f4d0(int *pOut);
void FUN_0046f4e0(int *pOut1, int *pOut2);
extern BYTE *g_unk0x005920f0;
extern FixVector g_unk0x00592114;

// GLOBAL: CMR2 0x00591c80
int g_stageHeightSamples[200];
// GLOBAL: CMR2 0x00591fa0
unsigned short *g_stageRandomTextures[3];   // one is picked at random for the whole stage mesh (0x492b50)
// GLOBAL: CMR2 0x00591fac
int g_stageHeightTarget;
// GLOBAL: CMR2 0x005920b8
SceneNode *g_stageLightNode;
// GLOBAL: CMR2 0x005920bc
SceneNode *g_stageLightObject;
// GLOBAL: CMR2 0x005920c0
SceneNode *g_stageLightRoot;
// GLOBAL: CMR2 0x005920c4
Mesh *g_stageMesh0;
// GLOBAL: CMR2 0x005920c8
Mesh *g_stageMesh0Copy;
// GLOBAL: CMR2 0x005920cc
Mesh *g_stageMesh1;
// GLOBAL: CMR2 0x005920d0
Mesh *g_stageMesh1Copy;
// GLOBAL: CMR2 0x005920d4
Mesh *g_stageMesh2;
// GLOBAL: CMR2 0x005920d8
Mesh *g_stageMesh2Copy;
// GLOBAL: CMR2 0x005920dc
Mesh *g_stageMesh3;
// GLOBAL: CMR2 0x005920e0
Mesh *g_stageMesh3Copy;
// GLOBAL: CMR2 0x005920e4
Mesh *g_stageMesh4;
// GLOBAL: CMR2 0x005920e8
Mesh *g_stageMesh4Copy;
// GLOBAL: CMR2 0x005920ec
Mesh *g_stageMesh5;
// GLOBAL: CMR2 0x005920f4
Mesh *g_stageMesh6;
// GLOBAL: CMR2 0x005920f8
Mesh *g_stageMesh6Copy;
// GLOBAL: CMR2 0x00592108
FixVector g_stageRangeOrigin;
// GLOBAL: CMR2 0x00592120
int g_stageHeightMin;
// GLOBAL: CMR2 0x00592124
int g_stageHeightScale;
// GLOBAL: CMR2 0x00592138
short g_stageMesh2Count;
// GLOBAL: CMR2 0x0059213a
short g_stageMesh3Count;
// GLOBAL: CMR2 0x0059213c
short g_stageMesh1Count;
// GLOBAL: CMR2 0x0059213e
short g_stageMesh0Count;
// GLOBAL: CMR2 0x00592140
short g_stageMesh4Count;
// GLOBAL: CMR2 0x00592142
short g_stageMesh5Count;
// GLOBAL: CMR2 0x00592144
short g_stageMesh6Count;
// GLOBAL: CMR2 0x00592146
BYTE g_unk0x00592146;
// GLOBAL: CMR2 0x00592147
BYTE g_stageColourAlpha;
// GLOBAL: CMR2 0x00592148
int g_stageColourDirty;
// GLOBAL: CMR2 0x0059214c
int g_stageColourMode;
// GLOBAL: CMR2 0x00592150
int g_stageColourValue;
// GLOBAL: CMR2 0x00592154
int g_stageColourStep;
// GLOBAL: CMR2 0x0059215c
int g_stageColourState;
// GLOBAL: CMR2 0x00592158
int g_stageLightReady;

// Loads the stage's light meshes and their height samples.  The samples are
// fixed point values converted from the first mesh's floating point vertices.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004919a0
void Stage_InitLightMeshes(void)
{
    int object0;
    int object1;
    int object2;
    int object3;
    int *node;
    int *root;
    int *child;
    int i;
    int value;
    int minimum;
    int maximum;
    float *vertices;

    FUN_0046f4c0(&object0);
    FUN_0046f4d0(&object1);
    FUN_0046f4e0(&object2, &object3);
    node = NULL;
    root = NULL;
    child = NULL;
    if (object0 != 0 && (node = *(int **)(object0 + 4)) != NULL &&
        (root = (int *)*node) != NULL)
        child = (int *)*root;

    g_stageRandomTextures[0] = NULL;
    g_stageRandomTextures[1] = NULL;
    g_stageRandomTextures[2] = NULL;
    g_stageLightNode = (SceneNode *)node;
    g_stageLightObject = SceneType2_Create((FixVector *)(node + 4), (FixAngles *)(node + 7), NULL,
                                           (SceneNode *)node);
    g_stageMesh0Count = 0;
    g_stageMesh1Copy = NULL;
    g_stageMesh1Count = 0;
    g_stageMesh2Copy = NULL;
    g_stageMesh2Count = 0;
    g_stageMesh3Copy = NULL;
    g_stageMesh3Count = 0;
    g_stageMesh4Copy = NULL;
    g_stageMesh4Count = 0;
    g_unk0x005920f0 = NULL;
    g_stageMesh5Count = 0;
    g_stageMesh6Copy = NULL;
    g_stageMesh6Count = 0;

    g_stageMesh0 = *(Mesh **)(object0 + 0xc);
    g_stageLightRoot = (SceneNode *)child;
    g_stageMesh0Copy = g_stageMesh0;
    g_stageMesh0Count = (short)Mesh_GetField0x10(g_stageMesh0);
    g_stageMesh1 = *(Mesh **)(object1 + 0xc);
    g_stageMesh1Copy = g_stageMesh1;
    g_stageMesh1Count = (short)Mesh_GetField0x10(g_stageMesh1);
    g_stageMesh2 = *(Mesh **)(object2 + 0xc);
    g_stageMesh2Copy = g_stageMesh2;
    g_stageMesh2Count = (short)Mesh_GetField0x10(g_stageMesh2);
    if (object3 != 0) {
        g_stageMesh3 = *(Mesh **)(object3 + 0xc);
        g_stageMesh3Copy = g_stageMesh3;
        g_stageMesh3Count = (short)Mesh_GetField0x10(g_stageMesh3);
    }
    g_stageMesh4 = (Mesh *)root[3];
    g_stageMesh4Copy = g_stageMesh4;
    g_stageMesh4Count = (short)Mesh_GetField0x10(g_stageMesh4);
    g_stageMesh5 = (Mesh *)node[3];
    g_unk0x005920f0 = (BYTE *)g_stageMesh5;
    g_stageMesh5Count = (short)Mesh_GetField0x10(g_stageMesh5);
    if (child != NULL) {
        g_stageMesh6 = *(Mesh **)((BYTE *)child + 0xc);
        g_stageMesh6Copy = g_stageMesh6;
        g_stageMesh6Count = (short)Mesh_GetField0x10(g_stageMesh6);
    }
    g_stageColourDirty = 0;
    g_stageColourMode = 0;
    g_stageColourValue = 0;
    g_stageColourStep = 0;
    g_stageColourState = 0;

    if (g_stageMesh0Count != 0) {
        maximum = 0;
        minimum = 0;
        for (i = g_stageMesh0Count - 1; i >= 0; i--) {
            vertices = (float *)((BYTE *)g_stageMesh0Copy->pVertexData + i * 0x30);
            value = (int)(__int64)((double)vertices[1] * CGraphics::m_65536);
            if (i == g_stageMesh0Count - 1) {
                minimum = value;
                maximum = value;
            } else {
                if (maximum < value)
                    maximum = value;
                if (value < minimum)
                    minimum = value;
            }
            if (i == 0x68) {
                g_unk0x00592114.y = value;
                g_unk0x00592114.x = (int)(__int64)((double)vertices[0] * CGraphics::m_65536);
                g_unk0x00592114.z = (int)(__int64)((double)vertices[2] * CGraphics::m_65536);
            }
            g_stageHeightSamples[i] = value;
        }
        if (maximum != minimum) {
            g_stageHeightScale = FixDiv(0x10000, maximum - minimum);
            g_stageHeightMin = minimum;
        }
    }
    if (g_stageMesh1Count != 0) {
        vertices = (float *)g_stageMesh1Copy->pVertexData;
        g_stageRangeOrigin.x = (int)(__int64)((double)*(float *)((BYTE *)vertices + 0x960) * CGraphics::m_65536);
        g_stageRangeOrigin.y = (int)(__int64)((double)*(float *)((BYTE *)vertices + 0x964) * CGraphics::m_65536);
        g_stageRangeOrigin.z = (int)(__int64)((double)*(float *)((BYTE *)vertices + 0x968) * CGraphics::m_65536);
        minimum = (int)(__int64)((double)vertices[1] * CGraphics::m_65536);
        maximum = minimum;
        for (i = g_stageMesh1Count - 1; i >= 0; i--) {
            value = (int)(__int64)((double)*(float *)((BYTE *)vertices + i * 0x30 + 4) * CGraphics::m_65536);
            if (maximum < value)
                maximum = value;
            if (value < minimum)
                minimum = value;
        }
        g_stageHeightTarget = minimum + FixMul(maximum - minimum, 0xc000);
    }
    for (i = g_stageMesh4Count - 1; i >= 0; i--)
        *(int *)((BYTE *)g_stageMesh4Copy->pVertexData + i * 0x30 + 0x18) = 0;
    g_stageLightReady = 1;
}

// Sets vertex colours from their heights, with a separate colour for the
// reference vertex recorded by Stage_InitLightMeshes.
// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00491d40
void Stage_SetHeightColours(BYTE *pLow, BYTE *pHigh, BYTE *pReference, int referenceBlend)
{
    FixVector low;
    FixVector delta;
    FixVector colour;
    FixVector refDelta;
    FixVector scaled;
    int t;
    int red;
    int green;
    int blue;
    int i;
    DWORD packedReference;
    float *vertex;

    low.x = (unsigned int)pLow[0] << 16;
    low.y = (unsigned int)pLow[1] << 16;
    low.z = (unsigned int)pLow[2] << 16;
    delta.x = ((unsigned int)pHigh[0] << 16) - low.x;
    delta.y = ((unsigned int)pHigh[1] << 16) - low.y;
    delta.z = ((unsigned int)pHigh[2] << 16) - low.z;

    t = FixMul(g_unk0x00592114.y - g_stageHeightMin, g_stageHeightScale);
    if (t < 0)
        t = 0;
    else if (t > 0x10000)
        t = 0x10000;
    FixVecScale(&scaled, &delta, t);
    colour.x = low.x + scaled.x;
    colour.y = low.y + scaled.y;
    colour.z = low.z + scaled.z;
    refDelta.x = ((unsigned int)pReference[0] << 16) - colour.x;
    refDelta.y = ((unsigned int)pReference[1] << 16) - colour.y;
    refDelta.z = ((unsigned int)pReference[2] << 16) - colour.z;
    FixVecScale(&scaled, &refDelta, referenceBlend);
    colour.x += scaled.x;
    colour.y += scaled.y;
    colour.z += scaled.z;
    packedReference = 0xff000000 | ((colour.x >> 16) & 0xff) << 16 |
                      ((colour.y >> 16) & 0xff) << 8 | ((colour.z >> 16) & 0xff);

    for (i = g_stageMesh0Count - 1; i >= 0; i--) {
        vertex = (float *)((BYTE *)g_stageMesh0Copy->pVertexData + i * 0x30);
        if ((int)(__int64)((double)vertex[1] * CGraphics::m_65536) == g_unk0x00592114.y &&
            (int)(__int64)((double)vertex[0] * CGraphics::m_65536) == g_unk0x00592114.x &&
            (int)(__int64)((double)vertex[2] * CGraphics::m_65536) == g_unk0x00592114.z) {
            *(DWORD *)((BYTE *)vertex + 0x18) = packedReference;
        } else {
            t = FixMul((int)(__int64)((double)vertex[1] * CGraphics::m_65536) - g_stageHeightMin,
                       g_stageHeightScale);
            if (t < 0)
                t = 0;
            else if (t > 0x10000)
                t = 0x10000;
            FixVecScale(&scaled, &delta, t);
            red = (low.x + scaled.x) >> 16;
            green = (low.y + scaled.y) >> 16;
            blue = (low.z + scaled.z) >> 16;
            if (red > 255) red = 255;
            else if (red < 0) red = 0;
            if (green > 255) green = 255;
            else if (green < 0) green = 0;
            if (blue > 255) blue = 255;
            else if (blue < 0) blue = 0;
            *(DWORD *)((BYTE *)vertex + 0x18) = 0xff000000 | (red << 16) | (green << 8) | blue;
        }
        *(DWORD *)((BYTE *)vertex + 0x1c) = (DWORD)g_stageColourAlpha << 24;
    }
    g_stageColourDirty = 1;
}

// Ramps field 0x818 of the auto-gear car toward +1 or -1 by its two flags.
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00494540
void FUN_00494540(void)
{
    int value = 0;

    if (g_pAutoGearCar->flag0x1d0[0] == 0) {
        if (g_pAutoGearCar->flag0x1d0[1] != 0) {
            if (g_pAutoGearCar->field_0x818 > 0)
                g_pAutoGearCar->field_0x818 = 0;
            value = -0x10000;
            g_pAutoGearCar->field_0x818 -= 0x10000;
            if (g_pAutoGearCar->field_0x818 > -0x10001)
                return;
        }
        g_pAutoGearCar->field_0x818 = value;
    } else {
        if (g_pAutoGearCar->field_0x818 < 0)
            g_pAutoGearCar->field_0x818 = 0;
        g_pAutoGearCar->field_0x818 += 0x10000;
        if (g_pAutoGearCar->field_0x818 > 0x10000) {
            g_pAutoGearCar->field_0x818 = 0x10000;
            return;
        }
    }
}

#define SWAP_RB(c) ((((((c) >> 24) << 8 | ((c) & 0xff)) << 8 | (((c) >> 8) & 0xff)) << 8) | (((c) >> 16) & 0xff))

// Sets the diffuse colour (and alpha) of every vertex of the stage sky mesh.
// match 13%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00492520
void FUN_00492520(DWORD *pColour)
{
    DWORD colour;
    int i;

    if (g_stageMesh6Copy != NULL) {
        colour = *pColour;
        for (i = g_stageMesh6Count - 1; i >= 0; i--) {
            *(DWORD *)((BYTE *)g_stageMesh6Copy->pVertexData + i * 0x30 + 0x18) = SWAP_RB(colour);
            *(DWORD *)((BYTE *)g_stageMesh6Copy->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
        }
        g_stageColourState = 1;
    }
}

// Sets the diffuse colour of every vertex of the stage light mesh.
// match 14%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004923d0
void FUN_004923d0(DWORD *pColour)
{
    DWORD colour = *pColour;
    int i;

    for (i = g_stageMesh4Count - 1; i >= 0; i--) {
        *(DWORD *)((BYTE *)g_stageMesh4Copy->pVertexData + i * 0x30 + 0x18) = SWAP_RB(colour);
        *(DWORD *)((BYTE *)g_stageMesh4Copy->pVertexData + i * 0x30 + 0x1c) = 0xff000000;
    }
    g_stageLightReady = 1;
}

// GLOBAL: CMR2 0x005920b0
SceneNode *g_stageAmbientNode;

void Scene_SetLightColour(SceneNode *pNode, int r, int g, int b);
void Mesh_RefreshVertices(Mesh *pMesh);

#define CLAMP_UNIT(v) ((v) > 0x10000 ? 0x10000 : ((v) < 0 ? 0 : (v)))

// Sets the stage ambient light from an 8-bit RGB triple (scaled to 16.16).
// FUNCTION: CMR2 0x00492e60
void FUN_00492e60(int *pRGB)
{
    int r = FixMul(pRGB[0], 0x106);
    int g;
    int b;

    if (r > 0x10000)
        r = 0x10000;
    else if (r < 0)
        r = 0;
    g = FixMul(pRGB[1], 0x106);
    if (g > 0x10000)
        g = 0x10000;
    else if (g < 0)
        g = 0;
    b = FixMul(pRGB[2], 0x106);
    if (b > 0x10000) {
        Scene_SetLightColour(g_stageAmbientNode, r, g, 0x10000);
        return;
    }
    if (b < 0)
        b = 0;
    Scene_SetLightColour(g_stageAmbientNode, r, g, b);
}

// Pushes the vertex colours of every recoloured stage mesh.
// FUNCTION: CMR2 0x00492f10
void FUN_00492f10(void)
{
    if (g_stageColourStep != 0) {
        Mesh_RefreshVertices(g_stageMesh2);
        if (g_stageMesh3Copy != NULL)
            Mesh_RefreshVertices(g_stageMesh3);
        g_stageColourStep = 0;
    }
    if (g_stageLightReady != 0) {
        Mesh_RefreshVertices(g_stageMesh4);
        g_stageLightReady = 0;
    }
    if (g_stageColourMode != 0) {
        Mesh_RefreshVertices(g_stageMesh1);
        g_stageColourMode = 0;
    }
    if (g_stageColourDirty != 0) {
        Mesh_RefreshVertices(g_stageMesh0);
        g_stageColourDirty = 0;
    }
    if (g_stageColourValue != 0) {
        Mesh_RefreshVertices(g_stageMesh5);
        g_stageColourValue = 0;
    }
    if (g_stageColourState != 0) {
        Mesh_RefreshVertices(g_stageMesh6);
        g_stageColourState = 0;
    }
}

// Sets the diffuse colour (and alpha) of every vertex of stage mesh 5.
// match 13%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00492470
void FUN_00492470(DWORD *pColour)
{
    DWORD colour = *pColour;
    int i;

    for (i = g_stageMesh5Count - 1; i >= 0; i--) {
        *(DWORD *)((BYTE *)((Mesh *)g_unk0x005920f0)->pVertexData + i * 0x30 + 0x18) = SWAP_RB(colour);
        *(DWORD *)((BYTE *)((Mesh *)g_unk0x005920f0)->pVertexData + i * 0x30 + 0x1c) = (DWORD)g_stageColourAlpha << 24;
    }
    g_stageColourValue = 1;
}

extern double g_unk0x00511300;

// Swings field 0x848 of the auto-gear car toward its target over time (a
// quarter sine), or resets it when the swing is off.
// FUNCTION: CMR2 0x00494960
void FUN_00494960(void)
{
    unsigned short angle;

    if (g_pAutoGearCar->field_0x1d8 != 0) {
        g_pAutoGearCar->field_0x84c += g_pAutoGearCar->field_0x840;
        if (g_pAutoGearCar->field_0x84c > 0x10000) {
            g_pAutoGearCar->field_0x84c = 0x10000;
            g_pAutoGearCar->field_0x848 = g_pAutoGearCar->field_0x844;
            return;
        }
        angle = (unsigned short)(__int64)(FixMul(g_pAutoGearCar->field_0x84c, 0x5a0000) * g_unk0x00511300);
        g_pAutoGearCar->field_0x848 = FixMul(g_pAutoGearCar->field_0x844, g_sinTable[angle & 0xfff]);
    } else {
        g_pAutoGearCar->field_0x84c = 0;
        g_pAutoGearCar->field_0x848 = 0;
    }
}

void Graphics_SetFog(int start, int end, int a, int b, DWORD colour);

// Sets the fog and the matching sky alpha: the sky fades out as the draw
// distance reaches into the fog.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00492fe0
void FUN_00492fe0(DWORD *pColour, int start, int end)
{
    int distance;
    int t;
    int alpha;

    Graphics_SetFog(start, end, start, end, *pColour);
    distance = (CGameInfo::FUN_00405ca0() + 2) * 0x320000;
    if (distance < start) {
        g_unk0x00592146 = 0xff;
        g_stageColourAlpha = 0xff;
        return;
    }
    if (distance <= end) {
        t = distance - start;
        if (end - start != 0)
            t = FixDiv(t, end - start);
        alpha = FixMul(t, 0xff0000) >> 16;
        if (alpha > 0xff)
            alpha = 0xff;
        else if (alpha < 0)
            alpha = 0;
        g_unk0x00592146 = (BYTE)(-1 - alpha);
        g_stageColourAlpha = g_unk0x00592146;
        return;
    }
    g_unk0x00592146 = 0;
    g_stageColourAlpha = 0;
}

// Tracks how fast the rolling direction of the auto-gear car follows its
// body (field 0x79c) from the phase in field 0x7a0, and latches flag 2 when
// the car starts rolling while the shift is unlocked.
// FUNCTION: CMR2 0x004946c0
void FUN_004946c0(void)
{
    unsigned short angle;

    if (g_pAutoGearCar->flag0x1d0[2] == 0 &&
        (g_pAutoGearCar->field_0xb94 == 0 || g_pAutoGearCar->flag0x1d0[3] == 0)) {
        *(int *)g_pAutoGearCar->field_0x7a0 =
            *(int *)g_pAutoGearCar->field_0x7a0 - FixMul(g_pAutoGearCar->field_0x790, 0x20000);
        if (*(int *)g_pAutoGearCar->field_0x7a0 < 0) {
            *(int *)g_pAutoGearCar->field_0x7a0 = 0;
            g_pAutoGearCar->field_0x79c = 0;
        } else {
            angle = (unsigned short)(__int64)(FixMul(*(int *)g_pAutoGearCar->field_0x7a0, 0x5a0000) *
                                              g_unk0x00511300);
            g_pAutoGearCar->field_0x79c = FixMul(*(int *)g_pAutoGearCar->field_0x788, g_sinTable[angle & 0xfff]);
        }
        if (g_pAutoGearCar->field_0x79c != 0 && g_pAutoGearCar->field_0xb94 == 0) {
            g_pAutoGearCar->flag0x1d0[2] = 0x3f;
            return;
        }
    } else {
        *(int *)g_pAutoGearCar->field_0x7a0 = *(int *)g_pAutoGearCar->field_0x7a0 + g_pAutoGearCar->field_0x790;
        if (*(int *)g_pAutoGearCar->field_0x7a0 > 0x10000) {
            *(int *)g_pAutoGearCar->field_0x7a0 = 0x10000;
            g_pAutoGearCar->field_0x79c = *(int *)g_pAutoGearCar->field_0x788;
            return;
        }
        angle = (unsigned short)(__int64)(FixMul(*(int *)g_pAutoGearCar->field_0x7a0, 0x5a0000) *
                                          g_unk0x00511300);
        g_pAutoGearCar->field_0x79c = FixMul(*(int *)g_pAutoGearCar->field_0x788, g_sinTable[angle & 0xfff]);
    }
}

// Swings field 0x838 of the auto-gear car toward 0x82c over time while its
// flag 3 is set and the shift is unlocked; else resets it.
// FUNCTION: CMR2 0x00494880
void FUN_00494880(void)
{
    unsigned short angle;

    if (g_pAutoGearCar->flag0x1d0[3] != 0 && g_pAutoGearCar->field_0xb94 == 0) {
        g_pAutoGearCar->field_0x83c += g_pAutoGearCar->field_0x834;
        if (g_pAutoGearCar->field_0x83c > 0x10000) {
            g_pAutoGearCar->field_0x83c = 0x10000;
            g_pAutoGearCar->field_0x838 = g_pAutoGearCar->field_0x82c;
            return;
        }
        angle = (unsigned short)(__int64)(FixMul(g_pAutoGearCar->field_0x83c, 0x5a0000) * g_unk0x00511300);
        g_pAutoGearCar->field_0x838 = FixMul(g_pAutoGearCar->field_0x82c, g_sinTable[angle & 0xfff]);
        return;
    }
    g_pAutoGearCar->field_0x83c = 0;
    g_pAutoGearCar->field_0x838 = 0;
}

// Flags whether the auto-gear car is at or below its best gear for the
// current revs (gear with the most torque below 98% of the limit).
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00493a40
void FUN_00493a40(void)
{
    int best;
    int bestTorque;
    int torque;
    int gear;

    if (g_pAutoGearCar->field_0x7bc[g_pAutoGearCar->field_0xb1e] < 1) {
        best = 7;
    } else {
        bestTorque = 0xd8f00000;
        best = 0;
        for (gear = 1; gear < 7; gear++) {
            torque = FixMul(FixMul(g_pAutoGearCar->field_0x7a4, g_pAutoGearCar->field_0x7dc[g_pAutoGearCar->field_0xb1e]),
                            g_pAutoGearCar->field_0x7bc[gear]);
            if (bestTorque < torque && (torque < FixMul(g_pAutoGearCar->field_0x794, 0xfae1) || gear == 6)) {
                best = gear;
                bestTorque = torque;
            }
        }
    }
    if (g_pAutoGearCar->field_0xb1e <= best) {
        g_pAutoGearCar->field_0xb98 = 1;
        return;
    }
    g_pAutoGearCar->field_0xb98 = 0;
}

// GLOBAL: CMR2 0x00592164
int g_unk0x00592164;

// GLOBAL: CMR2 0x00592160
int g_unk0x00592160;

// Reduces the steering scale as the car's forward velocity aligns with its
// right axis.  The result is expressed in the same 0x1680 units as field b16.
// FUNCTION: CMR2 0x004943d0
void FUN_004943d0(void)
{
    int alignment;
    int amount;

    if ((g_pAutoGearCar->field_0xb1b[1] & 1) != 0) {
        alignment = FixMul(g_pAutoGearCar->right.x, g_pAutoGearCar->velocity.x) +
                    FixMul(g_pAutoGearCar->right.y, g_pAutoGearCar->velocity.y) +
                    FixMul(g_pAutoGearCar->right.z, g_pAutoGearCar->velocity.z);
        alignment = FIX_ABS(alignment);
        amount = FixMul(alignment, g_pAutoGearCar->field_0x828);
        if (amount >= 0xb333)
            amount = 0xb333;
        g_unk0x00592160 = FixMul(0x10000 - amount, g_pAutoGearCar->field_0xb16) * 0x1680;
        return;
    }
    g_unk0x00592160 = g_pAutoGearCar->field_0xb16 * 0x1680;
}

void FUN_00494540(void);
extern int g_physicsTimeStep;

// Steering torque of the auto-gear car from its steering swing.
// FUNCTION: CMR2 0x004945d0
void FUN_004945d0(void)
{
    int torque;
    int a;
    int r;

    FUN_00494540();
    if (g_pAutoGearCar->flag0x1d0[1] == 0 && g_pAutoGearCar->flag0x1d0[0] == 0) {
        g_unk0x00592164 = 0;
        return;
    }
    torque = FixMul(g_pAutoGearCar->field_0x824, g_physicsTimeStep);
    if ((g_pAutoGearCar->field_0xb1b[1] & 2) != 0) {
        torque = FixMul(torque, FixMul(
            (g_pAutoGearCar->field_0x81c < 0 ? -g_pAutoGearCar->field_0x81c : g_pAutoGearCar->field_0x81c) - 0x10000,
            (g_pAutoGearCar->field_0x81c < 0 ? -g_pAutoGearCar->field_0x81c : g_pAutoGearCar->field_0x81c) - 0x10000));
    }
    a = g_pAutoGearCar->field_0x818 < 0 ? -g_pAutoGearCar->field_0x818 : g_pAutoGearCar->field_0x818;
    r = FixMul(torque, a);
    if (g_pAutoGearCar->flag0x1d0[1] != 0) {
        g_unk0x00592164 = -r;
        return;
    }
    if (g_pAutoGearCar->flag0x1d0[0] != 0)
        g_unk0x00592164 = r;
}

void Scene_SetLightPosition(SceneNode *pNode, int x, int y, int z);
extern int g_unk0x00592128;
extern int g_unk0x0059212c;
extern int g_unk0x00592130;

// Shifts stage mesh heights and moves the ambient light with the stage.
// FUNCTION: CMR2 0x004925c0
void FUN_004925c0(int oldHeight, int newHeight, int mode)
{
    int height = g_stageHeightTarget - oldHeight + newHeight;
    int i;
    int object;
    FixVector position;

    for (i = g_stageMesh0Count - 1; i >= 0; i--) {
        if (g_stageHeightSamples[i] < height) {
            float *vertex = (float *)((BYTE *)g_stageMesh0Copy->pVertexData + i * 0x30);
            if ((int)(__int64)((double)vertex[0] * CGraphics::m_65536) != g_unk0x00592114.x ||
                (int)(__int64)((double)vertex[1] * CGraphics::m_65536) != g_unk0x00592114.y ||
                (int)(__int64)((double)vertex[2] * CGraphics::m_65536) != g_unk0x00592114.z)
                vertex[1] = (float)((double)height * CGraphics::m_oneOver65536);
        }
    }
    g_stageColourDirty = 1;
    g_unk0x00592128 = mode;
    g_unk0x00592130 = newHeight;
    g_unk0x0059212c = oldHeight;
    FUN_0046f4c0(&object);
    FixMatrix_RotateVector(&position, &g_unk0x00592114, (FixMatrix *)(object + 0x98));
    position.y += g_unk0x0059212c;
    Scene_SetLightPosition(g_stageAmbientNode, position.x, position.y, position.z);
}

int FUN_00407270(void);
unsigned int RallyDataState(void);
unsigned int RallyData_GetFlag24(void);
unsigned int RallyData_FUN_00407e90(void);
BYTE *FUN_0041b390(void);
int FUN_0041f3d0(BYTE index);
BYTE *FUN_0041f350(int index);
void FUN_0047d5a0(BYTE car);

// Applies the automatic gearbox's mid-shift body nudge: while a shift is in
// progress the body is pushed along its right axis by an amount derived from
// the road speed, then the shift flag is cleared.
// FUNCTION: CMR2 0x004932f0
void FUN_004932f0(void)
{
    int shifted;
    int speed;
    FixVector offset;

    shifted = 0;
    if (CGameInfo::FUN_004063f0(5) != 0) {
        *(int *)((BYTE *)g_pAutoGearCar + 0xa84) = 0;
        if ((int)g_pAutoGearCar->field_0xb1a < (int)(BYTE)RallyDataState() &&
            FUN_0041f3d0(g_pAutoGearCar->field_0xb1a) != 0 &&
            *(char *)(FUN_0041f350((int)g_pAutoGearCar->field_0xb1a) + 0x10c) == 8)
            shifted = 1;
        if (*(char *)(*(int *)(FUN_0041b390() + 4) + g_pAutoGearCar->field_0xb1a * 8) == 8 ||
            shifted) {
            if (g_pAutoGearCar->field_0x1d8 != 0) {
                speed = g_pAutoGearCar->speed;
                if (speed <= 0x2c000) {
                    *(int *)((BYTE *)g_pAutoGearCar + 0xa84) = 0x10000;
                } else if (speed <= 0x30000) {
                    *(int *)((BYTE *)g_pAutoGearCar + 0xa84) = speed - 0x2c000;
                    *(int *)((BYTE *)g_pAutoGearCar + 0xa84) =
                        FixMul(*(int *)((BYTE *)g_pAutoGearCar + 0xa84), 0x40000);
                    if (*(int *)((BYTE *)g_pAutoGearCar + 0xa84) > 0x10000)
                        *(int *)((BYTE *)g_pAutoGearCar + 0xa84) = 0x10000;
                    *(int *)((BYTE *)g_pAutoGearCar + 0xa84) =
                        0x10000 - *(int *)((BYTE *)g_pAutoGearCar + 0xa84);
                }
                FixVecScale(&offset, &g_pAutoGearCar->right,
                            FixMul(*(int *)((BYTE *)g_pAutoGearCar + 0xa84), 0x4000));
                g_pAutoGearCar->field_0x5c4.x += offset.x;
                g_pAutoGearCar->field_0x5c4.y += offset.y;
                g_pAutoGearCar->field_0x5c4.z += offset.z;
            }
            g_pAutoGearCar->field_0x1d8 = 0;
        }
    } else {
        if (CGameInfo::FUN_004063f0(0) != 0) {
            if (((char)FUN_00407270() != 0 || (char)RallyData_GetFlag24() != 0 ||
                 (char)RallyData_FUN_00407e90() != 0) &&
                (g_pAutoGearCar->field_0xb9c != 0 && g_pAutoGearCar->field_0x1d8 != 0))
                FUN_0047d5a0(g_pAutoGearCar->field_0xb1a);
            g_pAutoGearCar->field_0x1d8 = 0;
        }
    }
}

extern double g_unk0x00511300;

// Eases the car's steering angle towards the value taken from its track
// surface, then stores the damped angle into 0xb10/0xb12.
// match 87%: implemented; MSVC keeps the two scale factors in different registers and
// reorders the fixpoint intermediates
// FUNCTION: CMR2 0x00493ed0
void FUN_00493ed0(void)
{
    int scaleRight;
    int scaleLeft;
    int value;
    int other;
    short target;
    short current;
    short delta;

    scaleRight = *(int *)((BYTE *)g_pAutoGearCar + 0x7fc);
    scaleLeft = *(int *)((BYTE *)g_pAutoGearCar + 0x800);
    value = FixMul(*(BYTE *)((BYTE *)g_pAutoGearCar + 0x1d1) << 16, 0x410);
    if (value > 0x10000)
        value = 0x10000;
    other = FixMul(*(BYTE *)((BYTE *)g_pAutoGearCar + 0x1d0) << 16, 0x410);
    if (other > 0x10000)
        other = 0x10000;
    value -= other;
    FUN_004943d0();

    if (value < -0x10000)
        value = -0x10000;
    else if (value > 0x10000)
        value = 0x10000;
    if (value == 0)
        target = 0;
    else
        target = (short)(__int64)((double)FixMul(g_unk0x00592160, value) * g_unk0x00511300);

    if (*(int *)((BYTE *)g_pAutoGearCar + 0xb88) == 2) {
        *(short *)((BYTE *)g_pAutoGearCar + 0xb10) = target;
        *(short *)((BYTE *)g_pAutoGearCar + 0xb12) = *(short *)((BYTE *)g_pAutoGearCar + 0xb10);
        return;
    }

    current = *(short *)((BYTE *)g_pAutoGearCar + 0xb10);
    delta = current - target;
    if (delta > 0x800)
        delta = 0x1000 - delta;
    if (delta > 0x1f || delta < -0x1f) {
        if (current == 0)
            delta = (short)(__int64)((double)FixMul(delta * 0x1680, scaleRight) * g_unk0x00511300);
        else if (current < 1) {
            if (delta < 1)
                delta = (short)(__int64)((double)FixMul(delta * 0x1680, scaleLeft) * g_unk0x00511300);
            else
                delta = (short)(__int64)((double)FixMul(delta * 0x1680, scaleRight) * g_unk0x00511300);
        } else if (delta < 0)
            delta = (short)(__int64)((double)FixMul(delta * 0x1680, scaleRight) * g_unk0x00511300);
        else
            delta = (short)(__int64)((double)FixMul(delta * 0x1680, scaleLeft) * g_unk0x00511300);
    }
    *(short *)((BYTE *)g_pAutoGearCar + 0xb10) -= delta;
    *(short *)((BYTE *)g_pAutoGearCar + 0xb12) = *(short *)((BYTE *)g_pAutoGearCar + 0xb10);
}

// GLOBAL: CMR2 0x00592168
int g_unk0x00592168;
// GLOBAL: CMR2 0x00511308
double g_unk0x00511308 = -4096.0 / (360.0 * 65536.0);

// Integrates the auto-gear steering accumulator for the current surface and
// eases the 16.16 angle in 0xb10/0xb12 towards the new target.
// match 43%: implemented; the original materialises FixMul(<constant zero>, v) instead of
// testing zero, which moves the whole decision tree's register allocation
// FUNCTION: CMR2 0x00494110
void FUN_00494110(void)
{
    int v;
    int sum;
    int cur;
    int magCur;
    int magSum;
    short target;
    short step;
    short limit;

    FUN_004943d0();
    FUN_004945d0();
    g_unk0x00592168 = *(int *)((BYTE *)g_pAutoGearCar + 0x820);
    if (*(int *)((BYTE *)g_pAutoGearCar + 0x81c) != 0) {
        if (FixMul(g_unk0x00592160, *(int *)((BYTE *)g_pAutoGearCar + 0x81c)) > 0)
            g_unk0x00592168 = -g_unk0x00592168;
    } else {
        g_unk0x00592168 = 0;
    }

    v = 0x10000 - FixMul(*(int *)((BYTE *)g_pAutoGearCar + 0xb8) +
                         *(int *)((BYTE *)g_pAutoGearCar + 0x94), 0x8000);
    g_unk0x00592164 = FixMul(FixMul(g_unk0x00592164, v), *(int *)((BYTE *)g_pAutoGearCar + 0x804));
    g_unk0x00592168 = FixMul(FixMul(g_unk0x00592168, v), *(int *)((BYTE *)g_pAutoGearCar + 0x804));

    if (*(char *)((BYTE *)g_pAutoGearCar + 0x1d0) == 0) {
        if (*(char *)((BYTE *)g_pAutoGearCar + 0x1d1) == 0)
            goto modeB;
        goto modeA;
    }
    if (g_unk0x00592168 > 0)
        goto modeB;
modeA:
    if (*(char *)((BYTE *)g_pAutoGearCar + 0x1d1) == 0)
        goto towardZero;
    if (g_unk0x00592168 >= 0)
        goto towardNeg;
modeB:
    sum = g_unk0x00592164 + g_unk0x00592168;
    cur = *(int *)((BYTE *)g_pAutoGearCar + 0x81c);
    magCur = cur;
    if (magCur < 0)
        magCur = -magCur;
    magSum = sum;
    if (magSum < 0)
        magSum = -magSum;
    if (magCur <= magSum)
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = 0;
    else
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = cur + sum;
    goto convert;

towardNeg:
    *(int *)((BYTE *)g_pAutoGearCar + 0x81c) += g_unk0x00592164;
    if (*(int *)((BYTE *)g_pAutoGearCar + 0x81c) < -0x10000)
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = 0xffff0000;
    else if (*(int *)((BYTE *)g_pAutoGearCar + 0x81c) > 0)
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = 0;
    goto convert;

towardZero:
    if (*(char *)((BYTE *)g_pAutoGearCar + 0x1d0) == 0)
        goto convert;
    *(int *)((BYTE *)g_pAutoGearCar + 0x81c) += g_unk0x00592164;
    if (*(int *)((BYTE *)g_pAutoGearCar + 0x81c) > 0x10000)
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = 0x10000;
    else if (*(int *)((BYTE *)g_pAutoGearCar + 0x81c) < 0)
        *(int *)((BYTE *)g_pAutoGearCar + 0x81c) = 0;

convert:
    target = (short)(__int64)((double)FixMul(g_unk0x00592160,
                                             *(int *)((BYTE *)g_pAutoGearCar + 0x81c)) *
                              g_unk0x00511308);
    *(short *)((BYTE *)g_pAutoGearCar + 0xb12) = target;
    step = *(short *)((BYTE *)g_pAutoGearCar + 0xb12) - *(short *)((BYTE *)g_pAutoGearCar + 0xb10);
    limit = (short)(FixMul(0x22, 0x10000 - v) + 0x22);
    cur = step;
    if (cur < 0)
        cur = -cur;
    if (cur < limit) {
        *(short *)((BYTE *)g_pAutoGearCar + 0xb10) = *(short *)((BYTE *)g_pAutoGearCar + 0xb12);
        return;
    }
    if (step > 0)
        *(short *)((BYTE *)g_pAutoGearCar + 0xb10) += limit;
    else
        *(short *)((BYTE *)g_pAutoGearCar + 0xb10) -= limit;
}
