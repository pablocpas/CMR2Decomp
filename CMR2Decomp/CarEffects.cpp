#include <windows.h>
#include <stdlib.h>
#include "Car.h"
#include "Graphics.h"
#include "Particle.h"
#include "Sprite.h"
#include "RallyData.h"
#include "StageTiming.h"

// Particle effects of the cars: breaking windows, glass shards and debris.

void Scene_GetLightColour(DWORD *pColour, int level);
unsigned int RallyData_FUN_00407e70(void);
int FUN_00457e10(BYTE *pCar, int offset);
short *Car_GetOrder(void);
short Car_GetOrderCount(void);
extern float g_oneOverRandMax;

#define EFFECT_RAND() (int)(__int64)((float)rand() * g_oneOverRandMax * (float)CGraphics::m_65536)

// A car window: four vertices of the car's window list, the Z of each
// mirrored for the other side of the car.
struct CarWindow {
    BYTE vertex[4];
    int mirrorZ[4];
};

// GLOBAL: CMR2 0x005203e0
CarWindow g_carWindows[6] = {
    {{0, 1, 1, 0}, {0, 0, 1, 1}}, {{4, 4, 3, 3}, {0, 1, 1, 0}}, {{0, 5, 2, 1}, {0, 0, 0, 0}},
    {{5, 4, 3, 2}, {0, 0, 0, 0}}, {{0, 1, 2, 5}, {1, 1, 1, 1}}, {{5, 2, 3, 4}, {1, 1, 1, 1}},
};

// GLOBAL: CMR2 0x00592870
BYTE g_windowSmashCount[8];         // queued window breaks per car
// Triangle of a spark streak.
// GLOBAL: CMR2 0x00592878
Quad2DInputVertex g_sparkTri[3];
// Random debris triangles.
// GLOBAL: CMR2 0x005928c0
FixVector g_debrisShapes[30][3];
// GLOBAL: CMR2 0x00592cf8
BYTE *g_carDamageData[8];           // damage section of each car's data
// GLOBAL: CMR2 0x00592d18
FixVector *g_carWindowVerts[8];     // window vertices of each car
// Triangle of a debris piece.
// GLOBAL: CMR2 0x00592d38
Quad2DInputVertex g_debrisTri[3];
// Small random triangles used as glass shards.
// GLOBAL: CMR2 0x00592d80
FixVector g_glassShards[10][3];
// GLOBAL: CMR2 0x00592ee8
int g_windowSmash[2][10];           // queued windows to break
// Break-up grid of a window (11 rows of 8 points).
// GLOBAL: CMR2 0x00592f38
FixVector g_windowGrid[11][8];
// GLOBAL: CMR2 0x00593358
FixVector g_windowSmashDir[2][10];  // impact direction of each queued break
// Triangle of a glass shard.
// GLOBAL: CMR2 0x00593718
Quad2DInputVertex g_shardTri[3];
// Colours of the debris pieces.
// GLOBAL: CMR2 0x005203d8
BYTE g_debrisColours[2][4] = {{0xff, 0xff, 0xff, 0xaa}, {0xff, 0, 0, 0xaa}};

// Prepares the effects for a stage: random glass shards and the window
// data of every car.
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00498dd0
void CarEffects_Init(void)
{
    int *p;
    FixVector *pShard;
    FixVector *pV;
    FixVector mid;
    FixVector c;
    short *pOrder;
    int n;
    int i;
    int car;

    p = (int *)g_sparkTri[0].colour;
    do {
        *p = 0xfffefefe;
        p += 6;
    } while (p < (int *)g_sparkTri[3].colour);
    pShard = g_glassShards[0];
    do {
        pV = pShard;
        for (i = 3; i != 0; i--) {
            pV->x = EFFECT_RAND();
            pV->y = EFFECT_RAND();
            pV->z = EFFECT_RAND();
            pV++;
        }
        mid.x = pShard[1].x - pShard[0].x;
        mid.y = pShard[1].y - pShard[0].y;
        mid.z = pShard[1].z - pShard[0].z;
        FixVecScale(&mid, &mid, 0x8000);
        mid.x += pShard[0].x;
        mid.y += pShard[0].y;
        mid.z += pShard[0].z;
        c.x = pShard[2].x - mid.x;
        c.y = pShard[2].y - mid.y;
        c.z = pShard[2].z - mid.z;
        FixVecScale(&c, &c, 0x8000);
        c.x += mid.x;
        c.y += mid.y;
        c.z += mid.z;
        pV = pShard;
        for (i = 3; i != 0; i--) {
            pV->x -= c.x;
            pV->y -= c.y;
            pV->z -= c.z;
            FixVecScale(pV, pV, 0x3333);
            pV++;
        }
        pShard += 3;
    } while (pShard < g_glassShards[10]);
    n = Car_GetOrderCount();
    pOrder = Car_GetOrder();
    for (i = n - 1; i >= 0; i--) {
        car = pOrder[i];
        g_carDamageData[car] = (BYTE *)FUN_00457e10((BYTE *)Car_Get(car), 2);
        g_carWindowVerts[car] = (FixVector *)(g_carDamageData[car] + 0xc);
    }
}

