#include "main.h"
#include <mmsystem.h>
#include "Graphics.h"
#include "Logger.h"
#include "Game.h"
#include "Input.h"
#include "Sound.h"

HINSTANCE CMain::m_hInstance;
int CMain::m_frameTime;
BOOL CMain::m_frameDeltaInitialised;
unsigned int CMain::m_frameDeltaStart;
unsigned int CMain::m_frameDeltaLast;
unsigned int CMain::m_frameDelta;
unsigned int CMain::m_frameDeltaMax;
HWND CMain::m_hWndList[1];
int CMain::m_hWndIx = 0;
int CMain::m_unk0x00663dbc;
int CMain::m_unk0x00663dc0;

// GLOBAL: CMR2 0x005210ac
int g_unk0x005210ac = 1;

char CMain::m_logFileLocation[14] = "c:\\error.txt";
char CMain::m_gameName[20] = "Colin McRae Rally 2";
char CMain::m_logFileHeader1[29] = "FILE_PRINT DEBUG INFORMATION";
char CMain::m_logFileAsterisks[29] = "****************************";
char CMain::m_logFileBlankLine[1] = "";
char CMain::m_logFileFinishedNormally[30] = "* Program finished normally *";
BOOL CMain::m_isShowingCursor = TRUE;

MSG CMain::m_win32Msg;

// GLOBAL: CMR2 0x00520b94
char m_lpszMenuName[5] = "menu";

int WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	return CMain::Initialize(hInstance, 0, lpCmdLine); // TODO: params aren't correct
}

// FUNCTION: CMR2 0x004a9720
unsigned char CMain::Initialize(HINSTANCE hInstance, unsigned char param2, LPSTR param3)
{
	HWND hWnd;
	BOOL isMessageAvailable;

	hWnd = FindWindowA(m_gameName, m_gameName);
	if (hWnd != NULL)
		return 0;

	CLogger::OpenLogFile(m_logFileLocation);
	CLogger::LogToFile(m_logFileHeader1);
	CLogger::LogToFile(m_logFileAsterisks);
	CLogger::LogToFile(m_logFileBlankLine);
	m_hInstance = hInstance;

	CreateGameWindow(hInstance, &m_hWndList[m_hWndIx], m_gameName, MessageHandler);

	MSG msg;
	while (!CGame::m_shouldExit)
	{
		if (!CGame::m_isActive)
			isMessageAvailable = PeekMessageA(&m_win32Msg, NULL, 0, 0, 1);
		else
			isMessageAvailable = GetMessageA(&m_win32Msg, NULL, 0, 0);

		if ((m_win32Msg.message == WM_ACTIVATEAPP) || (isMessageAvailable == 0))
		{
			if (!CGame::m_isActive)
			{
				CGame::FUN_004b7a40();
				CGame::FUN_004d0780();
			}
		}
		else
		{
			if (m_win32Msg.message == WM_QUIT)
				break;

			TranslateMessage(&m_win32Msg);
			DispatchMessageA(&m_win32Msg);
		}

		if (g_pGraphics->isFullscreen == FALSE)
		{
			if (m_isShowingCursor == FALSE)
			{
				ShowCursor(1);
				m_isShowingCursor = TRUE;
			}
		}
		else if (m_isShowingCursor != FALSE)
		{
			ShowCursor(0);
			m_isShowingCursor = FALSE;
		}
	}

	FUN_0049c130();
	CLogger::LogToFile(m_logFileBlankLine);
	CLogger::LogToFile(m_logFileAsterisks);
	CLogger::LogToFile(m_logFileFinishedNormally);
	CLogger::LogToFile(m_logFileAsterisks);
	CLogger::CloseLogFile();

	return m_win32Msg.wParam;
}

// STUB: CMR2 0x004a8270
BOOL FUN_004a8270(void)
{
	// todo
	return 1;
}

// FUNCTION: CMR2 0x004a8140
BOOL CMain::CreateGameWindow(HINSTANCE hInstance, HWND *pHWND, LPCSTR sWindowName, WNDPROC wndProc)
{
	ATOM AVar1;
	int nScreenHeight;
	int nScreenWidth;
	HWND hWnd;
	DWORD dwStyle = 0;
	HWND hWndParent;
	HMENU hMenu;
	LPVOID lpParam;
	WNDCLASSA wndClass;
	int nCmdShow;

	wndClass.lpfnWndProc = wndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIconA(hInstance, (const char *)0x6e);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(4);
	wndClass.lpszMenuName = m_lpszMenuName;
	wndClass.lpszClassName = sWindowName;
	wndClass.style = 3;

	dwStyle = 0x81cf0000;
	if (!g_pGraphics->isFullscreen)
		wndClass.hCursor = LoadCursorA(NULL, (const char *)0x7f00);
	else
		wndClass.hCursor = NULL;

	AVar1 = RegisterClassA(&wndClass);
	if (AVar1 == 0)
		return FALSE;

	lpParam = NULL;
	hMenu = NULL;
	hWndParent = NULL;
	nScreenHeight = GetSystemMetrics(1);
	nScreenWidth = GetSystemMetrics(0);

	hWnd = CreateWindowExA((DWORD)0x40000, sWindowName, sWindowName, dwStyle, 0, 0, nScreenWidth, nScreenHeight,
						   hWndParent, hMenu, hInstance, lpParam);

	m_hWndList[m_hWndIx] = hWnd;
	if (hWnd == NULL)
		return FALSE;

	if (!g_pGraphics->isFullscreen)
		nCmdShow = SW_HIDE;
	else
		nCmdShow = SW_MAXIMIZE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	*pHWND = hWnd;
	CGame::RegisterCallback(FUN_004a8270, NULL);
	return TRUE;
}

