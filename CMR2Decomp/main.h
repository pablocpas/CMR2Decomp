#ifndef _MAIN_H
#define _MAIN_H

#include <windows.h>

class CMain
{
public:
    static unsigned int Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
    static BOOL CreateGameWindow(HINSTANCE hInstance, HWND *pHWND, LPCSTR sWindowName, WNDPROC param_4);
    static LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void FUN_0049c130(void);
    static void FUN_004a9a50(int param1);
    static void FUN_004b2390(void);
    static int GetFrameTime(void);
    static void UpdateFrameTime(void);
    static unsigned int GetFrameDelta(void);
    static bool ResetFrameDelta(void);

    // GLOBAL: CMR2 0x00663ecc
    static BOOL m_frameDeltaInitialised;
    // GLOBAL: CMR2 0x00663ed0
    static unsigned int m_frameDeltaStart;
    // GLOBAL: CMR2 0x00663ed4
    static unsigned int m_frameDeltaLast;
    // GLOBAL: CMR2 0x00663ed8
    static unsigned int m_frameDelta;
    // GLOBAL: CMR2 0x00663edc
    static unsigned int m_frameDeltaMax;

    // GLOBAL: CMR2 0x00663ee0
    static int m_frameTime;

    // GLOBAL: CMR2 0x00663dbc
    static int m_unk0x00663dbc;
    // GLOBAL: CMR2 0x00663dc0
    static int m_unk0x00663dc0;

    // GLOBAL: CMR2 0x00663db0
    static HINSTANCE m_hInstance;    

    // GLOBAL: CMR2 0x00663c84
    static HWND m_hWndList[1];
    // GLOBAL: CMR2 0x00663dac
    static int m_hWndIx;

    // GLOBAL: CMR2 0x00663c68
    static MSG m_win32Msg;

    // GLOBAL: CMR2 0x0052ea5c
    static char m_logFileBlankLine[1]; // TODO: better name?

private:
    // GLOBAL: CMR2 0x00511430
    static char m_gameName[20];
    // GLOBAL: CMR2 0x00520c74
    static char m_logFileLocation[14];
    // GLOBAL: CMR2 0x00520c54
    static char m_logFileHeader1[29];
    // GLOBAL: CMR2 0x00520c34
    static char m_logFileAsterisks[29];

    // GLOBAL: CMR2 0x00520bf4
    static char m_logFileFinishedNormally[30];
    // GLOBAL: CMR2 0x00520bf0
    static BOOL m_isShowingCursor;
};

#endif