// Breaks a window of the car (quad corners in body space, pDir the impact
// direction): the window is cut into a grid and every cell throws a glass
// particle, faster near the impact and when the window faces the blow.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00499de0
void Car_ShatterWindow(FixVector *pQuad, FixVector *pDir, int window, Car *pCar)
{
    FixVector t;
    FixVector top;
    FixVector bottom;
    FixVector e;
    FixVector n;
    FixVector d;
    FixVector centre;
    FixVector u;
    FixVector v;
    FixVector pos;
    FixVector vel;
    FixVector *pRow;
    FixVector *pP;
    int param[2];
    int recip;
    int len;
    int facing;
    int r;
    int a;
    int b;
    int i;
    int j;

    param[0] = pCar->field_0xa70;
    pP = pQuad;
    for (i = 4; i != 0; i--) {
        FixMatrix_RotateVector(&t, pP, pCar->pWorld);
        *pP = t;
        pP++;
    }

    // Grid rows between the two side edges, then points along each row.
    top.x = pQuad[1].x - pQuad[0].x;
    top.y = pQuad[1].y - pQuad[0].y;
    top.z = pQuad[1].z - pQuad[0].z;
    bottom.x = pQuad[2].x - pQuad[3].x;
    bottom.y = pQuad[2].y - pQuad[3].y;
    bottom.z = pQuad[2].z - pQuad[3].z;
    i = 0;
    pRow = g_windowGrid[0];
    do {
        FixVecScale(&pRow[0], &top, FixMul(i << 16, 0x1999));
        pRow[0].x += pQuad[0].x;
        pRow[0].y += pQuad[0].y;
        pRow[0].z += pQuad[0].z;
        FixVecScale(&pRow[7], &bottom, FixMul(i << 16, 0x1999));
        pRow[7].x += pQuad[3].x;
        pRow[7].y += pQuad[3].y;
        i++;
        pRow[7].z += pQuad[3].z;
        pRow += 8;
    } while (pRow < g_windowGrid[11]);
    pRow = g_windowGrid[0];
    do {
        d.x = pRow[7].x - pRow[0].x;
        d.y = pRow[7].y - pRow[0].y;
        d.z = pRow[7].z - pRow[0].z;
        for (j = 1; j < 7; j++) {
            FixVecScale(&pRow[j], &d, FixMul(j << 16, 0x249b));
            pRow[j].x += pRow[0].x;
            pRow[j].y += pRow[0].y;
            pRow[j].z += pRow[0].z;
        }
        pRow += 8;
    } while (pRow < g_windowGrid[11]);

    // Centre, size and facing of the window.
    d.x = pQuad[0].x - pQuad[2].x;
    d.y = pQuad[0].y - pQuad[2].y;
    d.z = pQuad[0].z - pQuad[2].z;
    recip = FixDiv(0x10000, FixMul(FixVecLength(&d), 0x8000));
    FixVecScale(&centre, &d, 0x8000);
    centre.x += pQuad[2].x;
    centre.y += pQuad[2].y;
    centre.z += pQuad[2].z;
    e.x = pQuad[0].x - pQuad[1].x;
    e.y = pQuad[0].y - pQuad[1].y;
    e.z = pQuad[0].z - pQuad[1].z;
    FixVecCross(&d, &e, &d);
    d.y = 0;
    FixMatrix_RotateVector(&e, &d, pCar->pWorld);
    FIX_NORMALIZE_INTO(n, e);
    facing = FixMul(FixMul(FIX_ABS(FixVecDot(&n, pDir)), FixDiv(0x10000, 0x20000)), 0x6666) + 0x9999;
    if (facing > 0x10000)
        facing = 0x10000;

    // One particle per cell, at a random point of it.
    pRow = g_windowGrid[1];
    do {
        pP = pRow;
        for (i = 7; i != 0; i--) {
            u.x = pP[0].x - pP[-8].x;
            u.y = pP[0].y - pP[-8].y;
            u.z = pP[0].z - pP[-8].z;
            v.x = pP[-7].x - pP[-8].x;
            v.y = pP[-7].y - pP[-8].y;
            v.z = pP[-7].z - pP[-8].z;
            FixVecScale(&u, &u, EFFECT_RAND());
            FixVecScale(&v, &v, EFFECT_RAND());
            pos.y = pP[-8].y + u.y + v.y;
            pos.x = pP[-8].x + u.x + v.x;
            pos.z = pP[-8].z + u.z + v.z;
            t.x = pos.x - centre.x;
            t.y = pos.y - centre.y;
            t.z = pos.z - centre.z;
            len = FixVecLength(&t);
            if (len == 0) {
                t.x = 0;
                t.y = 0;
                t.z = 0;
            } else {
                FixVecScaleRecip(&t, &t, len);
            }
            len = FixMul(len, recip);
            if (len > 0xcccc)
                len = 0xcccc;
            r = FixMul(0x10000 - len, EFFECT_RAND());
            a = FixMul(r, FixMul(facing, 0x1999));
            b = FixMul(0x10000 - r, FixMul(facing, 0x1999));
            FixVecScale(&t, &t, b);
            FixVecScale(&vel, &n, a);
            vel.x += t.x;
            vel.y += t.y;
            vel.z += t.z;
            param[1] = (window == 7 || window == 6 ? 1 : 0) << 16;
            Particle_Spawn(0x1c, &pos, &vel, -0x40000, (int)&pCar->pNode0x71c->current, NULL, 0, (int)param,
                           *((BYTE *)pCar->pNode0x720 + 0x17c));
            pP++;
        }
        pRow += 8;
    } while (pRow < g_windowGrid[11]);
}

// Builds the quad of a window of the car from its window vertices.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049a870
int Car_BreakWindow(int window, FixVector *pDir, int unused, Car *pCar)
{
    FixVector *pVerts;
    FixVector quad[4];
    FixVector *pQ;
    int *pMirror;
    int i;

    i = 0;
    pVerts = NULL;
    if (window < 6)
        pVerts = g_carWindowVerts[pCar->field_0xb1a];
    pQ = quad;
    pMirror = g_carWindows[window].mirrorZ;
    do {
        *pQ = pVerts[g_carWindows[window].vertex[i]];
        if (*pMirror != 0)
            pQ->z = -pQ->z;
        i++;
        pMirror++;
        pQ++;
    } while (i < 4);
    Car_ShatterWindow(quad, pDir, window, pCar);
    return 1;
}

// Queues the windows broken by damage to a part of the car; the rear and
// side part ids break two windows each.
// match 37%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049a910
void Car_QueueWindowBreak(BYTE *pParts, Car *pCar, unsigned int part)
{
#define SMASH_SLOT (pCar->field_0xb1a * 10 + g_windowSmashCount[pCar->field_0xb1a])
    if (part == pParts[0x462]) {
        g_windowSmash[0][SMASH_SLOT] = 2;
        g_windowSmashDir[0][SMASH_SLOT] = pCar->field_0x5c4;
        g_windowSmashCount[pCar->field_0xb1a]++;
        g_windowSmash[0][SMASH_SLOT] = 3;
    } else if (part == pParts[0x463]) {
        g_windowSmash[0][SMASH_SLOT] = 4;
        g_windowSmashDir[0][SMASH_SLOT] = pCar->field_0x5c4;
        g_windowSmashCount[pCar->field_0xb1a]++;
        g_windowSmash[0][SMASH_SLOT] = 5;
    } else {
        if (part == pParts[0x460])
            g_windowSmash[0][SMASH_SLOT] = 0;
        else if (part == pParts[0x461])
            g_windowSmash[0][SMASH_SLOT] = 1;
        else
            return;
    }
    g_windowSmashDir[0][SMASH_SLOT] = pCar->field_0x5c4;
    g_windowSmashCount[pCar->field_0xb1a]++;
#undef SMASH_SLOT
}

