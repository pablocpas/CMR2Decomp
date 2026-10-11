#ifndef _RACE_CALLBACK_STORAGE_H
#define _RACE_CALLBACK_STORAGE_H

#include "CallbackState.h"

// Runtime storage owned by the in-race machine; never a file payload.
struct RaceCallbackStorage {
    CallbackStateMachine machine;  // 0x00
    CallbackStateRecord records[2]; // 0x10 on Win32
};

extern RaceCallbackStorage g_raceCallbackStorage;

#endif
