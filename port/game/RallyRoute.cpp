#include <windows.h>
#include "RallyRoute.h"

short g_routeDirKey[3];
BYTE *g_routeNodes;
short g_routeDirCount;
FixVector g_routeDir[3];

// FUNCTION: CMR2 0x00421570
void RallyRoute_CopyIndexedNodeDirection(unsigned int nodeIndex, FixVector *pOut)
{
    RallyRoute_GetNodeDirection(pOut, nodeIndex);
}

// Direction of the route at one node: the average of the directions to the
// previous and the next node, cached for the last three nodes asked for.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004201d0
void RallyRoute_GetNodeDirection(FixVector *pOut, unsigned int nodeIndex)
{
    int slot = -1;
    int hasNext = 0;
    int hasPrev = 0;
    int i = 0;
    int *pNode;
    int *pPrev;
    int *pNext;
    FixVector sum;
    FixVector d;
    int len;
    int index;

    do {
        if (nodeIndex == (unsigned int)(int)g_routeDirKey[i]) {
            slot = i;
            i = 3;
        }
        i++;
    } while (i < 3);

    if (slot >= 0) {
        *pOut = g_routeDir[slot];
        return;
    }

    pNode = (int *)(g_routeNodes + nodeIndex * 0x2c);
    if ((int)nodeIndex > 0) {
        hasPrev = 1;
        pPrev = pNode - 0xb;
    }
    if (nodeIndex < (unsigned int)(g_unk0x00538a84 - 1)) {
        hasNext = 1;
        pNext = (int *)(g_routeNodes + nodeIndex * 0x2c + 0x2c);
    }

    sum.x = 0;
    sum.y = 0;
    sum.z = 0;
    if (hasPrev != 0) {
        d.y = 0;
        d.x = pNode[0] - pPrev[0];
        d.z = pNode[2] - pPrev[2];
        len = FixVecLength(&d);
        if (len == 0) {
            d.x = 0;
            d.y = 0;
            d.z = 0;
        } else {
            FixVecScaleRecip(&d, &d, len);
        }
        sum.x = sum.x + d.x;
        sum.z = sum.z + d.z;
    }
    if (hasNext != 0) {
        d.y = 0;
        d.x = pNext[0] - pNode[0];
        d.z = pNext[2] - pNode[2];
        len = FixVecLength(&d);
        if (len == 0) {
            d.x = 0;
            d.y = 0;
            d.z = 0;
        } else {
            FixVecScaleRecip(&d, &d, len);
        }
        sum.x = sum.x + d.x;
        sum.z = sum.z + d.z;
    }

    len = FixVecLength(&sum);
    if (len == 0) {
        sum.x = 0;
        sum.y = 0;
    } else {
        FixVecScaleRecip(&sum, &sum, len);
    }

    index = g_routeDirCount;
    g_routeDirKey[index] = (short)nodeIndex;
    g_routeDirCount = (short)(index + 1);
    g_routeDir[index].x = sum.x;
    g_routeDir[index].y = sum.y;
    g_routeDir[index].z = sum.z;
    if (g_routeDirCount > 2) {
        g_routeDirCount = (short)(g_routeDirCount - 3);
    }
    pOut->x = sum.x;
    pOut->y = sum.y;
    pOut->z = sum.z;
}