// Breaks the queued windows of the car.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049ab20
void Car_BreakQueuedWindows(Car *pCar)
{
    int i;

    if (pCar->field_0xc0c == 0) {
        i = g_windowSmashCount[pCar->field_0xb1a];
        while (--i >= 0)
            Car_BreakWindow(g_windowSmash[pCar->field_0xb1a][i], &g_windowSmashDir[pCar->field_0xb1a][i], 1, pCar);
        g_windowSmashCount[pCar->field_0xb1a] = 0;
    }
}

// Texture coordinates of the effect triangles.
// FUNCTION: CMR2 0x00499020
void CarEffects_InitUVs(int unused1, int unused2)
{
    g_sparkTri[0].u = 0x8000;
    g_sparkTri[0].v = 0;
    g_sparkTri[1].u = 0xffff;
    g_sparkTri[1].v = 0xffff;
    g_sparkTri[2].u = 0;
    g_sparkTri[2].v = 0xffff;
    g_shardTri[0].u = 0;
    g_shardTri[0].v = 0;
    g_shardTri[1].u = 0xffff;
    g_shardTri[1].v = 0xffff;
    g_shardTri[2].u = 0;
    g_shardTri[2].v = 0xffff;
}

// Position of a particle: relative to its matrix when it has one.
#define PARTICLE_POS(pos, p)                                          \
    pos = (p)->vector0x1c;                                            \
    if ((p)->field0x40 != 0) {                                        \
        FixMatrix_GetPosition(&o, (FixMatrix *)(p)->field0x40);       \
        pos.x += o.x;                                                 \
        pos.y += o.y;                                                 \
        pos.z += o.z;                                                 \
    }

// Draw callback of a spark: a streak along its velocity, facing the view.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00499070
void Spark_Draw(Particle *p, ParticleType *pType, SceneNode *pView)
{
    FixVector pos;
    FixVector o;
    FixVector cam;
    FixVector d;
    FixVector side;
    FixVector dir;
    int len;

    if (p->age >= 0x130000)
        return;
    PARTICLE_POS(pos, p);
    FixMatrix_GetPosition(&cam, &pView->current);
    d.x = cam.x - pos.x;
    d.y = cam.y - pos.y;
    d.z = cam.z - pos.z;
    FixVecCross(&side, &d, &p->position);
    FIX_NORMALIZE_INTO(side, side);
    len = FixVecLength(&p->position);
    if (len < 0x599a) {
        dir = p->position;
    } else {
        FixVecScaleRecip(&dir, &p->position, len);
        FixVecScale(&dir, &dir, 0x5999);
        len = 0x5999;
    }
    FixVecScale(&side, &side, FixMul(len, 0x4000));
    g_sparkTri[0].x = FixMul(dir.x, 0x20000) + pos.x;
    g_sparkTri[0].y = FixMul(dir.y, 0x20000) + pos.y;
    g_sparkTri[0].z = FixMul(dir.z, 0x20000) + pos.z;
    g_sparkTri[2].x = side.x + pos.x;
    g_sparkTri[1].x = pos.x - side.x;
    g_sparkTri[2].y = side.y + pos.y;
    g_sparkTri[2].z = side.z + pos.z;
    g_sparkTri[1].z = pos.z - side.z;
    g_sparkTri[1].y = pos.y - side.y;
    g_sparkTri[0].colour[3] = p->type0x53;
    g_sparkTri[1].colour[3] = g_sparkTri[0].colour[3];
    g_sparkTri[2].colour[3] = g_sparkTri[0].colour[3];
    Quad2D_QueueFixedTriangle(0, &g_sparkTri[0], &g_sparkTri[1], &g_sparkTri[2], (Texture *)pType->field0x38,
                              (Quad2D *)0x14);
}

// Lit colour of a car piece: the car's paint scaled by the scene light.
#define EFFECT_LIT_COLOUR(out, level, pPaint)                                     \
    Scene_GetLightColour((DWORD *)light, (level));                                \
    lr = FixMul(light[0] << 16, 0x106);                                           \
    if (lr > 0x10000)                                                             \
        lr = 0x10000;                                                             \
    lg = FixMul(light[1] << 16, 0x106);                                           \
    if (lg > 0x10000)                                                             \
        lg = 0x10000;                                                             \
    lb = FixMul(light[2] << 16, 0x106);                                           \
    if (lb > 0x10000)                                                             \
        lb = 0x10000;

// Update callback: once old enough the particle switches to its type's look.
// FUNCTION: CMR2 0x004994d0
void FUN_004994d0(void *pParticle, ParticleType *pType, int param)
{
    Particle *p = (Particle *)pParticle;

    if (p->age >= 0x78000)
        p->type0x56 = pType->type;
}

// Draw callback of a glass shard, tinted like the car's windows.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004994f0
void GlassShard_Draw(Particle *p, ParticleType *pType, int unused)
{
    FixVector pos;
    FixVector o;
    FixVector *pShard;
    Quad2DInputVertex *pV;
    BYTE *pTint;
    BYTE light[4];
    int car;
    int shard;
    int tint;
    int lr;
    int lg;
    int lb;
    DWORD *pC;

    if (p->age >= 0x130000)
        return;
    pos = p->vector0x1c;
    FixMatrix_GetPosition(&o, (FixMatrix *)p->field0x40);
    car = p->field0x64 >> 8;
    shard = p->field0x64 - car * 0x100;
    tint = 0;
    if (shard > 2)
        tint = (shard > 5) + 1;
    pTint = g_carDamageData[car] + tint * 4;
    pShard = g_glassShards[shard];
    pV = g_shardTri;
    do {
        pV->x = pShard->x + pos.x + o.x;
        pV->y = pShard->y + pos.y + o.y;
        pV->z = pShard->z + pos.z + o.z;
        pV->colour[0] = pTint[0];
        pV->colour[1] = pTint[1];
        pV->colour[2] = pTint[2];
        pV->colour[3] = 0xff;
        pV++;
        pShard++;
    } while (pV < &g_shardTri[3]);
    EFFECT_LIT_COLOUR(light, p->size, pTint);
    light[2] = (BYTE)FixMulShift32(lb, g_carDamageData[car][2 + tint * 4] << 16);
    light[3] = 0xff;
    light[0] = (BYTE)FixMulShift32(lr, g_carDamageData[car][tint * 4] << 16);
    light[1] = (BYTE)FixMulShift32(lg, g_carDamageData[car][1 + tint * 4] << 16);
    pC = (DWORD *)g_shardTri[0].colour;
    do {
        *pC = *(DWORD *)light;
        pC += 6;
    } while (pC < (DWORD *)g_shardTri[3].colour);
    Quad2D_QueueFixedTriangle(0, &g_shardTri[0], &g_shardTri[1], &g_shardTri[2], (Texture *)pType->field0x38,
                              (Quad2D *)0x14);
}

