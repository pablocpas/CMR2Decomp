#ifndef _SOUND_CURVE_H
#define _SOUND_CURVE_H

#include <windows.h>

// Runtime sound lookup: count intervals, inclusive input range, and the
// count+1 pitch/volume samples assigned by the stage initializer.
struct SoundCurve {
    int count;
    int min;
    int max;
    unsigned short *pSamples;
    BYTE *pBytes;
};

#endif
