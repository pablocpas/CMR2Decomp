// Process entry point: sets up the platform and runs the game's main loop
// (CMain::Initialize, which was WinMain's body).

#include "platform/platform.h"
#include "port/sys.h"
#include "diagnostics/timing.h"
#include "main.h"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>

int main(int argc, char **argv)
{
    SDL_SetMainReady();
    if (!Platform_Init(argc, argv))
        return 1;
    Diagnostics::StartProfileFromEnvironment();
    unsigned int code = CMain::Initialize(Sys_GetCommandLine());
    Diagnostics::StopProfile();
    Platform_Shutdown();
    return (int)code;
}
