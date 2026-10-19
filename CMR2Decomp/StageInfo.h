#ifndef CMR2_STAGE_INFO_H
#define CMR2_STAGE_INFO_H

#include <windows.h>

// Share of one surface on a stage: shown as "<percent>% <surface name>".
struct StageSurfaceShare {
    int surface;        // 0x0 surface name text is a per-screen base + surface
    BYTE percent;       // 0x4
    BYTE pad[3];
};

// Facts of a stage (g_stageEntries) or of an event (g_eventEntries) shown on
// the stage and records screens: "<length>: <km>.<tenths> km" and up to four
// surfaces.
struct StageInfoEntry {
    BYTE lengthKm;                    // 0x00
    BYTE lengthTenths;                // 0x01
    BYTE surfaceCount;                // 0x02
    BYTE pad;
    StageSurfaceShare surfaces[4];    // 0x04
};

#endif
