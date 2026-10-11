#ifndef _AI_NETWORK_H
#define _AI_NETWORK_H

#include <windows.h>

// ON-DISK: a driving network of the stage's ai0/ai1/ai2.dat (0x20 bytes).
// AI_LoadStageRouteTables (0x47c2f0) stores in place, in pWeights[1..], the
// address of each layer's weights (they follow the records in the file), so
// the record holds pointers only on Win32.
struct AINetwork {
    char inputs[8];             // 0x00 [1..layerSizes[0]] car value selected by each input
    int layerCount;             // 0x08
    char layerSizes[4];         // 0x0c neurons per layer
    int *pWeights[4];           // 0x10 [k]: (layerSizes[k - 1] + 1) * layerSizes[k] weights
};

#endif
