// Overrides used only by the standalone stage probe. Normal play uses SDL.
#ifndef OPENCMR2_PLATFORM_TESTING_H
#define OPENCMR2_PLATFORM_TESTING_H
#include "port/types.h"
struct PlatformTestClock {
    DWORD (*ticks)();
    void (*sleep)(DWORD);
    unsigned int unixTime;
};
void Platform_SetTestClock(const PlatformTestClock *clock);
void Platform_SetTestKeyboard(const BYTE *keys); // 256 DIK entries, NULL disables
#endif
