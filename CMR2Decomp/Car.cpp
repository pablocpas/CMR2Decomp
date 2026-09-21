#include <windows.h>
#include "Car.h"
#include "FileBuffer.h"
#include "Game.h"

Car *g_cars[64];
int g_carCount;
Car *g_carBuffer;
Car *g_pCurrentCar;

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

    FixVecScale(&a, &g_pCurrentCar->vec0x360, g_pCurrentCar->scale0x764);
    FixVecScale(&b, &g_pCurrentCar->vec0x378, g_pCurrentCar->scale0x76c);
    FixVecScale(&c, &g_pCurrentCar->vec0x360, g_pCurrentCar->scale0x768);
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