// Spawn callback of a glass shard: remembers the car and picks a shard.
// FUNCTION: CMR2 0x00499710
void GlassShard_Init(Particle *p, ParticleType *pType, Car *pCar)
{
    p->field0x64 = (unsigned short)((BYTE)pCar->field_0xb1a << 8);
    p->field0x64 += (short)(rand() % 10);
    p->size = pCar->field_0xa70;
}

// Throws debris (and, a limited number of times per car, glass) from a
// point of the car: count pieces with random velocities along the given
// axes.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00499750
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance)
{
    FixVector off[3];
    FixVector vel;
    int ext[3];
    int *pRecord;
    int *pExt;
    FixVector *pOff;
    int glass;
    int n;
    int k;
    BYTE light;

    glass = 0;
    pRecord = FUN_00469680(pCar->field_0xb1a);
    if ((BYTE)RallyData_FUN_00407e70() != 0 || pCar->field_0xc0c != 0)
        return;
    n = (FixMul(count, EFFECT_RAND()) >> 16) + 1;
    ext[0] = FixMul(size, 0x3333);
    ext[1] = FixMul(size, 0x1999);
    ext[2] = FixMul(size, 0xccc);
    if (n <= 0)
        return;
    do {
        pExt = ext;
        pOff = off;
        for (k = 3; k != 0; k--) {
            FixVecScale(pOff, &pAxes[pOff - off], FixMul(EFFECT_RAND(), *pExt));
            pExt++;
            pOff++;
        }
        if (EFFECT_RAND() > 0x7fff) {
            off[0].y -= off[1].y;
            off[0].x -= off[1].x;
            off[0].z -= off[1].z;
        } else {
            off[0].y += off[1].y;
            off[0].x += off[1].x;
            off[0].z += off[1].z;
        }
        if (EFFECT_RAND() > 0x7fff) {
            vel.y = off[0].y - off[2].y;
            vel.x = off[0].x - off[2].x;
            vel.z = off[0].z - off[2].z;
        } else {
            vel.y = off[2].y + off[0].y;
            vel.x = off[2].x + off[0].x;
            vel.z = off[2].z + off[0].z;
        }
        if (EFFECT_RAND() < glassChance) {
            light = *((BYTE *)pCar->pNode0x720 + 0x17c);
            if (*((BYTE *)pRecord + 0x469) < 8) {
                Particle_Spawn(0x1b, pPos, &vel, pPos->y - 0x1999, (int)&pCar->pNode0x71c->current, NULL, 0,
                               (int)pCar, light);
                glass = 1;
            } else {
                Particle_Spawn(0x1a, pPos, &vel, pPos->y - 0x1999, (int)&pCar->pNode0x71c->current, NULL, 0, 0,
                               light);
                glass = 1;
            }
        } else {
            Particle_Spawn(0x1a, pPos, &vel, pPos->y - 0x1999, (int)&pCar->pNode0x71c->current, NULL, 0, 0,
                           *((BYTE *)pCar->pNode0x720 + 0x17c));
        }
    } while (--n != 0);
    if (glass != 0) {
        *(int *)((BYTE *)pRecord + 0x46c) = 1;
        if (*((BYTE *)pRecord + 0x469) < 0xfe)
            *((BYTE *)pRecord + 0x469) += 2;
    }
}

// Draw callback of a debris piece: a random triangle, lit, bulging
// towards the view.
// match 43%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00499ac0
void Debris_Draw(Particle *p, ParticleType *pType, SceneNode *pView)
{
    FixVector pos;
    FixVector o;
    FixVector cam;
    FixVector d;
    FixVector *pShape;
    Quad2DInputVertex *pV;
    BYTE light[4];
    DWORD *pC;
    int shape;
    int colour;
    int lr;
    int lg;
    int lb;
    int len;
    int k;

    shape = p->field0x64 >> 8;
    colour = p->field0x64 - shape * 0x100;
    EFFECT_LIT_COLOUR(light, p->size, NULL);
    light[2] = (BYTE)FixMulShift32(lb, g_debrisColours[colour][2] << 16);
    light[0] = (BYTE)FixMulShift32(lr, g_debrisColours[colour][0] << 16);
    light[1] = (BYTE)FixMulShift32(lg, g_debrisColours[colour][1] << 16);
    light[3] = 0xaa;
    pC = (DWORD *)g_debrisTri[0].colour;
    do {
        *pC = *(DWORD *)light;
        pC += 6;
    } while (pC < (DWORD *)g_debrisTri[3].colour);
    pos = p->vector0x1c;
    FixMatrix_GetPosition(&o, (FixMatrix *)p->field0x40);
    pV = g_debrisTri;
    pShape = g_debrisShapes[shape];
    do {
        pV->x = pShape->x + pos.x + o.x;
        pV->y = pShape->y + pos.y + o.y;
        pV->z = pShape->z + pos.z + o.z;
        pV++;
        pShape++;
    } while (pV < &g_debrisTri[3]);
    FixMatrix_GetPosition(&cam, &pView->current);
    pV = g_debrisTri;
    do {
        d.x = cam.x - pV->x;
        d.z = cam.z - pV->z;
        d.y = cam.y - pV->y;
        len = FixVecLength(&d);
        if (len > 0x18000) {
            k = FixDiv(0x18000, len);
            d.x = FixMul(d.x, k);
            d.y = FixMul(d.y, k);
            d.z = FixMul(d.z, k);
        }
        pV->x += d.x;
        pV->y += d.y;
        pV->z += d.z;
        pV++;
    } while (pV < &g_debrisTri[3]);
    Quad2D_QueueFixedTriangle(0, &g_debrisTri[0], &g_debrisTri[1], &g_debrisTri[2], (Texture *)pType->field0x38,
                              (Quad2D *)0x14);
}

