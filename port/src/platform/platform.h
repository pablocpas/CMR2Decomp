// Internal interface between the src/ subsystems (not used by game code).
#ifndef OPENCMR2_PLATFORM_H
#define OPENCMR2_PLATFORM_H

#include <SDL3/SDL.h>

// Sets up SDL, the directories and settings. Returns false (after telling
// the user) when the game data cannot be found.
bool Platform_Init(int argc, char **argv);
void Platform_Shutdown(void);

SDL_Window *Platform_GetWindow(void);
const char *Platform_GetDataDir(void);
const char *Platform_GetUserDir(void);

// OpenCMR2 settings (opencmr2.ini in the user directory): "section.key".
const char *Platform_GetSetting(const char *key, const char *fallback);
int Platform_GetSettingInt(const char *key, int fallback);

// DirectInput scan codes (DIK_*), the game's key codes.
int Keys_FromScancode(SDL_Scancode scancode);
SDL_Scancode Keys_ToScancode(int dik);
// Name of a key for the controls screen (GetKeyNameText).
const char *Keys_GetName(int dik);

#endif