// FUNCTION: CMR2 0x004a98b0
LRESULT CMain::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int *pDD7;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		if (wParam == 4 || wParam == 1)
			FUN_004a9a50(1);
		else
			FUN_004a9a50(0);
		break;

	case WM_KILLFOCUS:
		CSound::FUN_004a28c0();
		break;

	case WM_ENABLE:
		CSound::FUN_004a31a0();
		break;

	case WM_QUIT:
		if (g_pGraphics->isFullscreen != 0)
			return 0;

		msg = WM_KEYDOWN;
		wParam = VK_ESCAPE;
		PostQuitMessage(0);
		break;

	case WM_ACTIVATEAPP:
		FUN_004a9a50(wParam == 0);
		if (wParam != 0) {
			CGraphics::RestoreSurfaces();
			pDD7 = (int *)g_pGraphics->pDD7;
			if (pDD7 != NULL)
				((void (__stdcall *)(int *))*(int *)(*pDD7 + 0x64))(pDD7);
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		CInput::FUN_004b7d10(lParam);
		break;

	case WM_CHAR:
		CInput::FUN_004b7ca0(wParam);
		break;

	case WM_SYSCOMMAND:
		if (wParam == 0xf140 || wParam == 0xf170)
			return 1;
		break;

	case WM_SYSKEYUP:
	case 0x218:
		return 0;

	default:
		if (msg != 0 && msg == RegisterWindowMessageA("QueryCancelAutoPlay"))
			return 1;
		break;
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

// Switches the game between active and inactive: input/mouse cooperative
// level, the sound "hooked" flag and the menu bar, plus the inactive time.
// FUNCTION: CMR2 0x004a9a50
void CMain::FUN_004a9a50(int param1)
{
	int *pDD7;
	int frameTime;

	if (param1 != 0) {
		CInput::SetMouseCoopLevel(0);
		CSound::FUN_004b7b10();
		m_unk0x00663dbc = GetFrameTime();
		CGame::m_isActive = 1;

		pDD7 = (int *)g_pGraphics->pDD7;
		if (pDD7 != NULL)
			((void (__stdcall *)(int *))*(int *)(*pDD7 + 0x28))(pDD7);

		DrawMenuBar(m_hWndList[m_hWndIx]);
		RedrawWindow(m_hWndList[m_hWndIx], NULL, NULL, 0x400);
		return;
	}

	CInput::SetMouseCoopLevel(1);
	CSound::FUN_004b7b10();
	CInput::FUN_0049efc0();
	FUN_004b2390();
	frameTime = GetFrameTime();
	CGame::m_isActive = 0;
	m_unk0x00663dc0 = frameTime - m_unk0x00663dbc;
}

// FUNCTION: CMR2 0x004b2390
void CMain::FUN_004b2390(void)
{
	g_unk0x005210ac = 1;
}

// FUNCTION: CMR2 0x0049c130
void CMain::FUN_0049c130(void)
{
	CGame::UnwindCallbacks(0);
}

// FUNCTION: CMR2 0x004a9b60
int CMain::GetFrameTime(void)
{
    return m_frameTime;
}

// FUNCTION: CMR2 0x004a9b70
void CMain::UpdateFrameTime(void)
{
    m_frameTime = timeGetTime();
}

// FUNCTION: CMR2 0x004a9c20
bool CMain::ResetFrameDelta(void)
{
    m_frameDeltaInitialised = FALSE;
    return true;
}

// FUNCTION: CMR2 0x004a9b80
unsigned int CMain::GetFrameDelta(void)
{
    unsigned int now;

    now = GetFrameTime() / 10;
    if (m_frameDeltaInitialised == 0) {
        m_frameDelta = 0;
        m_frameDeltaMax = 0;
        m_frameDeltaInitialised = TRUE;
        m_frameDeltaStart = now;
        m_frameDeltaLast = now;
        CGame::RegisterCallback(ResetFrameDelta, NULL);
    }
    if (now - m_frameDeltaLast != 0) {
        m_frameDelta += now - m_frameDeltaLast;
        if (m_frameDelta < m_frameDeltaMax) {
            m_frameDeltaLast = now;
            m_frameDelta = m_frameDeltaMax;
            return m_frameDeltaMax;
        }
        m_frameDeltaLast = now;
        m_frameDeltaMax = m_frameDelta;
        return m_frameDelta;
    }
    m_frameDeltaLast = now;
    return m_frameDelta;
}