// Makes the random debris triangles (at most 0.1 from their centre).
// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049ab90
void CarEffects_InitDebris(void)
{
    FixVector *pShape;
    FixVector *pV;
    FixVector mid;
    FixVector c;
    int len;
    int i;

    g_debrisTri[0].v = 0;
    g_debrisTri[1].u = 0;
    g_debrisTri[1].v = 0x10000;
    g_debrisTri[2].u = 0x10000;
    g_debrisTri[2].v = 0x10000;
    g_debrisTri[0].u = 0x8000;
    pShape = g_debrisShapes[0];
    do {
        pShape[0].x = EFFECT_RAND();
        pShape[0].y = EFFECT_RAND();
        pShape[0].z = EFFECT_RAND();
        pShape[1].x = EFFECT_RAND();
        pShape[1].y = EFFECT_RAND();
        pShape[1].z = EFFECT_RAND();
        pShape[2].x = EFFECT_RAND();
        pShape[2].y = EFFECT_RAND();
        pShape[2].z = EFFECT_RAND();
        mid.y = pShape[1].y - pShape[0].y;
        mid.x = pShape[1].x - pShape[0].x;
        mid.z = pShape[1].z - pShape[0].z;
        FixVecScale(&mid, &mid, 0x8000);
        mid.z += pShape[0].z;
        mid.x += pShape[0].x;
        c.x = pShape[2].x - mid.x;
        mid.y += pShape[0].y;
        c.y = pShape[2].y - mid.y;
        c.z = pShape[2].z - mid.z;
        FixVecScale(&c, &c, 0x8000);
        c.y += mid.y;
        c.x += mid.x;
        c.z += mid.z;
        pV = pShape;
        for (i = 3; i != 0; i--) {
            pV->x -= c.x;
            pV->y -= c.y;
            pV->z -= c.z;
            len = FixVecLength(pV);
            if (len > 0x1999)
                FixVecScale(pV, pV, FixDiv(0x1999, len));
            pV++;
        }
        pShape += 3;
    } while (pShape < g_debrisShapes[30]);
}

BYTE *RallyData_GetTyreRecord(BYTE index);
void RallyData_MarkTyresChanged(int index);
BYTE FUN_0041b370(void);
int FUN_00460c80(BYTE *pCar);
int Car_GetWheelSpeed(Car *pCar, BYTE wheel, int unit);

// Wheel spray effects: colour, whether it is dust (longer, flatter), whether
// it is thrown even on a slow surface, and the countdown range between
// particles.
// GLOBAL: CMR2 0x005206f8
DWORD g_sprayColour[13] = {0x003b4c5d, 0x005f6b78, 0x006f7576, 0x00065487, 0x0003446f, 0x00003a68, 0x00667daf,
                           0x00728abd, 0x00728abd, 0x006f7576, 0x006f7576, 0x0016262c, 0x004c5154};
// GLOBAL: CMR2 0x0052072c
int g_sprayDust[13] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
// GLOBAL: CMR2 0x00520760
int g_sprayAlways[13] = {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0};
// GLOBAL: CMR2 0x00520794
char g_sprayInterval[13][2] = {{10, 2}, {10, 2}, {4, 4}, {4, 4}, {2, 2}, {2, 2}, {4, 4},
                               {3, 3}, {2, 2}, {3, 3}, {2, 2}, {10, 2}, {10, 2}};

// GLOBAL: CMR2 0x00593860
short g_spraySurface;          // last surface looked up
// GLOBAL: CMR2 0x00593864
int g_sprayHalf;
// GLOBAL: CMR2 0x00593868
int g_sprayRange;
// GLOBAL: CMR2 0x0059386c
short g_sprayEffect;           // its effect
// GLOBAL: CMR2 0x00593870
int g_sprayRecip;
// GLOBAL: CMR2 0x00593874
char g_sprayCountdown[8][4];
// GLOBAL: CMR2 0x00593894
int g_sprayStep;
// GLOBAL: CMR2 0x00593898
int g_sprayThird;
// GLOBAL: CMR2 0x0059389c
BYTE g_sprayLife;

// Sets up the wheel spray for a spread and range.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049af20
int WheelSpray_Init(int spread, int range)
{
    FixVector v;
    int i;
    int *p;

    g_sprayRange = range;
    g_sprayStep = FixDiv(FixMul(spread, 0x40000), range);
    g_sprayHalf = FixMul(g_sprayStep, 0x8000);
    g_sprayThird = FixMul(g_sprayStep, 0x5553);
    v.x = g_sprayThird;
    v.y = g_sprayStep;
    v.z = g_sprayStep;
    g_sprayRecip = FixDiv(0x10000, FixVecLength(&v));
    g_sprayLife = (BYTE)(FixDiv(range, 0xc0000) >> 16);
    p = (int *)g_sprayCountdown;
    for (i = 8; i != 0; i--)
        *p++ = 0;
    g_sprayEffect = -1;
    g_spraySurface = -1;
    return FixDiv(FixMul(spread, 0x80000), FixMul(range, range));
}

// Spray effect thrown on a surface in the current country (-1 for none).
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049b0d0
int WheelSpray_GetEffect(int surface)
{
    if (surface != g_spraySurface) {
        g_sprayEffect = -1;
        switch ((BYTE)RallyDataCountryIndex()) {
        case 0:
            if (surface == 0xd) {
                g_sprayEffect = 2;
                g_spraySurface = surface;
                return 2;
            }
            if (surface == 0x13) {
                g_spraySurface = surface;
                g_sprayEffect = 1;
                return 1;
            }
            if (surface == 0x1a) {
                g_sprayEffect = 0;
                g_spraySurface = surface;
                return 0;
            }
            break;
        case 1:
            switch (surface) {
            case 3:
                g_sprayEffect = 6;
                g_spraySurface = surface;
                return 6;
            case 4:
            case 6:
                g_spraySurface = surface;
                g_sprayEffect = 7;
                return 7;
            case 5:
            case 7:
                g_sprayEffect = 8;
                g_spraySurface = surface;
                return 8;
            case 8:
                g_sprayEffect = 2;
                g_spraySurface = surface;
                return 2;
            case 9:
            case 0xb:
                g_sprayEffect = 9;
                g_spraySurface = surface;
                return 9;
            case 10:
            case 0xc:
                g_sprayEffect = 10;
                g_spraySurface = surface;
                return 10;
            case 0x1b:
                g_sprayEffect = 1;
                g_spraySurface = surface;
                return 1;
            }
            break;
        case 2:
            switch (surface) {
            case 0x1a:
            case 0x22:
                g_sprayEffect = 0;
                g_spraySurface = surface;
                return 0;
            case 0x1c:
            case 0x1d:
                g_sprayEffect = 1;
                g_spraySurface = surface;
                return 1;
            }
            break;
        case 4:
            if (surface == 2) {
                g_sprayEffect = 3;
                g_spraySurface = surface;
                return 3;
            }
            if (surface == 0x47) {
                g_sprayEffect = 5;
                g_spraySurface = surface;
                return 5;
            }
            if (surface == 0x53) {
                g_sprayEffect = 4;
                g_spraySurface = surface;
                return 4;
            }
            break;
        case 5:
            switch (surface) {
            case 3:
            case 8:
            case 0xd:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x1b:
            case 0x1c:
                g_sprayEffect = 3;
                g_spraySurface = surface;
                return 3;
            }
            break;
        case 7:
            switch (surface) {
            case 0xf:
                g_sprayEffect = 9;
                g_spraySurface = surface;
                return 9;
            case 0x13:
            case 0x14:
                g_sprayEffect = 0xb;
                g_spraySurface = surface;
                return 0xb;
            case 0x1a:
            case 0x1f:
                g_sprayEffect = 0xc;
            }
        }
    }
    g_spraySurface = surface;
    return g_sprayEffect;
}

