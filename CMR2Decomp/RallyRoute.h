#ifndef _RALLY_ROUTE_H
#define _RALLY_ROUTE_H

#include "FixedPoint.h"
#include "RallyRouteNode.h"

// Route nodes of the current stage (0x2c bytes each) and the three-entry cache
// of node directions built by RallyRoute_GetNodeDirection.

// GLOBAL: CMR2 0x00538a8c
extern short g_routeDirKey[3];
// GLOBAL: CMR2 0x00538a9c
extern RallyRouteNode *g_routeNodes;
// GLOBAL: CMR2 0x00538aa0
extern short g_routeDirCount;
// GLOBAL: CMR2 0x00538b68
extern FixVector g_routeDir[3];

extern int g_routeNodeCount;

void RallyRoute_GetNodeDirection(FixVector *pOut, unsigned int nodeIndex);

#endif
