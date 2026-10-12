#ifndef _PAUSEMENUS_H
#define _PAUSEMENUS_H

#include "Menu.h"

// The in-race pause screen (0x4d4 bytes at 0x58ca90): its two menus and the
// fade timers of the knockout bracket drawn over it.
struct PauseMenus {
    Menu main;                  // 0x000 entries 0x13 and 0x63 (opens stageData)
    BYTE bracketTimer;          // 0x1e0 timer slot of the bracket fade
    BYTE pad_0x1e1[3];
    int bracketFade;            // 0x1e4 its current value, 16.16
    Menu stageData;             // 0x1e8 stage data panel, 0x72 exits
    int textFade;               // 0x3c8 value of textTimer, 16.16
    unsigned int *roundEntry;   // 0x3cc RallyData_GetRoundEntry() when it started
    BYTE field_0x3d0[0x100];
    BYTE textTimer;             // 0x4d0 timer slot of the text fade
    BYTE pad_0x4d1[3];
};

#endif
