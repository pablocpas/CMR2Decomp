#include <windows.h>
#include "FixedPoint.h"

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
// TODO: CMR2 0x00490f20 (implemented, match 56%)
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
// TODO: CMR2 0x004910f0 (implemented, match 78%)
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
// TODO: CMR2 0x004916a0 (implemented, match 51%)
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
// TODO: CMR2 0x00491550 (implemented, match 39%)
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