// Adds wear to a tyre of a driver and updates its state (1 worn, 2 badly
// worn, 3 damaged).
// FUNCTION: CMR2 0x0045c750
void Tyre_AddWear(int car, int wheel, int damage, int wear)
{
    BYTE *pRecord;
    int *pState;
    int i;

    pRecord = RallyData_GetTyreRecord(FUN_0041b370() + (char)car);
    if (pRecord == NULL)
        return;
    if (((int *)(pRecord + 0x80))[wheel] < 1000)
        ((int *)(pRecord + 0x80))[wheel] += damage;
    if (((int *)(pRecord + 0x60))[wheel] < 1000)
        ((int *)(pRecord + 0x60))[wheel] += wear;
    if (((int *)(pRecord + 0x20))[wheel] < ((int *)(pRecord + 0x80))[wheel])
        ((int *)(pRecord + 0x20))[wheel] = ((int *)(pRecord + 0x80))[wheel];
    if (((int *)pRecord)[wheel] < ((int *)(pRecord + 0x60))[wheel])
        ((int *)pRecord)[wheel] = ((int *)(pRecord + 0x60))[wheel];
    ((int *)(pRecord + 0x90))[wheel] = 0;
    pState = (int *)(pRecord + 0x90);
    for (i = 4; i != 0; i--) {
        if (pState[-4] > 0x32)
            *pState = 3;
        if (pState[-0xc] > 0x32)
            *pState = 1;
        if (pState[-0xc] > 300)
            *pState = 2;
        pState++;
    }
    RallyData_MarkTyresChanged((FUN_0041b370() & 0xff) + car);
}

// Throws the spray of the car's sliding wheels (gravel, mud, dust, water)
// and wears the tyres on the loose surfaces.
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049b3e0
void WheelSpray_Update(int player)
{
    Car *pCar;
    FixVector *pEmit;
    int *pSlip;
    short *pSurface;
    FixVector p;
    FixVector world;
    FixVector vel;
    FixVector wv;
    FixVector t;
    FixVector basis[3];
    DWORD colour;
    int wheel;
    int effect;
    int wet;
    int *pDust;
    int speed;
    int slip;
    int lat;
    int ax;
    int az;
    int bx;
    int bz;
    int dx;
    int dz;
    int ux;
    int uz;
    int r;
    int ground;
    int side;
    int along;
    int up;
    int across;
    int s;
    int n;
    int level;
    int type;
    Car *pOther;
    BYTE light;

    pCar = Car_Get(player);
    if (pCar->field_0xb74 == 0)
        return;
    pEmit = pCar->wheelEmitter;
    pSlip = pCar->field_0x870;
    pSurface = pCar->wheelSurfaceType;
    wheel = 0;
    do {
        if (pSlip[0xcf] != 0) {
            effect = WheelSpray_GetEffect(*pSurface);
            wet = FUN_00460c80((BYTE *)pCar);
            if (effect >= 0 && (wet != 0 || g_sprayAlways[effect] != 0) &&
                --g_sprayCountdown[pCar->field_0xb1a][wheel] < 1) {
                pDust = &g_sprayDust[effect];
                if (g_sprayDust[effect] == 0) {
                    if (Car_GetWheelSpeed(Car_Get(player), wheel, 0) < 0)
                        speed = -Car_GetWheelSpeed(Car_Get(player), wheel, 0);
                    else
                        speed = Car_GetWheelSpeed(Car_Get(player), wheel, 0);
                    if (speed > 0x1e0000)
                        Tyre_AddWear(pCar->field_0xb1a, wheel, 0, 1);
                }
                g_sprayCountdown[pCar->field_0xb1a][wheel] =
                    (char)FixMulShift32((g_sprayInterval[effect][1] - g_sprayInterval[effect][0]) << 16, wet) +
                    g_sprayInterval[effect][0];
                if (pSlip[0xde] == 0)
                    slip = FixMul(*pSlip, 0x4ccc);
                else
                    slip = *pSlip;
                lat = pSlip[4];
                if (pSlip[0xde] != 0 && FIX_ABS(lat) + FIX_ABS(slip) > 0x5999) {
                    if (wheel < 2) {
                        ax = FixMul(slip, g_sinTable[(pCar->heading + 0x400) & 0xfff]);
                        az = FixMul(slip, -g_sinTable[pCar->heading & 0xfff]);
                        bx = FixMul(lat, g_sinTable[pCar->heading & 0xfff]);
                        bz = FixMul(lat, g_sinTable[(pCar->heading + 0x400) & 0xfff]);
                        if (*pDust != 0) {
                            ax = FixMul(ax, 0x20000);
                            az = FixMul(az, 0x20000);
                            bx = FixMul(bx, 0x8000);
                            bz = FixMul(bz, 0x8000);
                        }
                        dx = bx + ax;
                        dz = bz + az;
                    } else {
                        dx = slip;
                        dz = lat;
                        if (*pDust != 0) {
                            dx = FixMul(slip, 0x20000);
                            dz = FixMul(lat, 0x8000);
                        }
                    }
                    r = FixDiv(0x10000, FixSqrt(FixMul(dz, dz) + FixMul(dx, dx)));
                    ux = FixMul(r, dx);
                    uz = FixMul(r, dz);
                    basis[0].x = ux;
                    basis[0].y = 0;
                    basis[0].z = uz;
                    basis[1].x = 0;
                    basis[1].y = 0x10000;
                    basis[1].z = 0;
                    basis[2].x = -uz;
                    basis[2].y = 0;
                    basis[2].z = ux;
                    p.x = pEmit->x;
                    p.y = pEmit->y - 0x5999;
                    p.z = pEmit->z;
                    FixMatrix_RotateVector(&world, &p, pCar->pWorld);
                    ground = world.y - 0x1999;
                    side = wheel % 2;
                    along = g_sprayStep - FixMul(EFFECT_RAND(), g_sprayHalf);
                    up = g_sprayStep - FixMul(EFFECT_RAND(), g_sprayHalf);
                    across = g_sprayThird - FixMul(EFFECT_RAND(), g_sprayThird);
                    if (EFFECT_RAND() > 0x8000)
                        across = -across;
                    s = FixMul(FixMul(FixSqrt(FixMul(lat, lat) + FixMul(slip, slip)), 0x8000), g_sprayRecip);
                    if (s > 0x10000)
                        s = 0x10000;
                    along = FixMul(along, s);
                    up = FixMul(up, s);
                    across = FixMul(across, s);
                    if (*pDust != 0) {
                        along = FixMul(along, 0x20000);
                        up = FixMul(up, 0x8000);
                        across = FixMul(across, 0xb333);
                    }
                    vel.x = 0;
                    vel.y = 0;
                    vel.z = 0;
                    FixVecScale(&t, &basis[0], along);
                    vel.x += t.x;
                    vel.y += t.y;
                    vel.z += t.z;
                    FixVecScale(&t, &basis[1], up);
                    vel.x += t.x;
                    vel.y += t.y;
                    vel.z += t.z;
                    FixVecScale(&t, &basis[2], across);
                    vel.x += t.x;
                    vel.y += t.y;
                    vel.z += t.z;
                    n = 0;
                    if (wheel < 2) {
                        if (vel.x < 0)
                            n = 1;
                    } else if (vel.x > 0) {
                        n = 1;
                    }
                    if (side == 0) {
                        if (vel.z < 0)
                            n++;
                    } else if (vel.z > 0) {
                        n++;
                    }
                    if (n < 2) {
                        level = pCar->field_0xa70;
                        FixMatrix_RotateVector(&wv, &vel, pCar->pWorld);
                        colour = g_sprayColour[effect];
                        light = *((BYTE *)pCar->pNode0x720 + 0x17c);
                        type = *pDust == 0 ? 0x1e : 0x1d;
                        Particle_Spawn(type, &world, &wv, ground, (int)&pCar->pNode0x71c->current, (BYTE *)&colour,
                                       (BYTE)(-1 - (char)((unsigned int)(level * 0xff) >> 16)), 0, light);
                    }
                }
            }
        }
        wheel++;
        pSurface++;
        pSlip++;
        pEmit++;
    } while (wheel < 4);
}

