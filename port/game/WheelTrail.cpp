#include <windows.h>
#include "WheelTrail.h"
#include "Car.h"

int g_trailTimer[8][4];
char g_trailLevel[8][4];
int g_trailReset[8][4];
FixVector g_trailPos[8][4];
FixVector g_trailLastPos[8][4];
int g_trailSurface[8][4];
int g_unk0x00543708[8][4];
int g_trailState[8][4];
FixVector g_trailOffset[8][4];
int g_trailCount[8];
int g_trailPrevState[8][4];
FixVector g_trailDelta[8][4];

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// Moves each wheel's dust emitter to its new world position, remembers how far
// the wheel has travelled since the last trail mark and counts the wheels that
// are currently laying one.
// FUNCTION: CMR2 0x0045def0
void WheelTrail_Update(int carIndex)
{
    if (carIndex < 8) {
        Car *pCar = Car_Get(carIndex);
        g_trailCount[carIndex] = 0;
        int i = 0;
        FixVector delta;
        int px;

        do {
            if (g_trailReset[carIndex][i] != 0) {
                g_trailLastPos[carIndex][i].x = g_trailPos[carIndex][i].x;
                g_trailLastPos[carIndex][i].y = g_trailPos[carIndex][i].y;
                g_trailLastPos[carIndex][i].z = g_trailPos[carIndex][i].z;
                g_trailReset[carIndex][i] = 0;
            } else {
                delta.x = g_trailLastPos[carIndex][i].x - g_trailPos[carIndex][i].x;
                delta.y = 0;
                delta.z = g_trailLastPos[carIndex][i].z - g_trailPos[carIndex][i].z;
                if (FIX_ABS(FixVecLength(&delta)) > 0x14ccc) {
                    g_trailLastPos[carIndex][i].x = g_trailPos[carIndex][i].x;
                    g_trailLastPos[carIndex][i].y = g_trailPos[carIndex][i].y;
                    g_trailLastPos[carIndex][i].z = g_trailPos[carIndex][i].z;
                    g_trailDelta[carIndex][i].x = 0;
                    g_trailDelta[carIndex][i].y = 0;
                    g_trailDelta[carIndex][i].z = 0;
                    g_trailReset[carIndex][i] = 1;
                } else {
                    if (FIX_ABS(FixVecLength(&delta)) > 0x1999) {
                        g_trailLastPos[carIndex][i].x = g_trailPos[carIndex][i].x;
                        g_trailLastPos[carIndex][i].y = g_trailPos[carIndex][i].y;
                        g_trailLastPos[carIndex][i].z = g_trailPos[carIndex][i].z;
                        g_trailDelta[carIndex][i].x = delta.x;
                        g_trailDelta[carIndex][i].y = delta.y;
                        g_trailDelta[carIndex][i].z = delta.z;
                    }
                }
            }

            g_trailState[carIndex][i] = pCar->cornerOnGround[i];
            g_trailPrevState[carIndex][i] = g_unk0x00543708[carIndex][i];
            g_trailSurface[carIndex][i] = pCar->wheelSurface[i];
            if (pCar->cornerOnGround[i] == 1) {
                g_trailTimer[carIndex][i]++;
                g_trailCount[carIndex]++;
            } else {
                g_trailTimer[carIndex][i] = 0;
            }

            delta = pCar->wheelEmitter[i];
            delta.y -= 0x5999;
            FixMatrix_RotateVector(&g_trailOffset[carIndex][i], &delta, pCar->pWorld);
            px = pCar->position.x;
            g_trailPos[carIndex][i].x = g_trailOffset[carIndex][i].x + px;
            g_trailPos[carIndex][i].y = g_trailOffset[carIndex][i].y + pCar->position.y;
            g_trailPos[carIndex][i].z = g_trailOffset[carIndex][i].z + pCar->position.z;
            {
                int level;

                level = 0xff00 - pCar->field_0xa70;
                if (level > 0xff00) {
                    level = 0xff00;
                } else {
                    if (level < 0) {
                        level = 0;
                    }
                }
                g_trailLevel[carIndex][i] = (char)(level / 256);
            }
            i++;
        } while (i < 4);
    }
}