#include "WheelTrail.h"

BYTE FUN_00460bf0(int index);
int FUN_00460c10(int index);
extern BYTE g_unk0x00538d2c[0xc8];

#define EFFECT_RAND_NEG() (int)(__int64)((float)rand() * g_oneOverRandMax * (float)g_minus65536)
extern double g_minus65536;

// Colour of the wheel splashes and its brightened copy.
// GLOBAL: CMR2 0x005435c0
BYTE g_unk0x005435c0[4];
// GLOBAL: CMR2 0x005435cc
BYTE g_unk0x005435cc[4];
// GLOBAL: CMR2 0x00543808
BYTE g_unk0x00543808;

// Sets the splash colour of a player's car (and its average brightness).
// match 21%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045d1e0
void FUN_0045d1e0(int player, BYTE *pColour)
{
    if (player < 8) {
        *(DWORD *)g_unk0x005435c0 = *(DWORD *)pColour;
        g_unk0x005435cc[1] = (g_unk0x005435c0[1] >> 1) + 0x7f;
        g_unk0x005435cc[0] = (g_unk0x005435c0[0] >> 1) + 0x7f;
        g_unk0x005435cc[2] = (g_unk0x005435c0[2] >> 1) + 0x7f;
        g_unk0x005435cc[3] = g_unk0x005435c0[3];
        g_unk0x00543808 = (BYTE)((pColour[2] + pColour[1] + pColour[0]) / 3);
    }
}

// Update callback: lifts the particle by half its size.
// FUNCTION: CMR2 0x0045d250
void FUN_0045d250(void *pParticle, ParticleType *pType, int param)
{
    Particle *p = (Particle *)pParticle;

    p->vector0x28.y += p->size / 2;
}

// Update callback: moves the particle with the car stored in its effect data.
// FUNCTION: CMR2 0x0045dea0
void FUN_0045dea0(void *pParticle, ParticleType *pType, int param)
{
    Particle *p = (Particle *)pParticle;
    int car = p->field0x64;

    if (car < 8) {
        p->vector0x28.x += Car_Get(car)->position.x;
        p->vector0x28.y += Car_Get(car)->position.y;
        p->vector0x28.z += Car_Get(car)->position.z;
    }
}

// Update callback: rises by half its size (capped at 1.5) and follows its car.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045d270
void FUN_0045d270(void *pParticle, ParticleType *pType, int param)
{
    Particle *p = (Particle *)pParticle;
    Car *pCar;

    p->vector0x28.y += p->size / 2;
    if (p->size > 0x18000)
        p->size = 0x18000;
    if (p->field0x64 < 8) {
        pCar = Car_Get(p->field0x64);
        p->vector0x28.x += pCar->position.x;
        p->vector0x28.y += pCar->position.y;
        p->vector0x28.z += pCar->position.z;
    }
}

// Exhaust emitter offsets of each car, rotated into the world.
// GLOBAL: CMR2 0x00543380
FixVector g_unk0x00543380[8];
// GLOBAL: CMR2 0x00543cf8
int g_unk0x00543cf8;
extern int g_trailTextureA[8][2];
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);

// Spawn callback: places the particle at one of the car's two exhausts
// (alternating) in world space.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045d2d0
void FUN_0045d2d0(void *pParticle, ParticleType *pType, int param)
{
    Particle *p = (Particle *)pParticle;
    int car = p->field0x64;
    Car *pCar;
    FixMatrix *pM;
    int x;
    int y;
    int z;

    if (car < 8) {
        if (g_trailTextureA[car][1] == 0)
            g_unk0x00543cf8 = 0;
        else
            g_unk0x00543cf8 = (g_unk0x00543cf8 + 1) % 2;
        pCar = Car_Get(car);
        FixMatrix_RotateVector(&g_unk0x00543380[car], (FixVector *)g_trailTextureA[car][g_unk0x00543cf8], pCar->pBodyMatrix);
        pM = pCar->pBodyMatrix;
        x = p->vector0x28.x + pM->position.x;
        p->vector0x28.x = x;
        y = p->vector0x28.y + pM->position.y;
        z = p->vector0x28.z + pM->position.z;
        p->vector0x28.y = y;
        p->vector0x28.z = z;
        p->vector0x28.x = g_unk0x00543380[car].x + x;
        p->vector0x28.y = g_unk0x00543380[car].y + y;
        p->vector0x28.z = g_unk0x00543380[car].z + z;
    }
}

// Spawn callback: stores the effect data passed by the spawner.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045de80
void FUN_0045de80(void *pParticle, ParticleType *pType, int param)
{
    ((Particle *)pParticle)->field0x64 = (short)*(int *)param;
}

// Splashes and sparks thrown along the wheel trails of a player's car:
// water on the wet surfaces, sparks on some hard ones (when the stage asks
// for them), each particle placed at a random point of the trail between
// the two wheels of the axle.
// match 35%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045c820
void WheelSplash_Update(int player)
{
    Car *pCar;
    FixVector *pVel;
    int *pOnGround;
    short *pSurface;
    FixVector seg;
    FixVector off;
    FixVector p;
    FixVector q;
    FixVector pos;
    FixVector rel;
    FixVector vel;
    FixVector fwd;
    BYTE colour[4];
    int wheel;
    int other;
    int front;
    int leading;
    int wet;
    int sparks;
    int emitting;
    int on;
    int type;
    int count;
    int speed;
    int a;
    int k;
    int f48;
    FixVector *pPos;
    short surface;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    if (player >= 8)
        return;
    pCar = Car_Get(player);
    emitting = 0;
    pVel = pCar->cornerVelocity;
    pOnGround = pCar->field_0xbac;
    pSurface = pCar->wheelSurface;
    wheel = 0;
    do {
        on = 0;
        type = 0;
        front = wheel == 0 || wheel == 2 ? 1 : 0;
        seg.x = g_trailPos[player][wheel].x - g_trailLastPos[player][wheel].x;
        seg.y = g_trailPos[player][wheel].y - g_trailLastPos[player][wheel].y;
        other = wheel ^ 1;
        seg.z = g_trailPos[player][wheel].z - g_trailLastPos[player][wheel].z;
        if (pCar->field_0xb1e == 7)
            leading = (wheel & 2) != 0;
        else
            leading = (wheel & 2) == 0;
        surface = *pSurface;
        wet = surface == 0xe || surface == 0xf;
        sparks = surface == 0x19 || surface == 0x18 || surface == 0x2a;
        if (FUN_00460bf0(player) == 1) {
            k = FixMul(FUN_00460c10(player), 0xff0000) >> 16;
            if (k > 0x1e && sparks) {
                colour[3] = (BYTE)k;
                type = 0xc;
                emitting = 0;
            }
        }
        count = 1;
        if (wet) {
            emitting = 1;
            colour[3] = 0xff;
            FixMatrix_GetForward(&fwd, (FixMatrix *)(g_unk0x00538d2c + 4 + player * 100));
            if (FixVecDot(&pCar->right, &fwd) < 0)
                front = front == 0;
            type = 0xf - (front != 0);
            if (EFFECT_RAND() < 0x8000) {
                if (leading)
                    count = 0;
                else
                    type = (EFFECT_RAND() > 0x7fff) + 0xe;
            }
            on = 1;
        } else if (emitting) {
            on = 1;
        }
        if (*pOnGround == 0)
            on = 0;
        if (Car_GetWheelSpeed(pCar, 0, 0) < 0)
            speed = -Car_GetWheelSpeed(pCar, 0, 0);
        else
            speed = Car_GetWheelSpeed(pCar, 0, 0);
        if (speed < 0x1e0000 && FixDiv(speed, 0x1e0000) < EFFECT_RAND())
            on = 0;
        if ((player < 1 || EFFECT_RAND() <= 0x8000) && on && count > 0) {
            do {
                FixVecScale(&off, &seg, EFFECT_RAND_NEG());
                if (leading || !wet) {
                    a = FixDiv(EFFECT_RAND(), 0x1547a - EFFECT_RAND_NEG() * 9);
                    a = FixDiv(a, 0x30000 - EFFECT_RAND_NEG() * 14);
                    FixVecScale(&p, &g_trailPos[player][wheel], a);
                    FixVecScale(&q, &g_trailPos[player][other], 0x10000 - a);
                    pos.x = p.x + q.x + off.x;
                    pos.y = p.y + q.y + off.y;
                    pos.z = p.z + q.z + off.z;
                } else {
                    if (type == 0xe || type == 0xf) {
                        a = FixDiv(EFFECT_RAND(), 0x1547a - EFFECT_RAND_NEG() * 9);
                        a = FixDiv(a, 0x30000 - EFFECT_RAND_NEG() * 14);
                    } else {
                        a = FixDiv(EFFECT_RAND(), 0x1547a - EFFECT_RAND_NEG() * 9);
                    }
                    FixVecScale(&p, &g_trailPos[player][wheel], a);
                    FixVecScale(&q, &g_trailPos[player][other], 0x10000 - a);
                    pos.x = p.x + q.x;
                    pos.y = p.y + q.y;
                    pos.z = p.z + q.z;
                }
                rel.x = pos.x - pCar->position.x;
                rel.y = pos.y - pCar->position.y;
                rel.z = pos.z - pCar->position.z;
                vel.y = pVel->y / 10;
                if (type == 0xe || type == 0xf) {
                    vel.x = pVel->x / -5;
                    vel.z = pVel->z / -5;
                } else if (wheel == 2 || wheel == 3) {
                    vel.x = pVel->x / -3;
                    vel.z = pVel->z / -3;
                } else {
                    vel.x = pVel->x * -2 / 3;
                    vel.z = pVel->z * -2 / 3;
                }
                speed = Car_GetWheelSpeed(pCar, 2, 0);
                if (speed > 0x1e0000) {
                    k = FixDiv(0x1e0000, speed);
                    vel.x = FixMul(vel.x, k);
                    vel.y = FixMul(vel.y, k);
                    vel.z = FixMul(vel.z, k);
                }
                if (type == 0xd) {
                    vel.x = pVel->x / 3;
                    vel.y = pVel->y / 3;
                    vel.z = pVel->z / 3;
                    pPos = &pos;
                    f48 = pos.y - 0x630000;
                } else {
                    pPos = &rel;
                    f48 = rel.y - 0x10000;
                }
                Particle_Spawn(type, pPos, &vel, f48, 0, colour, g_trailLevel[player][wheel], (int)&player,
                               *((BYTE *)pCar->pNode0x720 + 0x17c));
            } while (--count != 0);
            count = 0;
        }
        wheel++;
        pSurface++;
        pOnGround++;
        pVel++;
    } while (wheel < 4);
}

// Spawn callback of a debris piece or glass fragment: a random shape and
// the colour passed by the emitter ({light level, colour index << 16}).
// FUNCTION: CMR2 0x00499a80
void Debris_Init(Particle *p, ParticleType *pType, int *pParam)
{
    int shape;

    shape = (rand() % 30) << 8;
    p->field0x64 = (short)shape;
    p->field0x64 = (short)((pParam[1] >> 16) + shape);
    p->size = pParam[0];
}
