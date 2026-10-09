#include "port/sys.h"
#include "GameInfo.h"
#include "Menu.h"
#include "Graphics.h"
#include "Input.h"
#include "Frontend.h"
#include "RegKey.h"
#include "InstallInfo.h"
#include "FileBuffer.h"
#include "GenericFileLoader.h"
#include "main.h"
#include "Font.h"
#include "FixedPoint.h"
#include "Game.h"
#include "Sound.h"
#include "RallyData.h"
#include "NetPlayers.h"

#include <stdio.h>
#include <string.h>

// GLOBAL: CMR2 0x00516134
char gameRegionPoland[9] = "\\Poland\\";
// GLOBAL: CMR2 0x00516140
char gameRegionJapan[8] = "\\Japan\\";
// GLOBAL: CMR2 0x00516148
char gameRegionUSA[6] = "\\Usa\\";
// GLOBAL: CMR2 0x00516150
char gameRegionEurope[9] = "\\Europe\\";

GameInfo CGameInfo::m_gameInfo;
BYTE CGameInfo::m_unk0x0052ea52;
unsigned int CGameInfo::m_unk0x0052af80;
unsigned char CGameInfo::m_unk0x0052af40;
unsigned int CGameInfo::m_unk0x0052af84;
unsigned int CGameInfo::m_unk0x0052af88;
unsigned int CGameInfo::m_unk0x0052af8c;
unsigned int CGameInfo::m_unk0x0052af94;
unsigned int CGameInfo::m_unk0x0052af98;
unsigned int CGameInfo::m_unk0x0052af9c;
unsigned int CGameInfo::m_unk0x0052e93c;
unsigned int CGameInfo::m_unk0x0052e940;
unsigned int CGameInfo::m_unk0x0052ea44;
unsigned int CGameInfo::m_unk0x0052ea48;
unsigned int CGameInfo::m_gameRegion;
char *CGameInfo::m_gameRegionStrings[4] = {
    gameRegionEurope, gameRegionUSA, gameRegionJapan, gameRegionPoland};

BYTE CGameInfo::m_unk0x00817574;
int CGameInfo::m_unk0x0081a754;

char CGameInfo::m_stringCMR[4] = "cmr";
char CGameInfo::m_stringGameInfoRCF[32] = "%s\\Configuration\\GameInfo.rcf";
unsigned int CGameInfo::m_unk0x0059f8d0;
unsigned int CGameInfo::m_unk0x00520870 = 1;
void* CGameInfo::m_unk0x0081777c = NULL;
BOOL CGameInfo::m_unk0x00817678 = FALSE;
char CGameInfo::m_unk0x005a00b8[0x104];
char CGameInfo::m_unk0x005a02c0[0x104];
char CGameInfo::m_sessionNames[20][0x104];
BOOL CGameInfo::m_unk0x005a0060;
HRESULT CGameInfo::m_unk0x005a1814;
BYTE CGameInfo::m_unk0x005a01bc;
BOOL CGameInfo::m_unk0x0059fa20[400];

// FUNCTION: CMR2 0x004057f0
unsigned char CGameInfo::GetGameLanguage(void)
{
    return m_gameInfo.field_0x14 & 7;
}

// FUNCTION: CMR2 0x00405800
unsigned int CGameInfo::GetGameRegion(void)
{
    return m_gameRegion;
}

// FUNCTION: CMR2 0x00405810
void CGameInfo::SetGameRegion(unsigned int region)
{
    m_gameRegion = region;
}

// FUNCTION: CMR2 0x00405820
char *CGameInfo::GetGameRegionDirectory(void)
{
    return m_gameRegionStrings[m_gameRegion];
}

// FUNCTION: CMR2 0x00405d80
unsigned char CGameInfo::GetConfiguredGameMode(void)
{
    return m_gameInfo.field_0x14 >> 3 & 0x7f;
}

// FUNCTION: CMR2 0x00405d90
unsigned char CGameInfo::GetConfiguredDifficulty(void)
{
    return m_gameInfo.field_0x14 >> 10 & 7;
}

// FUNCTION: CMR2 0x00405d70
unsigned char CGameInfo::GetConfiguredPlayerCount(void)
{
    return m_gameInfo.field_0x14 >> 13 & 0xf;
}

// FUNCTION: CMR2 0x00405da0
unsigned char CGameInfo::IsConfiguredMultiplayer(void)
{
    return m_gameInfo.field_0x14 >> 17 & 1;
}

// FUNCTION: CMR2 0x00405c10
unsigned int CGameInfo::GetScreenWidth(void)
{
    return m_gameInfo.screenWidth;
}

// FUNCTION: CMR2 0x00405c30
unsigned int CGameInfo::GetScreenHeight(void)
{
    return m_gameInfo.screenHeight;
}

// FUNCTION: CMR2 0x00405c50
unsigned int CGameInfo::GetColourDepth(void)
{
    return m_gameInfo.screenColourDepth;
}

// FUNCTION: CMR2 0x00405c20
void CGameInfo::SetScreenWidth(unsigned int width)
{
    m_gameInfo.screenWidth = width;
}

// FUNCTION: CMR2 0x00405c40
void CGameInfo::SetScreenHeight(unsigned int height)
{
    m_gameInfo.screenHeight = height;
}

// FUNCTION: CMR2 0x00405c60
void CGameInfo::SetColourDepth(unsigned int depth)
{
    m_gameInfo.screenColourDepth = depth;
}

// FUNCTION: CMR2 0x004f4b40
void CGameInfo::InitRegionLanguageCount(void)
{
    switch (GetGameRegion())
    {
    case 0:
        m_unk0x0081a754 = 5;
        return;
    case 1:
        m_unk0x0081a754 = 3;
        return;
    case 2:
    case 3:
        m_unk0x0081a754 = 1;
    }
}

// FUNCTION: CMR2 0x00405de0
void CGameInfo::SetGameModeOptionBit19(BYTE param1)
{
    m_gameInfo.field_0x14 = ((param1 & 1) << 0x13) | (m_gameInfo.field_0x14 & 0xfff7ffffU);
}

// FUNCTION: CMR2 0x00405dc0
unsigned char CGameInfo::IsSplitBarEnabled(void)
{
    return m_gameInfo.field_0x14 >> 18 & 1;
}

// FUNCTION: CMR2 0x00405dd0
unsigned char CGameInfo::GetGameModeOptionBits20To22(void)
{
    return m_gameInfo.field_0x14 >> 20 & 7;
}

// FUNCTION: CMR2 0x00405e00
unsigned char CGameInfo::GetGameModeOptionBit19(void)
{
    return m_gameInfo.field_0x14 >> 19 & 1;
}

// FUNCTION: CMR2 0x00405e10
void CGameInfo::SetMasterSoundVolume(unsigned int param1)
{
    m_gameInfo.field_0x18 = m_gameInfo.field_0x18 ^ (m_gameInfo.field_0x18 ^ param1) & 0x7f;
    CSound::SetMusicStreamVolume(m_gameInfo.field_0x18 & 0x7f);
}

// FUNCTION: CMR2 0x00405e40
unsigned int CGameInfo::GetMasterSoundVolume(void)
{
    return m_gameInfo.field_0x18 & 0x7f;
}

// FUNCTION: CMR2 0x00405e50
void CGameInfo::SetEffectsSoundVolume(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 0x7f) << 7) | (m_gameInfo.field_0x18 & 0xffffc07fU);
}

// FUNCTION: CMR2 0x00405e70
unsigned int CGameInfo::GetEffectsSoundVolume(void)
{
    return m_gameInfo.field_0x18 >> 7 & 0x7f;
}

// FUNCTION: CMR2 0x00405e80
void CGameInfo::SetCoDriverSoundVolume(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 0x7f) << 14) | (m_gameInfo.field_0x18 & 0xffe03fffU);
}

// FUNCTION: CMR2 0x00405ea0
unsigned int CGameInfo::GetCoDriverSoundVolume(void)
{
    return m_gameInfo.field_0x18 >> 14 & 0x7f;
}

// FUNCTION: CMR2 0x00405eb0
unsigned int CGameInfo::IsDashOptionEnabled(void)
{
    return m_gameInfo.field_0x18 >> 31;
}

// FUNCTION: CMR2 0x00405ec0
void CGameInfo::SetDashOptionEnabled(BYTE param1)
{
    m_gameInfo.field_0x18 = (param1 << 31) | (m_gameInfo.field_0x18 & 0x7fffffffU);
}

// FUNCTION: CMR2 0x00405ef0
unsigned int CGameInfo::GetNetworkOptionBits1To2(void)
{
    return m_gameInfo.field_0x1c >> 1 & 3;
}

// FUNCTION: CMR2 0x00405f00
void CGameInfo::SetSoundOptionBits24To25(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 3) << 24) | (m_gameInfo.field_0x18 & 0xfcffffffU);
}

// FUNCTION: CMR2 0x00405f20
void CGameInfo::SetSoundOptionBit27(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 27) | (m_gameInfo.field_0x18 & 0xf7ffffffU);
}

// FUNCTION: CMR2 0x00405f40
void CGameInfo::SetSoundOptionBit26(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 26) | (m_gameInfo.field_0x18 & 0xfbffffffU);
}

// FUNCTION: CMR2 0x00405f60
void CGameInfo::SetSoundOptionBit28(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 28) | (m_gameInfo.field_0x18 & 0xefffffffU);
}

// FUNCTION: CMR2 0x00405f80
void CGameInfo::SetSoundOptionBit29(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 29) | (m_gameInfo.field_0x18 & 0xdfffffffU);
}

// FUNCTION: CMR2 0x00405fa0
void CGameInfo::SetDefaultCameraParameters(DWORD *param1, WORD param2, DWORD param3)
{
    m_gameInfo.field_0x88 = param1[1];
    m_gameInfo.field_0x8c = param1[2];
    m_gameInfo.field_0x94 = param2;
    m_gameInfo.field_0x90 = param3;
}

// FUNCTION: CMR2 0x00405fd0
void CGameInfo::SetGameInfoField98(unsigned int param1)
{
    *(unsigned int *)&m_gameInfo.field_0x98 = param1;
}

// FUNCTION: CMR2 0x00405db0
unsigned int *CGameInfo::GetGameInfoField9CAddress(void)
{
    return &m_gameInfo.field_0x9c;
}

// FUNCTION: CMR2 0x00405fe0
GameInfo0xa4 *CGameInfo::GetGameInfoFieldA4Address(void)
{
    return &m_gameInfo.field_0xa4;
}

// FUNCTION: CMR2 0x00405ff0
GameInfo0xa4 *CGameInfo::GetPlayerRecordTable(int param1)
{
    return param1 != 0 ? &m_gameInfo.field_0x1368 : &m_gameInfo.field_0x262c;
}

// FUNCTION: CMR2 0x00405b30
void CGameInfo::SetFullscreen(BYTE fullscreen)
{
    m_gameInfo.unknownGraphicsOptions = (fullscreen & 1) | (m_gameInfo.unknownGraphicsOptions & 0xfffffffeU);
}

// FUNCTION: CMR2 0x00405b50
unsigned int CGameInfo::GetGraphicsOptionBits1To2(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 1 & 3;
}

// FUNCTION: CMR2 0x00405b60
void CGameInfo::SetGraphicsOptionBits1To2(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 1) | (m_gameInfo.unknownGraphicsOptions & 0xfffffff9U);
}

// FUNCTION: CMR2 0x00405b80
void CGameInfo::SetGraphicsOptionBit3(BYTE param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 1) << 3) | (m_gameInfo.unknownGraphicsOptions & 0xfffffff7U);
}

// FUNCTION: CMR2 0x00405ba0
unsigned int CGameInfo::GetGraphicsOptionBit4(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 4 & 1;
}

// FUNCTION: CMR2 0x00405bb0
void CGameInfo::SetGraphicsOptionBit4(BYTE param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 1) << 4) | (m_gameInfo.unknownGraphicsOptions & 0xffffffefU);
}

// FUNCTION: CMR2 0x00405be0
void CGameInfo::SetGraphicsOptionBits5To8(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 0xf) << 5) | (m_gameInfo.unknownGraphicsOptions & 0xfffffe1fU);
}

// FUNCTION: CMR2 0x00405c70
unsigned int CGameInfo::GetGraphicsOptionBits18To19(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 18 & 3;
}

// FUNCTION: CMR2 0x00405c80
void CGameInfo::SetGraphicsOptionBits18To19(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 18) | (m_gameInfo.unknownGraphicsOptions & 0xfff3ffffU);
}

// FUNCTION: CMR2 0x00405cb0
void CGameInfo::SetGraphicsOptionBits21To24(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 0xf) << 21) | (m_gameInfo.unknownGraphicsOptions & 0xfe1fffffU);
}

// FUNCTION: CMR2 0x00405cd0
unsigned int CGameInfo::GetGraphicsOptionBits25To26(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 25 & 3;
}

// FUNCTION: CMR2 0x00405ce0
void CGameInfo::SetGraphicsOptionBits25To26(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 25) | (m_gameInfo.unknownGraphicsOptions & 0xf9ffffffU);
}

// FUNCTION: CMR2 0x00405d00
unsigned int CGameInfo::GetGraphicsOptionBits27To28(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 27 & 3;
}

// FUNCTION: CMR2 0x00405d20
void CGameInfo::SetGraphicsOptionBits27To28(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 27) | (m_gameInfo.unknownGraphicsOptions & 0xe7ffffffU);
}

// FUNCTION: CMR2 0x00405d60
unsigned int CGameInfo::GetGraphicsOptionBit29(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 29 & 1;
}

// FUNCTION: CMR2 0x00405d10
unsigned int CGameInfo::GetPreviewMode(void)
{
    return m_gameInfo.field_0x34 & 3;
}

// FUNCTION: CMR2 0x00405d40
void CGameInfo::SetPreviewMode(unsigned int param1)
{
    m_gameInfo.field_0x34 = m_gameInfo.field_0x34 ^ (m_gameInfo.field_0x34 ^ param1) & 3;
}

// FUNCTION: CMR2 0x00406310
unsigned char CGameInfo::GetSoundOptionBit30(void)
{
    return m_gameInfo.field_0x18 >> 30 & 1;
}

// FUNCTION: CMR2 0x00406320
BYTE CGameInfo::GetGameInfoSessionFlag(void)
{
    return m_unk0x0052ea52;
}

// FUNCTION: CMR2 0x00406330
void CGameInfo::SetGameInfoSessionFlag(BYTE param1)
{
    m_unk0x0052ea52 = param1;
}

// FUNCTION: CMR2 0x00406340
void CGameInfo::SetSplitBarEnabled(BYTE param1)
{
    m_gameInfo.field_0x14 = ((param1 & 1) << 18) | (m_gameInfo.field_0x14 & 0xfffbffffU);
}

// FUNCTION: CMR2 0x00406360
unsigned int CGameInfo::GetUnlockFlagMask(int param1)
{
    return (BYTE)(1 << param1) & m_gameInfo.field_0x20;
}

// FUNCTION: CMR2 0x00406380
void CGameInfo::SetUnlockFlag(int param1, int param2)
{
    BYTE mask;

    mask = (BYTE)(1 << param1);
    if (param2 != 0) {
        m_gameInfo.field_0x20 |= mask;
        return;
    }
    m_gameInfo.field_0x20 &= (BYTE)~mask | 0xffffff00;
}

// FUNCTION: CMR2 0x004063d0
int CGameInfo::IsConfiguredCheatEnabled(int param1)
{
    return ((BYTE)(m_gameInfo.field_0x20 >> 8) & (BYTE)(1 << param1)) != 0;
}

// FUNCTION: CMR2 0x004063f0
int CGameInfo::IsActiveCheatEnabled(int param1)
{
    return ((BYTE)(m_gameInfo.field_0x20 >> 16) & (BYTE)(1 << param1)) != 0;
}

// FUNCTION: CMR2 0x00406430
unsigned int CGameInfo::GetActiveCheatMask(void)
{
    return ((BYTE *)&m_gameInfo.field_0x20)[2];
}

// FUNCTION: CMR2 0x00406440
unsigned int CGameInfo::GetNetworkOptionBit11(void)
{
    return m_gameInfo.field_0x1c >> 11 & 1;
}

// FUNCTION: CMR2 0x00406450
unsigned int CGameInfo::GetRecordFlagsWord(unsigned int **param1)
{
    if (param1 != NULL)
        *param1 = &m_gameInfo.field_0x38f4;
    return m_gameInfo.field_0x38f4;
}

// FUNCTION: CMR2 0x00406470
void CGameInfo::SaveFrontendOptionSettings(void)
{
    m_unk0x0052af94 = GetConfiguredGameMode();
    m_unk0x0052ea44 = GetConfiguredPlayerCount();
    m_unk0x0052af9c = GetConfiguredDifficulty();
    m_unk0x0052af80 = RallyData_GetSecondarySelectionNibble();
    m_unk0x0052af84 = (BYTE)RallyData_GetSelectionBits10To11();
    m_unk0x0052af8c = (BYTE)RallyData_GetSelectionBits12To13();
    m_unk0x0052e93c = (BYTE)RallyData_GetSelectionBits16To19();
    m_unk0x0052ea48 = (BYTE)RallyDataStageIndex();
    m_unk0x0052e940 = (BYTE)RallyDataCountryIndex();
    m_unk0x0052af98 = m_gameInfo.field_0x14 >> 23 & 0xf;
    m_unk0x0052af88 = m_gameInfo.field_0x14 >> 27 & 0xf;
}

// FUNCTION: CMR2 0x00406520
BYTE CGameInfo::GetStageOptionState(int param1, int param2)
{
    return m_gameInfo.field_0x38f8[param1 * 11 + param2];
}

// FUNCTION: CMR2 0x00406540
void CGameInfo::SetStageOptionState(int param1, int param2, BYTE param3)
{
    m_gameInfo.field_0x38f8[param1 * 11 + param2] = param3;
}

// FUNCTION: CMR2 0x00406690
char *CGameInfo::GetSessionName(void)
{
    return m_gameInfo.field_0x3950;
}

// FUNCTION: CMR2 0x004066a0
void CGameInfo::SetSessionName(char *name)
{
    strcpy(m_gameInfo.field_0x3950, name);
}

// FUNCTION: CMR2 0x004066d0
char *CGameInfo::GetSessionPassword(void)
{
    return m_gameInfo.field_0x3965;
}

// FUNCTION: CMR2 0x004066e0
void CGameInfo::SetSessionPassword(char *name)
{
    strcpy(m_gameInfo.field_0x3965, name);
}

void Input_TranslatePedalsToMenuKeys(void);
void Input_MergeAssignedJoystickButtons(int slot, DeviceInfo *pOut);
void Sound_UpdateMusicStreaming(void);
IDirectSound *Sound_GetSampleTableState(void);
BOOL OptionMovie_PlayFrame(BYTE skipOnSpace);
void OptionMovie_Close(void);


// Bink movie state: the open movie and its dimensions, the Bink buffer the
// frames are played on, the DirectDraw surface they are converted to, and the
// surface type returned for it (-1 while unknown). The four screen coordinates
// are the rectangle the movie is scaled into (left, top, right, bottom).
// GLOBAL: CMR2 0x005297d0
int g_unk0x005297d0 = -1;
// GLOBAL: CMR2 0x00831ac8
unsigned int g_unk0x00831ac8;
// GLOBAL: CMR2 0x00831acc
unsigned int g_unk0x00831acc;
// GLOBAL: CMR2 0x00831ad0
HBINK g_pUnk0x00831ad0;
// GLOBAL: CMR2 0x00831ad4
HBINKBUFFER g_pUnk0x00831ad4;
// GLOBAL: CMR2 0x00831c54
IDirectDrawSurface7 *g_pUnk0x00831c54;
// GLOBAL: CMR2 0x00831c58
int g_unk0x00831c58;
// GLOBAL: CMR2 0x00831c5c
int g_unk0x00831c5c;
// GLOBAL: CMR2 0x00831c60
int g_unk0x00831c60;
// GLOBAL: CMR2 0x00831c64
int g_unk0x00831c64;
// GLOBAL: CMR2 0x00831c68
int g_unk0x00831c68;
// GLOBAL: CMR2 0x00831c6c
IDirectDrawSurface7 *g_pUnk0x00831c6c;

// Starts the movie in the file: remembers the rectangle it is played in (or
// uses the whole screen when no rectangle is given) and the surface it is
// copied to, then opens it. With flag 4 only one frame is played and 2 is
// returned while frames are left; otherwise it plays to the end. Returns 1 when
// it is done, or when the movie could not be opened.
// FUNCTION: CMR2 0x0050fdf0
int OptionMovie_StartPlayback(char *path, Texture *pTexture, short *pRect, unsigned int flags, unsigned int track)
{
    RECT windowRect;

    if (g_unk0x00831c68 == 0) {
        if (pRect != NULL) {
            g_unk0x00831c58 = pRect[0];
            g_unk0x00831c5c = pRect[1];
            g_unk0x00831c60 = pRect[2] + pRect[0];
            g_unk0x00831c64 = pRect[3] + pRect[1];
        } else {
            g_unk0x00831c58 = 0;
            g_unk0x00831c5c = 0;
            g_unk0x00831c60 = g_pGraphics->resX;
            g_unk0x00831c64 = g_pGraphics->resY;
        }
        if (!g_pGraphics->isFullscreen) {
            int borderX = GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXBORDER);
            int borderY = GetSystemMetrics(SM_CYEDGE) + GetSystemMetrics(SM_CYBORDER) +
                          GetSystemMetrics(SM_CYCAPTION);

            GetWindowRect(CMain::m_hWndList[CMain::m_hWndIx], &windowRect);
            g_unk0x00831c58 += windowRect.left + borderX;
            g_unk0x00831c5c += windowRect.top + borderY;
            g_unk0x00831c60 += windowRect.left + borderX;
            g_unk0x00831c64 += windowRect.top + borderY;
        }
        if (pTexture != NULL)
            g_pUnk0x00831c54 = pTexture->pSurface;
        else
            g_pUnk0x00831c54 = g_pGraphics->pPrimarySurface;
        if (OptionMovie_Open(path, track) == 0)
            return 1;
        g_unk0x00831c68 = 1;
    }
    if (flags & 4) {
        if (OptionMovie_PlayFrame((BYTE)flags) != 0)
            return 2;
    } else {
        while (OptionMovie_PlayFrame((BYTE)flags) != 0) {
        }
    }
    OptionMovie_Close();
    return 1;
}

// Opens the movie file with Bink, creates the DirectDraw surface its frames are
// copied to and opens a Bink buffer on the game window. Returns 0 if the CD is
// missing.
// FUNCTION: CMR2 0x0050ff90
int OptionMovie_Open(char *fileName, unsigned int trackIndex)
{
    DDSURFACEDESC2 desc;

    BinkSoundUseDirectSound(Sound_GetSampleTableState());
    BinkSetSoundTrack(trackIndex);
    g_pUnk0x00831ad0 = BinkOpen(fileName, BINKNOTHREADEDIO | BINKSNDTRACK);
    while (g_pUnk0x00831ad0 == NULL) {
        if (!CInstallInfo::ShowNoCDErrorMessage())
            return 0;
        g_pUnk0x00831ad0 = BinkOpen(fileName, BINKNOTHREADEDIO | BINKSNDTRACK);
    }

    g_unk0x00831ac8 = g_pUnk0x00831ad0->Width;
    g_unk0x00831acc = g_pUnk0x00831ad0->Height;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = g_pUnk0x00831ad0->Width;
    desc.dwHeight = g_pUnk0x00831ad0->Height;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    if (CGraphics::GetSelectedRenderDeviceSurfaceCaps() == 1 || CGraphics::GetSelectedRenderDeviceSurfaceCaps() == 2)
        desc.ddsCaps.dwCaps2 = DDSCAPS2_DONOTPERSIST | DDSCAPS2_TEXTUREMANAGE;
    else
        desc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    desc.ddsCaps.dwCaps2 |= DDSCAPS2_HINTDYNAMIC;
    desc.ddpfPixelFormat = CGraphics::m_pTextureManager->textureInfo1->desc.ddpfPixelFormat;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    g_pGraphics->pDD7->CreateSurface(&desc, &g_pUnk0x00831c6c, NULL);

    g_unk0x005297d0 = BinkDDSurfaceType(g_pUnk0x00831c6c);
    if (g_pUnk0x00831c54 == g_pGraphics->pPrimarySurface && g_pGraphics->isFullscreen)
        g_pGraphics->pDD7->FlipToGDISurface();

    g_pUnk0x00831ad4 = BinkBufferOpen(CMain::m_hWndList[CMain::m_hWndIx], g_pUnk0x00831ad0->Width,
                                      g_pUnk0x00831ad0->Height,
                                      BINKBUFFERSTRETCHX | BINKBUFFERSTRETCHY);
    return 1;
}

// Half of the letterbox border added to centre the movie (0.5).
// GLOBAL: CMR2 0x005113b8
double g_unk0x005113b8 = 0.5;

// Plays one frame of the current movie: scales and offsets the Bink buffer to
// the game window, copies the frame into it and blits it to the screen.
// Returns whether the movie has more frames left.
// FUNCTION: CMR2 0x00510120
BOOL OptionMovie_PlayFrame(BYTE skipOnSpace)
{
    DeviceInfo *pDevice;
    RECT clientRect;
    BYTE keyByte;
    int scaleWidth;
    int scaleHeight;
    int waitResult;

    CInput::UpdateAllAvailableDevices();
    pDevice = CInput::GetAvailableDeviceRecord(0);
    Input_MergeAssignedJoystickButtons(0, pDevice);
    if (pDevice->field_0x8 & 0x10)
        return FALSE;
    if (skipOnSpace & 1) {
        keyByte = (BYTE)((USHORT)GetAsyncKeyState(VK_SPACE) >> 8);
        if (keyByte != 0)
            return TRUE;
    }

    BinkDoFrame(g_pUnk0x00831ad0);
    if (!g_pGraphics->isFullscreen) {
        GetClientRect(CMain::m_hWndList[CMain::m_hWndIx], &clientRect);
        BinkBufferSetScale(g_pUnk0x00831ad4, clientRect.right - clientRect.left,
                           clientRect.bottom - clientRect.top);
    } else {
        if (CGraphics::GetTextureFormatCap200(CGraphics::GetSelectedDisplayDriverIndex()) != 0) {
            if (CGameInfo::GetScreenWidth() >= 0x640) {
                scaleWidth = 0x500;
                scaleHeight = 0x3c0;
            } else if (CGameInfo::GetScreenWidth() >= 0x400) {
                scaleWidth = 0x400;
                scaleHeight = 0x300;
            } else {
                scaleWidth = 0x280;
                scaleHeight = 0x1e0;
            }
            BinkBufferSetScale(g_pUnk0x00831ad4, scaleWidth, scaleHeight);
            BinkBufferSetOffset(g_pUnk0x00831ad4,
                                (int)(__int64)((g_pGraphics->resX - scaleWidth) * g_unk0x005113b8),
                                (int)(__int64)((g_pGraphics->resY - scaleHeight) * g_unk0x005113b8));
        } else {
            BinkBufferSetOffset(g_pUnk0x00831ad4, (int)(__int64)((g_pGraphics->resX - 0x280) * g_unk0x005113b8),
                                (int)(__int64)((g_pGraphics->resY - 0x1e0) * g_unk0x005113b8));
        }
        if (CGraphics::GetTextureFormatCap80000(CGraphics::GetSelectedDisplayDriverIndex()) == 0)
            BinkBufferSetOffset(g_pUnk0x00831ad4,
                                (int)(__int64)((int)(g_pGraphics->screenResX - 0x280) * g_unk0x005113b8),
                                (int)(__int64)((int)(g_pGraphics->screenResY - 0x1e0) * g_unk0x005113b8));
    }

    if (BinkBufferLock(g_pUnk0x00831ad4) != 0) {
        BinkCopyToBuffer(g_pUnk0x00831ad0, g_pUnk0x00831ad4->Buffer, g_pUnk0x00831ad4->BufferPitch,
                         g_pUnk0x00831ad4->Height, 0, 0, g_pUnk0x00831ad4->SurfaceType);
        BinkBufferUnlock(g_pUnk0x00831ad4);
    }
    BinkBufferBlit(g_pUnk0x00831ad4, g_pUnk0x00831ad0->FrameRects,
                   BinkGetRects(g_pUnk0x00831ad0, g_pUnk0x00831ad4->SurfaceType));
    BinkNextFrame(g_pUnk0x00831ad0);
    waitResult = BinkWait(g_pUnk0x00831ad0);
    while (waitResult != 0)
        waitResult = BinkWait(g_pUnk0x00831ad0);
    return g_pUnk0x00831ad0->FrameNum < g_pUnk0x00831ad0->Frames;
}

// Releases the DirectDraw surface the movie frames are converted to and closes
// the open movie.
// FUNCTION: CMR2 0x005103d0
void OptionMovie_Close(void)
{
    ULONG refCount;

    g_unk0x00831c68 = 0;
    g_pUnk0x00831c54 = NULL;
    if (g_pUnk0x00831c6c != NULL) {
        refCount = g_pUnk0x00831c6c->Release();
        if (refCount == 0)
            g_pUnk0x00831c6c = NULL;
    }
    if (g_pUnk0x00831ad0 != NULL)
        BinkClose(g_pUnk0x00831ad0);
}

// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00510410
void CGameInfo::InitDefaultGameInfo(void)
{
    memset(&m_gameInfo, 0, sizeof(m_gameInfo));
    m_gameInfo.magicNumber = 0x11;

    strcpy(m_gameInfo.field_0x3950, CMain::m_logFileBlankLine);
    strcpy(m_gameInfo.field_0x3965, CMain::m_logFileBlankLine);

    m_gameInfo.field_0x3980 = 8;
    m_gameInfo.field_0x3990 = 8;
    m_gameInfo.field_0x9c = (m_gameInfo.field_0x9c & 0xffc11157U) | 0x11154;
    m_gameInfo.field_0x18 = (m_gameInfo.field_0x18 & 0xfcffffffU) | 0xbc000000;
    m_gameInfo.field_0x1c = (m_gameInfo.field_0x1c & 0xfff33d19U) | 0x33518;
    m_gameInfo.field_0x14 = (m_gameInfo.field_0x14 & 0x807627ffU) | 0x42400;
    m_gameInfo.field_0x397c = 10;
    m_gameInfo.field_0x3984 = 6;
    m_gameInfo.field_0x3988 = 3;
    m_gameInfo.field_0x398c = 0x1e;

    ResetDefaultCameraParameters();

    *(int *)&m_gameInfo.field_0x98 = 4;
    m_gameInfo.field_0x18 = (m_gameInfo.field_0x18 & 0xfff93264) | 0x40393264;

    InitProfileRecordDefaults(&m_gameInfo.field_0xa4);
    InitProfileRecordDefaults(&m_gameInfo.field_0x1368);
    InitProfileRecordDefaults(&m_gameInfo.field_0x262c);

    m_gameInfo.field_0x14 = m_gameInfo.field_0x14 & 0x7fffffff;
}

// FUNCTION: CMR2 0x00510570
void CGameInfo::ResetDefaultCameraParameters(void)
{
    m_gameInfo.field_0x88 = 0x18000;
    m_gameInfo.field_0x8c = 0x68000;
    m_gameInfo.field_0x94 = 0x2d;
    m_gameInfo.field_0x90 = 0xe0000;
}

// Initializes profile record labels, flags, default times and split tables.
// FUNCTION: CMR2 0x00406010
void CGameInfo::InitProfileRecordDefaults(GameInfo0xa4 *pInfo)
{
    unsigned int *pFlags = &pInfo->firstLoop[0].flags;
    int group = 3;
    int count;
    int track;
    int row;
    int split;
    unsigned int *pFinalTime;
    short *pFinalSplits;
    do {
        count = 5;
        do {
            strcpy((char *)(pFlags - 1), m_stringCMR);
            *pFlags = (*pFlags & 0xffffc000) | 0x3c000;
            memset(pFlags + 1, 0, 3);
            pFlags += 3;
        } while (--count);
    } while (--group);
    pFlags = &pInfo->secondLoop[0].flags;
    track = 0xcb;
    pFinalTime = &pInfo->rallyStageRecordTimes[10].value;
    pFinalSplits = pInfo->rallyStageRecordSplits[10];
    do {
        group = 3;
        do {
            count = 5;
            do {
                strcpy((char *)(pFlags - 1), m_stringCMR);
                *pFlags &= 0xffffff80;
                pFlags[1] = 360000;
                *pFlags = (*pFlags & 0xffff87ff) | 0x780;
                pFlags += 3;
            } while (--count);
        } while (--group);
        row = 0;
        do {
            GameInfo0xa4SubStruct8 *pTime = (GameInfo0xa4SubStruct8 *)((BYTE *)pInfo + (row + track) * 8 - 4);
            strcpy(pTime->ident, m_stringCMR);
            unsigned int value = pTime->value & 0xffffff80;
            pTime->value = value;
            unsigned int *pDest = &pTime->value;
            if (row == 10) {
                value = (value & 0xffaee07f) | 0x2ee000;
                pDest = pFinalTime;
            } else {
                value = (value & 0xffddc07f) | 0x5dc000;
            }
            row++;
            *pDest = value;
        } while (row < 11);
        row = 0;
        do {
            short *pSplit;
            if (row == 10) {
                pSplit = pFinalSplits;
                split = 0;
                do {
                    *pSplit++ = (short)(split * 12000);
                    split++;
                } while (split < 10);
            } else {
                split = 0;
                pSplit = (short *)((BYTE *)pInfo + (row + track) * 20 - 0x6c8);
                do {
                    *pSplit++ = (short)(split * 6000);
                    split++;
                } while (split < 10);
            }
            row++;
        } while (row < 11);
        pFinalTime += 22;
        pFinalSplits += 110;
        track += 11;
    } while (track < 0x123);
    pFlags = &pInfo->thirdLoop[0].flags;
    track = 3;
    do {
        group = 3;
        do {
            count = 5;
            do {
                strcpy((char *)(pFlags - 1), m_stringCMR);
                *pFlags = (*pFlags & 0xffff0280) | 0x280;
                pFlags += 3;
            } while (--count);
        } while (--group);
    } while (--track);
    pFlags = &pInfo->arcadeRecordTimes[0].value;
    short *pSplits = pInfo->arcadeRecordSplits[0];
    track = 3;
    do {
        group = 3;
        do {
            strcpy((char *)(pFlags - 1), m_stringCMR);
            *pFlags = (*pFlags & 0xffaee000) | 0x2ee000;
            split = 0;
            do {
                *pSplits++ = (short)(split * 6000);
                split++;
            } while (split < 6);
            pFlags += 2;
        } while (--group);
    } while (--track);
}

// FUNCTION: CMR2 0x00406560
void CGameInfo::ResetStageOptionStates(void)
{
    memset(m_gameInfo.field_0x38f8, 0x03, sizeof(m_gameInfo.field_0x38f8));
}

// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00406580
void CGameInfo::ApplyStageOptionUnlockFlags(void) {
    char bVar1;
    int uVar5 = 0;
    int uVar2 = 0;
    int iVar3 = 0;
    char *piVar4 = m_gameInfo.field_0x38f8;
    char *end = m_gameInfo.field_0x38f8 + sizeof(m_gameInfo.field_0x38f8);
    bool bVar6 = false;

    do {
        if (((int)piVar4 < (int)(m_gameInfo.field_0x38f8 + 44)) && (*piVar4 & 2) != 0) {
            *piVar4 = *piVar4 & 0xfd;
        }

        if (uVar5 < (int)(m_gameInfo.field_0x9c >> 8 & 0xf)) {
            iVar3 = 0;
            do {
                if ((piVar4[iVar3] & 2) != 0) {
                    piVar4[iVar3] = piVar4[iVar3] & 0xfd;
                }
                iVar3 ++;
            } while (iVar3 < 4);

            if (uVar5 < (int)(m_gameInfo.field_0x9c >> 0xc & 0xf)) {
                iVar3 = 4;
                do {
                    if ((piVar4[iVar3] & 2) != 0) {
                        piVar4[iVar3] = piVar4[iVar3] & 0xfd;
                    }
                    iVar3 ++;
                } while (iVar3 < 8);
            
                if ((uVar5 < (int)(m_gameInfo.field_0x9c >> 0x10 & 0xf)) && (m_gameInfo.field_0x9c & 1) != 0) {
                    iVar3 = 8;
                    do {
                        if ((piVar4[iVar3] & 2) != 0) {
                            piVar4[iVar3] = piVar4[iVar3] & 0xfd;
                        }
                        iVar3 ++;
                    } while (iVar3 < 10);
                }
            } 
        }

        uVar2 = uVar5;
        uVar2 = uVar2 & 0x80000001;

        if (uVar2 < 0) {
            uVar2 = (uVar2 - 1) | 0xfffffffe;
            uVar2++;
        }

        if (uVar2) {
            uVar2 = 1 << (((uVar5 + 1) / 2 - 1) & 0x1f);

            if (((((uVar2 & m_gameInfo.field_0xa0 & 0x1f) != 0) ||
                (uVar2 & m_gameInfo.field_0xa0 >> 5 & 0x1f) != 0) ||
                (0 != (uVar2 & m_gameInfo.field_0xa0 >> 10 & 0x1f))) && ((piVar4[10] & 2) != 0)
                ) {
                piVar4[10] = piVar4[10] & 0xfd;
            }            
        } else {
            piVar4[10] = 4;
        }
        
        piVar4 = piVar4 + 0xb;
        uVar5++;
    } while (piVar4 < end);
}

// TODO: is this actually gameinfo related?
// FUNCTION: CMR2 0x0049eaf0
DWORD CGameInfo::SetupInputs(int unused) {
    if (CInput::DInputCreate()) {
        CInput::m_unk0x0059f8cc.field_0x0 = 0;
        CInput::m_unk0x0059f8cc.field_0x1 = 2;
        CInput::m_unk0x0059f8cc.field_0x2 = 0;
        
        for (int i = 0; i < 8; i++) {
            CInput::m_availableDevices[i].field_0x0 = -1;
            CInput::m_availableDevices[i].field_0x18 = -1;
        }
        
        CInput::SetupKeyboard();
        CInput::SetupMouse();
        CInput::GetAttachedJoysticks();
    }

    return CInput::m_unk0x0059f8cc.field_0x0 & 0xFF;
}

// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ea5e0
bool CGameInfo::LoadGameInfo(void) {
    char *hdPath;
    GameInfo *fileBuffer;
    size_t fileSize;
    int iVar4;
    unsigned int graphicsOptions, temp_field0x3bc;

    hdPath = CInstallInfo::GetGameHDPath();
    sprintf(CFrontend::m_stringDest, m_stringGameInfoRCF, hdPath);

    fileBuffer = (GameInfo*)CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, TRUE);
    if (fileBuffer == NULL)
        return false;

    // make sure file size is valid and that it has the magic number
    fileSize = CGenericFileLoader::GetGenericFileSize();
    if (fileSize != 0x399c || fileBuffer->magicNumber != 0x11) {
        CFileBuffer::FreeGenericFileBuffer(fileBuffer);
        return false;
    }

    memcpy(&m_gameInfo, fileBuffer, sizeof(GameInfo));

    g_pGraphics->resX = m_gameInfo.screenWidth;
    g_pGraphics->resY = m_gameInfo.screenHeight;
    g_pGraphics->depth = m_gameInfo.screenColourDepth;

    g_pGraphics->isFullscreen = IsFullscreen() & 0xff;

    graphicsOptions = m_gameInfo.unknownGraphicsOptions & 6;
    if (graphicsOptions == 2) {
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc &= 0xffffffef;
        g_pGraphics->field913_0x3bc &= 0xffffff7f;
    } else if (graphicsOptions == 4) {
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc |= 0x10;
        g_pGraphics->field913_0x3bc &= 0xffffff7f;
    } else if (graphicsOptions == 6) {
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc &= 0xffffffef;
        g_pGraphics->field913_0x3bc |= 0x80;
    } else {
        g_pGraphics->field913_0x3bc &= 0xfffffff7;
        g_pGraphics->field913_0x3bc &= 0xffffffef;
        g_pGraphics->field913_0x3bc &= 0xffffff7f;
    }

    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions & 0x8) << 2 | g_pGraphics->field913_0x3bc & 0xffffffdf;
    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions & 0x10) << 2 | g_pGraphics->field913_0x3bc & 0xffffffbf;

    graphicsOptions = m_gameInfo.unknownGraphicsOptions & 0xc0000;
    if (graphicsOptions == 0x40000) {
        g_pGraphics->field913_0x3bc |= 1;
        g_pGraphics->field913_0x3bc &= 0xfffffffd;
    } else if (graphicsOptions == 0x80000) {
        g_pGraphics->field913_0x3bc &= 0xfffffffe;
        g_pGraphics->field913_0x3bc |= 2;
    } else {
        g_pGraphics->field913_0x3bc &= 0xfffffffe;
        g_pGraphics->field913_0x3bc &= 0xfffffffd;
    }

    g_pGraphics->field917_0x3c0 = GetGraphicsOptionBits21To24();
    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions >> 0x1b & 4) | g_pGraphics->field913_0x3bc & 0xfffffffb;

    if (IsRecordFlagSet(0x11) != 0)
        ToggleRecordFlag(0x11);

    SetFrontendSessionFlag(0);
    CFileBuffer::FreeGenericFileBuffer(fileBuffer);

    return true;
}

// FUNCTION: CMR2 0x00405b20
unsigned int CGameInfo::IsFullscreen(void) {
    return m_gameInfo.unknownGraphicsOptions & 1;
}

// FUNCTION: CMR2 0x00405ca0
int CGameInfo::GetGraphicsOptionBits21To24(void)
{
  return (int)(m_gameInfo.unknownGraphicsOptions >> 0x15 & 0xf);
}

// FUNCTION: CMR2 0x00406410
bool CGameInfo::IsRecordFlagSet(int param1) {
    bool response = m_gameInfo.field_0x38f0 & (1 << param1);
    return response;
}

// FUNCTION: CMR2 0x004eac50
bool CGameInfo::ToggleRecordFlag(int param1) {
    bool response = IsRecordFlagSet(param1);
    m_gameInfo.field_0x38f0 = m_gameInfo.field_0x38f0 ^ 1 << param1;
    return !response;
}

// GLOBAL: CMR2 0x00817780
int g_unk0x00817780;
// GLOBAL: CMR2 0x00817784
char g_unk0x00817784[5][256];
// GLOBAL: CMR2 0x00817c84
char *g_unk0x00817c84[5];

// FUNCTION: CMR2 0x004d05f0
void NetworkChat_ClearLog(void)
{
    char **pp = g_unk0x00817c84;
    char *p = g_unk0x00817784[0];

    do {
        *p = 0;
        *pp = p;
        p += 256;
        pp++;
    // 0x817c84 in the original (g_unk0x00817c84, the next global).
    } while ((int)p < (int)g_unk0x00817784[5]);
    g_unk0x00817780 = -1;
}

int Network_FindSessionPlayerIndex(DPID *pId, char *pIndex);
Unk0x005a1820 *Network_GetActiveSessionPlayerRecord(BYTE index);
char Network_SendPlayerMessage(int to, int guaranteed, int data, int size);

// GLOBAL: CMR2 0x00523bbc
char g_chatLineFormat[] = "%s > %s";

// Adds a chat line "name > text" to the ring of the last five lines and
// rebuilds g_unk0x00817c84 newest first.
// FUNCTION: CMR2 0x004d0620
void NetworkChat_AppendLine(DPID *pFrom, char *text, char local)
{
    int line;
    char **pp;
    int i;

    if (local) {
        if (++g_unk0x00817780 >= 5)
            g_unk0x00817780 = 0;
        sprintf(g_unk0x00817784[g_unk0x00817780], g_chatLineFormat, (char *)RallyData_GetRecord(0), text);
    } else {
        if (!Network_FindSessionPlayerIndex(pFrom, &local))
            return;
        if (++g_unk0x00817780 >= 5)
            g_unk0x00817780 = 0;
        sprintf(g_unk0x00817784[g_unk0x00817780], g_chatLineFormat, Network_GetActiveSessionPlayerRecord(local)->field_0x0, text);
    }
    line = g_unk0x00817780 + 5;
    pp = g_unk0x00817c84;
    i = 5;
    do {
        *pp = g_unk0x00817784[line-- % 5];
        pp++;
    } while (--i);
}

// Sends a chat line to every player and adds it to the local log.
// FUNCTION: CMR2 0x004d0700
void NetworkChat_SendLine(char *text)
{
    char message[0x101];

    message[0] = 0;
    strcpy(message + 1, text);
    Network_SendPlayerMessage(0, 0, (int)message, 0x101);
    NetworkChat_AppendLine(NULL, text, 1);
}

// FUNCTION: CMR2 0x00406710
int GameInfo_GetSessionField397C(void)
{
    return CGameInfo::m_gameInfo.field_0x397c;
}

// FUNCTION: CMR2 0x00406730
int GameInfo_GetSessionField3984(void)
{
    return CGameInfo::m_gameInfo.field_0x3984;
}

// FUNCTION: CMR2 0x00406720
void GameInfo_SetSessionField397C(int param1)
{
    CGameInfo::m_gameInfo.field_0x397c = param1;
}

// FUNCTION: CMR2 0x00406740
void GameInfo_SetSessionField3984(int param1)
{
    CGameInfo::m_gameInfo.field_0x3984 = param1;
}

// FUNCTION: CMR2 0x00406750
int GameInfo_GetSessionField3988(void)
{
    return CGameInfo::m_gameInfo.field_0x3988;
}

// FUNCTION: CMR2 0x00406760
void GameInfo_SetSessionField3988(int param1)
{
    CGameInfo::m_gameInfo.field_0x3988 = param1;
}

// FUNCTION: CMR2 0x00406770
int GameInfo_GetSessionField398C(void)
{
    return CGameInfo::m_gameInfo.field_0x398c;
}

// FUNCTION: CMR2 0x00406790
int GameInfo_GetSessionField3990(void)
{
    return CGameInfo::m_gameInfo.field_0x3990;
}

// FUNCTION: CMR2 0x004067a0
void GameInfo_SetSessionField3990(int param1)
{
    CGameInfo::m_gameInfo.field_0x3990 = param1;
}

// FUNCTION: CMR2 0x00406780
void GameInfo_SetSessionField398C(int param1)
{
    CGameInfo::m_gameInfo.field_0x398c = param1;
}

// FUNCTION: CMR2 0x004d0580
unsigned char GameInfo_GetFrontendSessionFlag(void) {
    return CGameInfo::m_unk0x00817574;
}

// FUNCTION: CMR2 0x004d0590
void CGameInfo::SetFrontendSessionFlag(BYTE param1) {
    m_unk0x00817574 = param1;
}

// FUNCTION: CMR2 0x0049ea90
void CGameInfo::SetInputAndGamePaused(unsigned int param1) {
    SetInputPausedFlag(param1);
    m_unk0x0059f8d0 = param1;
}

// FUNCTION: CMR2 0x0049e930
void CGameInfo::SetInputPausedFlag(unsigned int param1) {
    m_unk0x00520870 = param1;
}

// FUNCTION: CMR2 0x004d05d0
void CGameInfo::InitFrontendSessionBuffer(void) {
    m_unk0x0081777c = 0;
    m_unk0x00817678 = FALSE;
    CGame::RegisterCallback(ReleaseFrontendSessionBuffer, NULL);
}

// FUNCTION: CMR2 0x004d05a0
bool CGameInfo::ReleaseFrontendSessionBuffer(void) {
    if (m_unk0x0081777c != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_unk0x0081777c);
        m_unk0x0081777c = NULL;
    }

    m_unk0x00817678 = FALSE;
    return true;
}

// FUNCTION: CMR2 0x004a0c60
void CGameInfo::ResetNetworkSessionState(void) {
    g_sessionNamePtr = (LPVOID *)m_unk0x005a00b8;
    g_sessionPasswordPtr = (LPVOID *)m_unk0x005a02c0;
    m_unk0x005a0060 = FALSE;
    memset(m_unk0x0059fa20, 0, sizeof(m_unk0x0059fa20));
    m_unk0x005a1814 = FALSE;
    m_unk0x005a01bc = false;
}

// FUNCTION: CMR2 0x00405bd0
unsigned int CGameInfo::GetGraphicsOptionBits5To8(void) {
  return m_gameInfo.unknownGraphicsOptions >> 5 & 0xf;
}

// FUNCTION: CMR2 0x00405c00
unsigned int CGameInfo::GetGraphicsOptionBits9To12(void) {
  return m_gameInfo.unknownGraphicsOptions >> 9 & 0xf;
}

// GLOBAL: CMR2 0x0082af88
int g_unk0x0082af88;
// GLOBAL: CMR2 0x0082af8c
BYTE g_unk0x0082af8c;
// GLOBAL: CMR2 0x0082b0a8
int g_unk0x0082b0a8;
// GLOBAL: CMR2 0x0082b0ac
BYTE g_unk0x0082b0ac;
// GLOBAL: CMR2 0x0082ac58
int g_unk0x0082ac58;
// GLOBAL: CMR2 0x0082ac5c
int g_unk0x0082ac5c;
// GLOBAL: CMR2 0x0082b0a0
int g_unk0x0082b0a0;
// GLOBAL: CMR2 0x0082b2c0
Unk0x0082b2c0 g_unk0x0082b2c0[8];
// Byte flag the option menu state (0x500c80) derives from the stage index.
// GLOBAL: CMR2 0x00527000
BYTE g_unk0x00527000 = 1;
// Option menu music track ("%s\\select3.adp").
// GLOBAL: CMR2 0x0052703c
char g_str0x0052703c[16] = "%s\\select3.adp";
// GLOBAL: CMR2 0x00526f54
int g_unk0x00526f54[7] = { 0, 1, 3, 4, 5, 5, 7 };
// GLOBAL: CMR2 0x00526f70
int g_unk0x00526f70[7] = { 0, 1, 2, 3, 4, 5, 6 };
// GLOBAL: CMR2 0x00526f8c
int g_unk0x00526f8c[11] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
// GLOBAL: CMR2 0x00527098
int g_unk0x00527098[7] = { 12000, 30000, 48000, 42000, 24000, 24000, 30000 };
// GLOBAL: CMR2 0x005270b4
int g_unk0x005270b4[12] = { 0, 96000, 48000, 48000, 72000, 60000, 60000, 84000, 36000, 24000, 48000, 48000 };
// GLOBAL: CMR2 0x0082a90c
int g_unk0x0082a90c[6];
// GLOBAL: CMR2 0x0082a928
int g_unk0x0082a928;
// GLOBAL: CMR2 0x0082a92c
int g_unk0x0082a92c;
// GLOBAL: CMR2 0x0082a930
int g_unk0x0082a930;
// GLOBAL: CMR2 0x0082af78
int g_unk0x0082af78[4];
// GLOBAL: CMR2 0x0082af90
int g_unk0x0082af90[4];
// GLOBAL: CMR2 0x0082b1b4
int g_unk0x0082b1b4;
// GLOBAL: CMR2 0x0082b1bc
int g_unk0x0082b1bc;
// GLOBAL: CMR2 0x0082b488
BYTE g_unk0x0082b488[0x1e0];
// GLOBAL: CMR2 0x0082ba28
BYTE g_unk0x0082ba28[0x1e0];
// GLOBAL: CMR2 0x0082bee8
BYTE g_unk0x0082bee8[8][7];
// GLOBAL: CMR2 0x0082bf20
BYTE g_unk0x0082bf20[16][7];
// Same table as g_unk0x0082bee8, viewed from its 4th row.
#define g_unk0x0082bf04 ((char *)&g_unk0x0082bee8[4][0])
// The four cached option records follow the 48 dirty flags. The copy/update
// routines address both regions; a pointer past the flags alone has no storage.
struct PlayerOptionCache {
    BYTE dirty[4][12];
    BYTE records[4][0x148];
};
typedef char PlayerOptionCacheSize[sizeof(PlayerOptionCache) == 0x550 ? 1 : -1];
// GLOBAL: CMR2 0x0082c040
PlayerOptionCache g_playerOptionCache;
#define g_unk0x0082c040 (g_playerOptionCache.dirty)
#define g_unk0x0082c070 ((BYTE *)g_playerOptionCache.records)
struct Unk0x0082d220Vec {
    int v[4];
};

// 3-component integer vector of the mesh record (12 bytes).
struct Unk0x0082d220Vec3 {
    int v[3];
};

// Source copy of one vertex of a stage mesh (0x20 bytes): position and normal in
// 16.16, then the packed normal bytes. Filled by OptionPreview_AllocateConvertedWheelVertices.
struct Unk0x0082d220VertexFixed {
    FixVector position; // 0x0
    FixVector normal;   // 0xc
    BYTE field_0x18[8]; // 0x18
};

// Mesh vertex the engine renders (0x30 bytes), same layout as MeshVertexF.
struct Unk0x0082d220VertexF {
    float x, y, z;      // 0x0
    float nx, ny, nz;   // 0xc
    BYTE field_0x18[0x18]; // 0x18
};

// Stage mesh record (0x2ac bytes): 15 mesh slots (mesh id - 5) with their scene
// node, source vertex data, bounding box and per-option state.
struct Unk0x0082d220 {
    Mesh *pMeshes[15];                            // 0x0
    SceneNode *pNodes[15];                        // 0x3c
    Unk0x0082d220VertexFixed *pVertexData[15];    // 0x78
    Unk0x0082d220Vec3 centre[15];                 // 0xb4  bounding box centre
    Unk0x0082d220Vec3 halfSize[15];               // 0x168 half size of the box
    int field_0x21c;                              // 0x21c
    int field_0x220;
    int field_0x224;
    int field_0x228;
    Unk0x0082d220Vec field_0x22c;
    Unk0x0082d220Vec field_0x23c;
    WORD vertexCount[15];                         // 0x24c
    BYTE meshCount;                               // 0x26a used slots
    int field_0x26c[15];                          // 0x26c
    int field_0x2a8;                              // 0x2a8 meshes rebuilt
};

// GLOBAL: CMR2 0x0082d220
Unk0x0082d220 g_unk0x0082d220[16];
// GLOBAL: CMR2 0x00831778
Menu *g_pMenu0x00831778;
// GLOBAL: CMR2 0x0083177c
Menu *g_pMenu0x0083177c;
// GLOBAL: CMR2 0x00831a90
int g_unk0x00831a90[4];
// GLOBAL: CMR2 0x00831aa0
int g_unk0x00831aa0[4];
// GLOBAL: CMR2 0x00831ab0
int g_unk0x00831ab0[4];

// FUNCTION: CMR2 0x004ff4b0
int OptionMenu_GetColumnLabelId(int index)
{
    return g_unk0x00526f54[index];
}

// FUNCTION: CMR2 0x004ff4c0
int OptionMenu_GetColumnUnitId(int index)
{
    return g_unk0x00526f70[index];
}

// FUNCTION: CMR2 0x004ff4d0
int OptionMenu_GetColumnWeight(int index)
{
    return g_unk0x00526f8c[index];
}

// GLOBAL: CMR2 0x00526f44
int g_unk0x00526f44 = -1;
// GLOBAL: CMR2 0x00526f48
int g_unk0x00526f48 = -1;
// GLOBAL: CMR2 0x00526f4c
int g_unk0x00526f4c = -1;
// GLOBAL: CMR2 0x00526f50
int g_unk0x00526f50 = -1;

void OptionMenu_ResetRecordTransitions(int count);

// Clears the option menu counters and rebuilds the eight option records.
// FUNCTION: CMR2 0x004ff4e0
void OptionMenu_ResetRecords(void)
{
    g_unk0x00526f48 = 0;
    g_unk0x00526f4c = 0;
    g_unk0x00526f50 = 0;
    g_unk0x0082a92c = 0;
    g_unk0x0082ac58 = 0;
    g_unk0x0082ac5c = 0;
    g_unk0x0082a90c[0] = 0;
    g_unk0x00526f44 = 2;
    g_unk0x0082a90c[1] = 3;
    g_unk0x0082a90c[2] = 3;
    g_unk0x0082a90c[3] = 3;
    g_unk0x0082a90c[4] = 3;
    g_unk0x0082a90c[5] = 3;
    OptionMenu_ResetRecordTransitions(8);
}

// FUNCTION: CMR2 0x004ff540
int OptionMenu_GetScrollDistance(void)
{
    return g_unk0x0082a928 - g_unk0x0082a930;
}

// FUNCTION: CMR2 0x004ff5a0
int OptionMenu_GetSlotType(int index)
{
    return g_unk0x0082a90c[index];
}

// FUNCTION: CMR2 0x005004a0
int OptionMenu_GetSharedValue(void)
{
    return g_unk0x0082a92c;
}

// FUNCTION: CMR2 0x005004b0
void OptionMenu_SetSharedValue(int value)
{
    g_unk0x0082a92c = value;
}

// FUNCTION: CMR2 0x00500520
void OptionMenu_ClearTimeout(void)
{
    g_unk0x0082ac58 = 0;
    g_unk0x0082ac5c = 0;
}

// FUNCTION: CMR2 0x00500530
void OptionMenu_UpdateTimeout(void)
{
    if (g_unk0x0082ac58 != 0 && (unsigned int)(CMain::GetFrameDelta() - g_unk0x0082ac5c) >= 30)
        OptionMenu_ClearTimeout();
}

// The option menu's interpolation base (0x82acf0) and the eight bytes at
// 0x82ace8 that the record loop walks over before it.
struct Unk0x0082ace8 {
    short pad[4];
    int pairs[16][2];
};
// GLOBAL: CMR2 0x0082ace8
Unk0x0082ace8 g_unk0x0082ace8;
#define g_unk0x0082acf0 (g_unk0x0082ace8.pairs)
// 16.16 coordinate pairs of the option menu's layout records: the current
// position, the target it animates to and the interpolated delta.
// GLOBAL: CMR2 0x0082ac68
int g_unk0x0082ac68[16][2];
// GLOBAL: CMR2 0x0082ad70
int g_unk0x0082ad70[16][2];
// GLOBAL: CMR2 0x0082adf0
int g_unk0x0082adf0[16][2];

// A 32-byte row of the option layout table: four slots of two coordinates.
struct Unk0x0082ac68Row {
    int v[4][2];
};

int OptionMenu_GetRecordTransitionMode(int index);

void Network_SetSessionStateFlag(BOOL param1);
void Network_RemoveSessionPlayerByID(DWORD *pId);
void NetPlayers_RemovePlayerByID(int *pId);

// Recomputes the four layout pairs of option record param1; param2 selects a
// straight copy of the target pairs instead of the param3 percent blend.
// FUNCTION: CMR2 0x00500920
void OptionMenu_InterpolateLayoutPairs(int param1, int param2, int param3)
{
    int i;

    if (param2 == 0) {
        for (i = 0; i < 4; i++) {
            g_unk0x0082ac68[param1 * 4 + i][0] =
                (g_unk0x0082adf0[param1 * 4 + i][0] * param3) / 100 + g_unk0x0082acf0[param1 * 4 + i][0];
            g_unk0x0082ac68[param1 * 4 + i][1] =
                (g_unk0x0082adf0[param1 * 4 + i][1] * param3) / 100 + g_unk0x0082acf0[param1 * 4 + i][1];
        }
        return;
    }
    for (i = 0; i < 8; i++)
        ((int *)g_unk0x0082ac68[param1 * 4])[i] = ((int *)g_unk0x0082ad70[param1 * 4])[i];
}

// Stores a new target pair per layout slot of option record param1 and rebuilds
// the deltas; param3 also warps the current pairs to the target.
// FUNCTION: CMR2 0x005009c0
void OptionMenu_SetTargetLayoutPairs(int param1, int *param2, int param3)
{
    int i;

    for (i = 0; i < 4; i++) {
        g_unk0x0082acf0[param1 * 4 + i][0] = g_unk0x0082ac68[param1 * 4 + i][0];
        g_unk0x0082acf0[param1 * 4 + i][1] = g_unk0x0082ac68[param1 * 4 + i][1];
        g_unk0x0082ad70[param1 * 4 + i][0] = param2[0];
        g_unk0x0082ad70[param1 * 4 + i][1] = param2[1];
        if (param3 != 0) {
            g_unk0x0082ac68[param1 * 4 + i][0] = param2[0];
            g_unk0x0082ac68[param1 * 4 + i][1] = param2[1];
            g_unk0x0082acf0[param1 * 4 + i][0] = g_unk0x0082ad70[param1 * 4 + i][0];
            g_unk0x0082acf0[param1 * 4 + i][1] = g_unk0x0082ad70[param1 * 4 + i][1];
        }
        g_unk0x0082adf0[param1 * 4 + i][0] =
            g_unk0x0082ad70[param1 * 4 + i][0] - g_unk0x0082acf0[param1 * 4 + i][0];
        g_unk0x0082adf0[param1 * 4 + i][1] =
            g_unk0x0082ad70[param1 * 4 + i][1] - g_unk0x0082acf0[param1 * 4 + i][1];
        param2 += 2;
    }
}

// Option menu item notification: refreshes the player list of a network device
// entry when it changes, or opens the advanced options on the select action.
// FUNCTION: CMR2 0x00500a70
void OptionMenu_HandleItemNotification(int unused, int *param2)
{
    if (*param2 != 5) {
        if (*param2 == 0x101)
            Network_SetSessionStateFlag(1);
        return;
    }
    Network_RemoveSessionPlayerByID((DWORD *)(param2 + 2));
    NetPlayers_RemovePlayerByID(param2 + 2);
}

void NetPlayers_ReceiveStatistics(DPID *pId, NetStats *pStats);
void NetPlayers_MarkPlayerReadyByID(DPID *pId);
void NetPlayers_RecordPlayerFinishTime(DPID *pId, unsigned int time, int value);
void NetPlayers_MarkPlayerFinishedByID(DPID *pId);
void NetPlayers_RecordRemoteSplitTime(DPID *pId, int split, unsigned int time);
void NetPlayers_SetRemoteCarClass(DPID *pId, unsigned int carClass);
void NetPlayers_BuildFinalClassification(void);
void NetPlayers_ReceivePublishedLeaderboard(char valid, BYTE *p);
void OptionMenu_Start(char param1);
Unk0049c2c0 *Game_GetSecondaryCallbackMachine(void);

// Handles an option menu notification of a network player: the first byte of the
// record selects the operation, the following ones carry its arguments.
// FUNCTION: CMR2 0x00500aa0
void OptionMenu_HandleNetworkNotification(DPID *pId, BYTE *pData)
{
    // the case order mirrors the original's jump table layout
    switch (pData[0]) {
    case 11:
        NetPlayers_ReceiveStatistics(pId, (NetStats *)(pData + 2));
        break;
    case 7:
        NetPlayers_MarkPlayerReadyByID(pId);
        break;
    case 8:
        NetPlayers_RecordRemoteSplitTime(pId, pData[1], *(unsigned int *)(pData + 4));
        break;
    case 10:
        NetPlayers_MarkPlayerFinishedByID(pId);
        NetPlayers_RecordPlayerFinishTime(pId, *(unsigned int *)(pData + 4), *(int *)(pData + 8));
        break;
    case 12:
        OptionMenu_Start(1);
        NetPlayers_BuildFinalClassification();
        break;
    case 6:
        NetPlayers_SetRemoteCarClass(pId, pData[1]);
        break;
    case 16:
        g_unk0x0082b0ac = 1;
        CGame::PromoteCallbackEntryByRule(Game_GetSecondaryCallbackMachine(), 0, 0, 2);
        NetPlayers_BuildFinalClassification();
        break;
    case 17:
        NetPlayers_ReceivePublishedLeaderboard(pData[1], pData + 4);
        NetPlayers_BuildFinalClassification();
        break;
    }
}

int Network_PollReceivedMessageBuffer(int param1, void **param2);

// Drains the pending network messages: those coming from the system id (0) go to
// the player-list handler, the rest to the option notification handler.
// FUNCTION: CMR2 0x00500ba0
void GameInfo_ProcessNetworkMessages(void)
{
    int senderId;
    void *pMessage;

    while (Network_PollReceivedMessageBuffer((int)&senderId, &pMessage) != 0) {
        if (senderId == 0)
            OptionMenu_HandleItemNotification((int)&senderId, (int *)pMessage);
        else
            OptionMenu_HandleNetworkNotification((DPID *)&senderId, (BYTE *)pMessage);
    }
}

// Arms the countdown of every option record that has not been started yet.
// FUNCTION: CMR2 0x00500ec0
void OptionMenu_ArmRecordCountdowns(void)
{
    if (OptionMenu_GetRecordTransitionMode(2) == 0)
        CGameInfo::StartOptionRecordTransition(2, 0x28, 0);
    if (OptionMenu_GetRecordTransitionMode(6) == 0)
        CGameInfo::StartOptionRecordTransition(6, 0xf, 0);
    if (OptionMenu_GetRecordTransitionMode(7) == 0)
        CGameInfo::StartOptionRecordTransition(7, 0xf, 0);
    if (OptionMenu_GetRecordTransitionMode(4) == 0)
        CGameInfo::StartOptionRecordTransition(4, 0x1e, 0);
    if (OptionMenu_GetRecordTransitionMode(5) == 0)
        CGameInfo::StartOptionRecordTransition(5, 0x1e, 1);
    if (OptionMenu_GetRecordTransitionMode(0) == 0) {
        CGameInfo::StartOptionRecordTransition(0, 0x14, 1);
        return;
    }
    if (OptionMenu_GetRecordTransitionMode(0) == 2) {
        if (OptionMenu_GetRecordTransitionMode(1) == 0)
            CGameInfo::StartOptionRecordTransition(1, 0x14, 0);
        if (OptionMenu_GetRecordTransitionMode(3) == 0)
            CGameInfo::StartOptionRecordTransition(3, 0x14, 0);
    }
}

void OptionMenu_ResetModeCursor(void);
extern FixAngles g_unk0x005273c0[12];
extern int g_unk0x008313c8[96];
void OptionPreview_SetWheelNodesVisible(int index, char visible);
void OptionPreview_SetTargetWheelAngles(int param1, short *param2, int param3);

// Boot state of the country menus: marks the item of the selected country and
// points its two record tables at the matching entry.
// FUNCTION: CMR2 0x00500df0
void OptionMenu_EnterCountryState(Unk0049c2c0 *p1, BYTE p2)
{
    int i;

    OptionMenu_ResetModeCursor();
    for (i = 0; i < (int)(CGameInfo::GetConfiguredPlayerCount() & 0xff); i = i + 1) {
        if (i == CGameInfo::GetActiveOptionSlot()) {
            OptionPreview_SetWheelNodesVisible(i, 1);
            OptionPreview_SetTargetWheelAngles(i, (short *)&g_unk0x005273c0[OptionMenu_GetColumnWeight(
                             OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14])], 0);
            OptionMenu_SetTargetLayoutPairs(i, &g_unk0x008313c8[OptionMenu_GetColumnWeight(
                             OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14]) * 8], 1);
        } else {
            OptionPreview_SetWheelNodesVisible(i, 0);
        }
    }
    CGame::PromoteCallbackEntryByRule(p1, p2, 0, 2);
    g_unk0x0082b0a0 = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x005011d0
int OptionMenu_GetPlayerSlotOffset(void)
{
    return g_unk0x0082b0a8;
}

// FUNCTION: CMR2 0x005011e0
void OptionMenu_SetPlayerSlotOffset(int value)
{
    g_unk0x0082b0a8 = value;
}

// FUNCTION: CMR2 0x005011f0
int OptionMenu_GetSlotSelector(int index)
{
    return g_unk0x0082af90[index];
}

// FUNCTION: CMR2 0x00501200
int OptionMenu_GetSlotBaseSelector(int index)
{
    return g_unk0x0082af78[index];
}

// FUNCTION: CMR2 0x00501210
void OptionMenu_SetSlotSelector(int index, int value)
{
    g_unk0x0082af90[index] = value;
}

// FUNCTION: CMR2 0x00501510
int OptionMenu_GetBackgroundRoot(void)
{
    return g_unk0x0082b1b4;
}

void OptionMenu_CreateBackgroundWorld(void);
void OptionMenu_LoadArchives(void);
void OptionPreview_FillTextureSetIndices(void);
void RallyData_LoadCurrentRallyWeatherTextures(void);
void RallyData_SetupWeatherTextureEntries(void);
int *OptionMenu_GetCommonArchive(void);
void Particle_BuildTriangleStripIndices(void);
void Graphics_SetShadowGeometryState(int value);

// Format of the environment texture of the track. The %s is the install
// directory returned by the setup.
// GLOBAL: CMR2 0x00527070
char g_str0x00527070[] = "%s\\textures\\environment\\environment.tga";

// Starts the option menu background: clear colour and blend mode, then the
// option menu data, the world, the weather textures and their stage entries.
// Finally it registers the environment texture of the track, at the resolution
// the display option asks for.
// The only differences against the original are the call sites reccmp shows as
// <OFFSETn>: 0x4b1150, 0x503ea0, 0x5040f0 and the two 0x4b23c0 calls go to
// functions that are still annotated TODO, so reccmp cannot name them.
// FUNCTION: CMR2 0x00501520
void OptionMenu_InitBackground(void)
{
    CGraphics::SetClearColour(1, 0x8d, 0x97, 0x9f);
    Font_SetBlendMode(1);
    CGameInfo::SaveOptionMenuCallbackCount();
    OptionMenu_CreateBackgroundWorld();
    OptionMenu_LoadArchives();
    Particle_BuildTriangleStripIndices();
    CGame::SetSectorDrawState(3);
    RallyData_LoadCurrentRallyWeatherTextures();
    RallyData_SetupWeatherTextureEntries();
    OptionPreview_FillTextureSetIndices();
    sprintf(CFrontend::m_stringDest, g_str0x00527070, CInstallInfo::GetSetupRepDir());
    Graphics_SetShadowGeometryState(!CGameInfo::IsRecordFlagSet(0xf));
    if (CGameInfo::IsRecordFlagSet(0x10))
        Graphics_ReserveCubeMapsAndLoadEnvironment(CFrontend::m_stringDest, 0, (GenericFile *)OptionMenu_GetCommonArchive(), 0x80);
    else
        Graphics_ReserveCubeMapsAndLoadEnvironment(CFrontend::m_stringDest, 0, (GenericFile *)OptionMenu_GetCommonArchive(), 0x40);
}

// FUNCTION: CMR2 0x00501d00
void OptionMenu_ClearRecordTransition(int index)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];

    p->field_0x8 = 0;
    p->field_0x4 = 0;
    p->field_0x0 = 0;
    p->field_0xc = 0;
}

// FUNCTION: CMR2 0x00501d20
void OptionMenu_ResetRecordTransitions(int count)
{
    int i;

    g_unk0x0082b1bc = count;
    for (i = 0; i < g_unk0x0082b1bc; i++)
        OptionMenu_ClearRecordTransition(i);
}

// FUNCTION: CMR2 0x005021c0
int OptionMenu_GetRecordTransitionMode(int index)
{
    return g_unk0x0082b2c0[index].field_0xc;
}

// FUNCTION: CMR2 0x005021e0
void OptionMenu_FinishRecordTransitions(void)
{
    int i;
    Unk0x0082b2c0 *p;

    if (g_unk0x0082b1bc > 0) {
        p = g_unk0x0082b2c0;
        i = g_unk0x0082b1bc;
        do {
            p->field_0x0 = 0x10000;
            p->field_0xc = 2;
            p++;
        } while (--i != 0);
    }
}

// FUNCTION: CMR2 0x00502210
BYTE *OptionMenu_GetStatusMenu(void)
{
    return g_unk0x0082ba28;
}

// FUNCTION: CMR2 0x00502220
BYTE *OptionMenu_GetAdvancedOptionsMenu(void)
{
    return g_unk0x0082b488;
}

// FUNCTION: CMR2 0x00502990
BYTE OptionMenu_GetColumnDirtyFlag(int i, int j)
{
    return g_unk0x0082bf20[i][j];
}

// Option menu builders and loaders that live in the cascade scaffold.
void OptionMenu_RefreshStageSetup(void);
void OptionMenu_LoadRallyRecords(void);
void OptionMenu_InitDefaultRecordTables(void);
void OptionMenu_InitBackground(void);
BYTE OptionMenu_LoadSounds(void);
void OptionMenu_ConfigureSoundHandles(void);
void OptionMenu_LoadFonts(void);
BYTE OptionMenu_LoadRegionalText(void);
void OptionMenu_BuildAndActivatePages(void);
void SavedGames_LoadRecords(void);
bool SavedGames_ReleaseRecords(void);
void NetPlayers_SendCarClass(BYTE carClass);
HRESULT Sound_StartLoopingMusicStream(int param1);

// Starts the option menu state: rebuilds the option records, starts the menu
// music and asks for the next state (level 2) of the grouped callback machine.
// FUNCTION: CMR2 0x00500c80
void OptionMenu_EnterStartState(Unk0049c2c0 *p1, BYTE state)
{
    char path[MAX_PATH];
    int i;

    g_unk0x0082b0ac = 0;
    CGameInfo::SetInputAndGamePaused(1);
    if (CGameInfo::GetGameModeOptionBit19() != 0)
        NetPlayers_SendCarClass(1);
    Menu_SetInputStateFlag(0);
    CInput::SetInputRepeatTimingParameters(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
    g_unk0x0082b0a8 = CGameInfo::GetConfiguredPlayerCount() & 0xff;
    OptionMenu_RefreshStageSetup();
    OptionMenu_LoadRallyRecords();
    OptionMenu_InitDefaultRecordTables();
    OptionMenu_InitBackground();
    SavedGames_LoadRecords();
    CGame::RegisterCallback(SavedGames_ReleaseRecords, NULL);
    Sound_Init(0x5622, 2, 0x10, 0);
    OptionMenu_LoadSounds();
    OptionMenu_ConfigureSoundHandles();
    OptionMenu_LoadFonts();
    OptionMenu_LoadRegionalText();
    OptionMenu_BuildAndActivatePages();

    if (!(CGameInfo::GetConfiguredGameMode() == 0 || CGameInfo::GetConfiguredGameMode() == 1)) {
        g_unk0x00527000 = 1;
    } else {
        g_unk0x00527000 = 0;
        if ((int)RallyDataStageIndex() % 4 == 0)
            g_unk0x00527000 = 1;
    }

    // The original re-reads the flag before each pair of stores: both arms of
    // the conditional hold the same value, so the branch is gone from the
    // binary but the load and its test are still there.
    for (i = 0; i < (int)(CGameInfo::GetConfiguredPlayerCount() & 0xff); i = i + 1) {
        if (g_unk0x00527000) {
            g_unk0x0082af90[i] = 360000;
            g_unk0x0082af78[i] = 0x3c;
        } else {
            g_unk0x0082af90[i] = 360000;
            g_unk0x0082af78[i] = 0x3c;
        }
    }

    sprintf(path, g_str0x0052703c, CInstallInfo::GetMusicDir());
    CSound::OpenStreamingMusicFile(path);
    CSound::SetMusicStreamVolume(CGameInfo::GetMasterSoundVolume());
    Sound_StartLoopingMusicStream(1);
    CGame::PromoteCallbackEntryByRule(p1, state, 0, 2);
    g_unk0x0082af8c = 1;
}

// FUNCTION: CMR2 0x00502d40
int OptionMenu_GetColumnBaseWeight(int index)
{
    return g_unk0x00527098[index];
}

struct Unk0x0052ebc0 *RallyData_GetDriverGroupRecord(int index);

// Copies every option record from the rally data into the working table and
// clears the per-record dirty words.
// FUNCTION: CMR2 0x00502d50
void OptionMenu_LoadRallyRecords(void)
{
    int i;
    int j;
    int *pDst;
    int *pDirty;
    int *pSrc;

    i = 0;
    if (CGameInfo::GetConfiguredPlayerCount() > 0) {
        pDirty = (int *)g_unk0x0082c040;
        pDst = (int *)g_unk0x0082c070;
        do {
            pSrc = (int *)RallyData_GetDriverGroupRecord(i);
            i++;
            memcpy(pDst, pSrc, 0x148);
            for (j = 0; j < 3; j++)
                pDirty[j] = 0;
            pDst += 0x52;
            pDirty += 3;
        } while (i < (int)(CGameInfo::GetConfiguredPlayerCount() & 0xff));
    }
}

// FUNCTION: CMR2 0x00503930
int OptionMenu_GetRecordGroupWeight(int index)
{
    return g_unk0x005270b4[index];
}

// FUNCTION: CMR2 0x00503940
BYTE OptionMenu_GetRecordGroupAppliedFlag(int i, int j)
{
    return g_unk0x0082c040[i][j];
}

// FUNCTION: CMR2 0x00509d00
void OptionPreview_ClearSkyFadeShape(int index)
{
    Unk0x0082d220 *pRec = &g_unk0x0082d220[index];
    int i;

    for (i = 0; i < 4; i++)
        pRec->field_0x22c.v[i] = 0;
}

// FUNCTION: CMR2 0x00509d90
void OptionPreview_ClearColourFractions(int index)
{
    Unk0x0082d220 *pRec = &g_unk0x0082d220[index];
    int i;

    for (i = 0; i < 4; i++)
        pRec->field_0x23c.v[i] = 0;
}

// FUNCTION: CMR2 0x0050a020
int OptionPreview_UsesWheelVariantSix(int mode, int type)
{
    if (type == 6 && mode != 11 && mode != 8 && mode != 10 && mode != 13)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x0050a050
int OptionPreview_UsesLowWheelVariant(int mode, int type)
{
    if (type <= 1 && mode != 11 && mode != 8 && mode != 10 && mode != 13)
        return 1;
    return 0;
}

// Rotation angles of the 12 parts of the option menu car preview.
// GLOBAL: CMR2 0x005273c0
FixAngles g_unk0x005273c0[12] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0xfc00, 0x0000, 0x0000, 0x0000,
    0x0000, 0x01c7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0400, 0x0000, 0x0000, 0x0000, 0x0000, 0x01c7, 0x0000, 0x0000,
    0xfc00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0238, 0x0000, 0x0000,
    0x0000, 0x0400, 0xffbc, 0x0000, 0x0400, 0x0000, 0x0000, 0x0000,
    0x0400, 0x0000, 0x0000, 0x0000, 0x0400,
};

// Wheel vertices of the option menu car preview: one 12x4 block of vertices
// per car (14 cars, one per id of g_unk0x00516b40.ids).
// GLOBAL: CMR2 0x00527420
FixVector g_unk0x00527420[672] = {
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -134742, -27459, -34013 }, { -134742, -27459, -34013 },
    { -134742, -27459, -34013 }, { -134742, -27459, -34013 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 85131, -51118, 49807 }, { 85131, -51118, -49807 },
    { -83820, -51118, 49807 }, { -83820, -49020, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 87752, -3538, 57671 }, { 87752, -3538, -57671 },
    { -79233, -3538, 57671 }, { -79233, -3538, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 87752, -3538, 57671 }, { 87752, -3538, -57671 },
    { -79233, -3538, 57671 }, { -79233, -3538, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 85131, -48758, 49807 }, { 85131, -48758, 49807 },
    { 85131, -48758, -49807 }, { 85131, -48758, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -136052, -26411, 31260 }, { -136052, -26411, 31260 },
    { -136052, -26411, 31260 }, { -136052, -26411, 31260 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -85196, -48496, 49807 }, { -85196, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -85196, -3276, 57671 }, { -85196, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -85196, -3276, 57671 }, { -85196, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 36700 }, { 121831, 0, 36700 },
    { 121831, 0, -36700 }, { 121831, 0, -36700 },
    { -126156, -27656, -28508 }, { -126156, -27656, -28508 },
    { -126156, -27656, -28508 }, { -126156, -27656, -28508 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 80543, -43909, 55705 }, { 80543, -43909, -55705 },
    { -84475, -43909, 55705 }, { -84475, -43909, -55705 },
    { 39321, -11796, 0 }, { 39321, -11796, 0 },
    { 39321, -11796, 0 }, { 39321, -11796, 0 },
    { 86441, 6553, 0 }, { 86441, 6553, 0 },
    { 86441, 6553, 0 }, { 86441, 6553, 0 },
    { 80543, -1310, 55705 }, { 80543, -1310, -55705 },
    { -84475, -1310, 55705 }, { -84475, -1310, -55705 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 80543, -1310, 55705 }, { 80543, -1310, -55705 },
    { -84475, -1310, 55705 }, { -84475, -1310, -55705 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 80543, -46530, 47185 }, { 80543, -46530, 47185 },
    { 80543, -46530, -47185 }, { 80543, -46530, -47185 },
    { 127008, -3276, 35389 }, { 127008, -3276, 35389 },
    { 127008, -3276, -35389 }, { 127008, -3276, -35389 },
    { -139853, -28639, -21299 }, { -139853, -28639, -21299 },
    { -139853, -28639, -21299 }, { -139853, -28639, -21299 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -129236, -31326, 32243 }, { -129236, -31326, 32243 },
    { -129236, -31326, 32243 }, { -129236, -31326, 32243 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -134217, -21757, 30539 }, { -134217, -21757, 30539 },
    { -134217, -21757, 30539 }, { -134217, -21757, 30539 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -124518, -28835, -28246 }, { -124518, -28835, -28246 },
    { -124518, -28835, -28246 }, { -124518, -28835, -28246 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 80543, -43909, 55705 }, { 80543, -43909, -55705 },
    { -84475, -43909, 55705 }, { -84475, -43909, -55705 },
    { 39321, -11796, 0 }, { 39321, -11796, 0 },
    { 39321, -11796, 0 }, { 39321, -11796, 0 },
    { 86441, 6553, 0 }, { 86441, 6553, 0 },
    { 86441, 6553, 0 }, { 86441, 6553, 0 },
    { 80543, -1310, 55705 }, { 80543, -1310, -55705 },
    { -84475, -1310, 55705 }, { -84475, -1310, -55705 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 80543, -1310, 55705 }, { 80543, -1310, -55705 },
    { -84475, -1310, 55705 }, { -84475, -1310, -55705 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 80543, -46530, 47185 }, { 80543, -46530, 47185 },
    { 80543, -46530, -47185 }, { 80543, -46530, -47185 },
    { 127008, -3276, 35389 }, { 127008, -3276, 35389 },
    { 127008, -3276, -35389 }, { 127008, -3276, -35389 },
    { -154140, -26083, -17498 }, { -154140, -26083, -17498 },
    { -154140, -26083, -17498 }, { -154140, -26083, -17498 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 66191, -45875, 43909 }, { 66191, -45875, -43909 },
    { -70057, -45875, 43909 }, { -70057, -45875, -43909 },
    { 37355, -11141, 0 }, { 37355, -11141, 0 },
    { 37355, -11141, 0 }, { 37355, -11141, 0 },
    { 61538, 8519, 0 }, { 61538, 8519, 0 },
    { 61538, 8519, 0 }, { 61538, 8519, 0 },
    { 66191, -9830, 43909 }, { 66191, -9830, -43909 },
    { -70057, -9830, 43909 }, { -70057, -9830, -43909 },
    { 58327, -28835, 0 }, { 58327, -28835, 0 },
    { -58327, -28835, 0 }, { -58327, -28835, 0 },
    { 66191, -9830, 43909 }, { 66191, -9830, -43909 },
    { -70057, -9830, 43909 }, { -70057, -9830, -43909 },
    { 73990, 17694, 0 }, { 73990, 17694, 0 },
    { -20971, 52428, 0 }, { -20971, 52428, 0 },
    { 66191, -45875, 43909 }, { 66191, -45875, 43909 },
    { 66191, -45875, -43909 }, { 66191, -45875, -43909 },
    { 108068, 3932, 34734 }, { 108068, 3932, 34734 },
    { 108068, 3932, -34734 }, { 108068, 3932, -34734 },
    { -102498, -28377, 196 }, { -102498, -28377, 196 },
    { -102498, -28377, 196 }, { -102498, -28377, 196 },
    { 65, -30801, 0 }, { 65, -30801, 0 },
    { 65, -30801, 0 }, { 65, -30801, 0 },
    { 58327, -28835, 0 }, { 58327, -28835, 0 },
    { -58327, -28835, 0 }, { -58327, -28835, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -121896, -31784, -36438 }, { -121896, -31784, 36438 },
    { -121896, -31784, -36438 }, { -121896, -31784, 36438 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 73334, -41287, 42598 }, { 73334, -41287, -42598 },
    { -79888, -41287, 42598 }, { -79888, -41287, -42598 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 73334, -1966, 42598 }, { 73334, -1966, -42598 },
    { -79888, -1966, 42598 }, { -79888, -1966, -42598 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 73334, -1966, 42598 }, { 73334, -1966, -42598 },
    { -79888, -1966, 42598 }, { -79888, -1966, -42598 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 73334, -41287, 42598 }, { 73334, -41287, 42598 },
    { 73334, -41287, -42598 }, { 73334, -41287, -42598 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -115015, -17694, -25952 }, { -115015, -17694, 25952 },
    { -115015, -17694, -25952 }, { -115015, -17694, 25952 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, -49807 },
    { -96927, -48496, 49807 }, { -96927, -48496, -49807 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 81199, -3276, 57671 }, { 81199, -3276, -57671 },
    { -92340, -3276, 57671 }, { -92340, -3276, -57671 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 78577, -48496, 49807 }, { 78577, -48496, 49807 },
    { 78577, -48496, -49807 }, { 78577, -48496, -49807 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -124125, -26738, -1769 }, { -124125, -26738, -1769 },
    { -124125, -26738, -1769 }, { -124125, -26738, -1769 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 73334, -41287, 42598 }, { 73334, -41287, -42598 },
    { -79888, -41287, 42598 }, { -79888, -41287, -42598 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 22937, -19660, 0 }, { 22937, -19660, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 73334, -1966, 42598 }, { 73334, -1966, -42598 },
    { -79888, -1966, 42598 }, { -79888, -1966, -42598 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 73334, -1966, 42598 }, { 73334, -1966, -42598 },
    { -79888, -1966, 42598 }, { -79888, -1966, -42598 },
    { 106102, 11796, 0 }, { 106102, 11796, 0 },
    { -43253, 54394, 0 }, { -43253, 54394, 0 },
    { 73334, -41287, 42598 }, { 73334, -41287, 42598 },
    { 73334, -41287, -42598 }, { 73334, -41287, -42598 },
    { 121831, 0, 40632 }, { 121831, 0, 40632 },
    { 121831, 0, -40632 }, { 121831, 0, -40632 },
    { -127336, -34406, -23986 }, { -127336, -34406, -23986 },
    { -127336, -34406, -23986 }, { -127336, -34406, -23986 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 0, -38010, 0 }, { 0, -38010, 0 },
    { 75300, -38010, 0 }, { 75300, -38010, 0 },
    { -81199, -38010, 0 }, { -81199, -38010, 0 },
    { 93650, -48496, 44564 }, { 93650, -48496, -44564 },
    { -75300, -48496, 44564 }, { -75300, -48496, -44564 },
    { 33423, -7864, 0 }, { 33423, -7864, 0 },
    { 33423, -7864, 0 }, { 33423, -7864, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 67436, 5898, 0 }, { 67436, 5898, 0 },
    { 93650, -4587, 44564 }, { 93650, -4587, -44564 },
    { -75300, -4587, 44564 }, { -75300, -4587, -44564 },
    { 75300, 38010, 0 }, { 75300, 38010, 0 },
    { -81199, 38010, 0 }, { -81199, 38010, 0 },
    { 93650, -4587, 44564 }, { 93650, -4587, -44564 },
    { -75300, -4587, 44564 }, { -75300, -4587, -44564 },
    { 102825, 16384, 0 }, { 102825, 16384, 0 },
    { -22937, 51118, 0 }, { -22937, 51118, 0 },
    { 93650, -48496, 44564 }, { 93650, -48496, 44564 },
    { 93650, -48496, -44564 }, { 93650, -48496, -44564 },
    { 137625, -65, 35389 }, { 137625, -65, 35389 },
    { 137625, -65, -35389 }, { 137625, -65, -35389 },
    { -126418, -25559, 25559 }, { -126418, -25559, 25559 },
    { -126418, -25559, 25559 }, { -126418, -25559, 25559 },
    { 2621, -29491, 0 }, { 2621, -29491, 0 },
    { 2621, -29491, 0 }, { 2621, -29491, 0 },
    { 91029, -29491, 0 }, { 91029, -29491, 0 },
    { -69402, -29491, 0 }, { -69402, -29491 },
};

// Projected 2D points of the option menu car preview: 4 points per part.
// GLOBAL: CMR2 0x008313c8
int g_unk0x008313c8[96];

void OptionPreview_RotateSlotVector(int param1, FixVector *param2, FixVector *param3, FixAngles *param4);
void Game_ProjectMenuBackgroundPoint(int *param1, FixVector *param2);

// Projects the wheel vertices of one car's option menu preview: every part's
// 4 vertices are rotated by the part angles and the projected points are
// stored in the shared preview buffer.
// FUNCTION: CMR2 0x0050f120
void OptionPreview_ProjectWheelVertices(int param1)
{
    FixVector *pVec;
    int *pOut;
    FixAngles *pAngles;
    FixVector local;
    int k;

    pOut = g_unk0x008313c8;
    pAngles = g_unk0x005273c0;
    pVec = &g_unk0x00527420[param1 * 48];
    do {
        for (k = 0; k < 4; k++) {
            OptionPreview_RotateSlotVector(0, pVec, &local, pAngles);
            Game_ProjectMenuBackgroundPoint(pOut, &local);
            pVec++;
            pOut += 2;
        }
        pAngles++;
    } while ((int)pAngles < (int)(g_unk0x005273c0 + 12)); // 0x527420 in the original
}

// FUNCTION: CMR2 0x0050f1c0
void OptionMenu_ResetActivePage(void)
{
    g_pMenu0x00831778 = g_pMenu0x0083177c;
}

// Per-frame update of the option menu: refreshes the active menu, polls the
// input device and forwards its state to Menu_Update.
// FUNCTION: CMR2 0x0050f1d0
void OptionMenu_UpdateActivePage(void)
{
    int input;
    DeviceInfo *pDevice;
    Menu *pNextMenu;

    OptionMenu_ResetActivePage();
    Sound_UpdateMusicStreaming();
    CInput::UpdateAllAvailableDevices();
    input = 0;
    if (CGameInfo::GetActiveOptionSlot() == 1) {
        if (CGameInfo::IsConfiguredMultiplayer() == 0)
            input = 1;
    }
    Input_TranslatePedalsToMenuKeys();
    pDevice = CInput::GetAvailableDeviceRecord(input);
    Input_MergeAssignedJoystickButtons(input, pDevice);
    pNextMenu = (Menu *)Menu_Update(g_pMenu0x00831778, pDevice->field_0x8);
    if (pNextMenu != NULL)
        g_pMenu0x0083177c = pNextMenu;
}

// FUNCTION: CMR2 0x0050f230
void OptionMenu_DrawActivePage(void)
{
    Menu_CallCallback2(g_pMenu0x00831778);
}

// Option menu text file prefixes, indexed by game language ("s" + language).
// GLOBAL: CMR2 0x0052962c
char g_str0x0052962c[8] = "spolish";
// GLOBAL: CMR2 0x00529634
char g_str0x00529634[8] = "sengusa";
// GLOBAL: CMR2 0x0052963c
char g_str0x0052963c[8] = "sgerman";
// GLOBAL: CMR2 0x00529644
char g_str0x00529644[12] = "sitalian";
// GLOBAL: CMR2 0x00529650
char g_str0x00529650[12] = "sspanish";
// GLOBAL: CMR2 0x0052965c
char g_str0x0052965c[8] = "sfrench";
// GLOBAL: CMR2 0x00529664
char g_str0x00529664[12] = "senglish";

extern char g_strTxtFormat[];
extern int g_unk0x00831880;
extern BYTE g_unk0x00831884;
int *OptionMenu_GetCommonArchive(void);
int *OptionMenu_GetLanguageArchive(void);
int *OptionMenu_GetStageArchive(void);
bool OptionMenu_ReleaseSoundBuffer(void);
int OptionMenu_ReleaseTextures(void);

// Loads the option menu text file of the current region and language and
// registers it with the frontend.
// FUNCTION: CMR2 0x0050f240
BYTE OptionMenu_LoadRegionalText(void)
{
    char *europe[5];
    char *usa[3];
    char *japan[1];
    char *poland[1];
    char **names;

    europe[0] = g_str0x00529664;
    europe[1] = g_str0x0052965c;
    europe[2] = g_str0x00529650;
    europe[3] = g_str0x00529644;
    europe[4] = g_str0x0052963c;
    usa[0] = g_str0x00529634;
    usa[1] = g_str0x0052965c;
    usa[2] = g_str0x00529650;
    japan[0] = g_str0x00529664;
    poland[0] = g_str0x0052962c;

    names = NULL;
    switch (CGameInfo::GetGameRegion()) {
    case 0:
        names = europe;
        break;
    case 1:
        names = usa;
        break;
    case 2:
        names = japan;
        break;
    case 3:
        names = poland;
    }

    sprintf(CFrontend::m_stringDest, g_strTxtFormat, names[CGameInfo::GetGameLanguage()]);
    g_unk0x00831880 = (int)CGenericFileLoader::FindFile((GenericFile *)OptionMenu_GetLanguageArchive(),
                                                       CFrontend::m_stringDest, &g_unk0x00831884, NULL, 0);
    // the original tests the address of the buffer, so this is always true
    if (&g_unk0x00831880 != NULL) {
        CFrontend::BuildLocalizedTextStringTable(1, 0x146, (BYTE **)&g_unk0x00831880);
        CGame::RegisterCallback(OptionMenu_ReleaseSoundBuffer, NULL);
        return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x0050f620
int *OptionMenu_GetCommonArchive(void)
{
    return g_unk0x00831a90;
}

// FUNCTION: CMR2 0x0050f630
int *OptionMenu_GetLanguageArchive(void)
{
    return g_unk0x00831ab0;
}

// FUNCTION: CMR2 0x0050f640
int *OptionMenu_GetStageArchive(void)
{
    return g_unk0x00831aa0;
}

unsigned char GameInfo_GetFrontendSessionFlag(void);
void OptionMenu_ArmRecordCountdowns(void);
void OptionMenu_UpdateRecordTransitions(void);
void OptionMenu_UpdateActivePage(void);
void OptionPreview_AnimateWheelRotations(void);
void GameInfo_ProcessNetworkMessages(void);
int NetPlayers_GetInputFrameDelta(void);
void NetPlayers_SendType16Notification(void);
BOOL Network_GetSessionStateFlag(void);
void StageUI_PublishNetworkRaceWins(void);
extern int g_unk0x0082b0a4;

// Set when the option menu has finished fading; drawn by the boot state 0x500c80.

// Frame callback of the boot state machine: while the fade has not timed out
// (or the menu is not the options one) it keeps the option menu alive,
// otherwise it lets the state machine advance.
// FUNCTION: CMR2 0x00500f80
void OptionMenu_UpdateFadeState(Unk0049c2c0 *p1, BYTE state)
{
    if (GameInfo_GetFrontendSessionFlag() == 0) {
        if ((unsigned int)(CMain::GetFrameDelta() - g_unk0x0082b0a0) > 0x17d4
            && CGameInfo::GetGameModeOptionBit19() != 0 && CGameInfo::GetConfiguredGameMode() != 0xa)
            goto other;
        if (CMain::GetFrameDelta() - NetPlayers_GetInputFrameDelta() > GameInfo_GetSessionField397C() * 6000
            && CGameInfo::GetGameModeOptionBit19() != 0 && CGameInfo::GetConfiguredGameMode() == 0xa
            && Network_GetSessionStateFlag() != 0 && GameInfo_GetSessionField397C() != 0)
            goto other;
        OptionMenu_ArmRecordCountdowns();
        OptionMenu_UpdateRecordTransitions();
        OptionMenu_UpdateActivePage();
        OptionPreview_AnimateWheelRotations();
        goto tail;
    }
other:
    if (CMain::GetFrameDelta() - NetPlayers_GetInputFrameDelta() > GameInfo_GetSessionField397C() * 6000
        && CGameInfo::GetConfiguredGameMode() == 0xa && Network_GetSessionStateFlag() != 0 && GameInfo_GetSessionField397C() != 0) {
        g_unk0x0082b0ac = 1;
        StageUI_PublishNetworkRaceWins();
        NetPlayers_SendType16Notification();
    }
    CGame::PromoteCallbackEntryByRule(p1, state, 0, 2);
    g_unk0x0082b0a4 = CMain::GetFrameDelta();
tail:
    if (CGameInfo::GetGameModeOptionBit19() != 0)
        GameInfo_ProcessNetworkMessages();
}

// FUNCTION: CMR2 0x005011a0
void CGameInfo::SaveOptionMenuCallbackCount(void)
{
    g_unk0x0082af88 = CGame::GetCallbackCount();
}

// FUNCTION: CMR2 0x004f8a70
char *CGameInfo::GetCodeEntryText(int index)
{
    return CFrontend::GetTextString(index + 0x1f1);
}

// FUNCTION: CMR2 0x005011b0
int CGameInfo::GetActiveOptionSlot(void)
{
    return GetConfiguredPlayerCount() - g_unk0x0082b0a8;
}

// FUNCTION: CMR2 0x00500500
void CGameInfo::StartOptionMenuTimeout(void)
{
    g_unk0x0082ac58 = 1;
    g_unk0x0082ac5c = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x005012c0
int CGameInfo::GetOptionMenuRemainingTime(void)
{
    int result;

    result = g_unk0x0082b0a0 - CMain::GetFrameDelta() + 0x17d5;
    if (result < 0)
        result = 0;
    return result;
}

// FUNCTION: CMR2 0x0040a420
int CGameInfo::GetNetworkStageBestTime(int index)
{
    if (GetConfiguredGameMode() == 0xc)
        return g_netStageBest[0];
    return g_netStageBest[index - 1];
}

// FUNCTION: CMR2 0x00501cc0
void CGameInfo::StartOptionRecordTransition(int index, int param2, int param3)
{
    Unk0x0082b2c0 *pEntry;

    pEntry = &g_unk0x0082b2c0[index];
    pEntry->field_0x8 = CMain::GetFrameDelta();
    pEntry->field_0x4 = param2;
    pEntry->field_0x0 = 0;
    pEntry->field_0xc = 1;
    pEntry->field_0x10 = param3;
}

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x005004c0
int CGameInfo::IsOptionMenuTimeoutPulseOn(void)
{
    unsigned int delta;

    if (g_unk0x0082ac58 != 0) {
        delta = CMain::GetFrameDelta() - g_unk0x0082ac5c;
        return (unsigned char)~(delta / 10) & 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x00501230
int CGameInfo::GetPreviewLayoutMode(void)
{
    if (RallyDataStageIndex() == 0xa ||
        GetConfiguredGameMode() == 2 ||
        GetConfiguredGameMode() == 3 ||
        GetConfiguredGameMode() == 8 ||
        GetConfiguredGameMode() == 9 ||
        GetConfiguredGameMode() == 0xa)
        return 1;
    return 2;
}

// 0x50-byte entry of the table at 0x82c6c8.
struct Unk0x0082c6c8 {
    BYTE field_0x0[0x1c];
    int field_0x1c;
    BYTE field_0x20[0x2c];
    int field_0x4c;
};

// GLOBAL: CMR2 0x0082c6c8
Unk0x0082c6c8 g_unk0x0082c6c8[16];
// GLOBAL: CMR2 0x0082ca1c
BYTE g_unk0x0082ca1c;
// GLOBAL: CMR2 0x0082c6c0
int g_unk0x0082c6c0;
// GLOBAL: CMR2 0x0082cb44
int g_unk0x0082cb44;

// Selects a new entry only when the current entry has no pending activity.
// match 75%: the original pushes ESI in the prologue and only then reloads the
// parameter (at [esp+8]); this compiler sinks the push of the callee-saved
// register into the taken branch and computes the return value one store early
// (identical byte count, different scheduling).
// FUNCTION: CMR2 0x00505e10
int CGameInfo::SelectIdleOptionPreviewEntry(BYTE param1)
{
    int result = 0;
    Unk0x0082c6c8 *pEntry;
    if (g_unk0x0082ca1c == 0xff ||
        ((pEntry = &g_unk0x0082c6c8[(signed char)g_unk0x0082ca1c])->field_0x4c == 0 &&
         pEntry->field_0x1c == 0)) {
        result = 1;
        g_unk0x0082ca1c = param1;
        g_unk0x0082c6c0 = CMain::GetFrameDelta();
        g_unk0x0082cb44 = 0;
    }
    return result;
}

// Adds the mesh of a scene node to a stage mesh record (g_unk0x0082d220): stores
// the node, its mesh and its vertex count in the slot of the node's mesh id,
// expands the +/-100 bounding box with every vertex and stores the centre and
// the half size of the slot. Bumps the counter of used slots.
// FUNCTION: CMR2 0x00507290
void OptionPreview_AddNodeMeshToRecord(SceneNode *pNode, Unk0x0082d220 *pRecord)
{
    Unk0x0082d220VertexF *pVertex;
    FixVector halfSize;
    Mesh *pMesh;
    WORD count;
    int slot;
    int maxX, maxY, maxZ;
    int minX, minY, minZ;
    int x, y, z;
    int i;

    slot = (pNode->flags & 0xff) - 5;
    pMesh = (Mesh *)pNode->pObject;
    if (pMesh != NULL && pNode->type == 0) {
        pRecord->pNodes[slot] = pNode;
        pRecord->pMeshes[slot] = pMesh;
        count = (WORD)Mesh_GetField0x10(pMesh);
        pRecord->vertexCount[slot] = count;
        if (pRecord->pMeshes[slot] != NULL && count > 0) {
            maxZ = -0x640000;
            maxY = -0x640000;
            maxX = -0x640000;
            minZ = 0x640000;
            minY = 0x640000;
            minX = 0x640000;
            for (i = 0; i < (int)pRecord->vertexCount[slot]; i++) {
                x = (int)(__int64)(((Unk0x0082d220VertexF *)pRecord->pMeshes[slot]->pVertexData)[i].x *
                                   CGraphics::m_65536);
                pVertex = ((Unk0x0082d220VertexF *)pRecord->pMeshes[slot]->pVertexData) + i;
                y = (int)(__int64)(pVertex->y * CGraphics::m_65536);
                z = (int)(__int64)(pVertex->z * CGraphics::m_65536);
                if (x > maxX)
                    maxX = x;
                if (x < minX)
                    minX = x;
                if (y > maxY)
                    maxY = y;
                if (y < minY)
                    minY = y;
                if (z > maxZ)
                    maxZ = z;
                if (z < minZ)
                    minZ = z;
                if (x >= 0) {
                    if (x > pRecord->field_0x21c)
                        pRecord->field_0x21c = x;
                } else {
                    if (x < pRecord->field_0x220)
                        pRecord->field_0x220 = x;
                }
                if (z >= 0) {
                    if (z > pRecord->field_0x224)
                        pRecord->field_0x224 = z;
                } else {
                    if (z < pRecord->field_0x228)
                        pRecord->field_0x228 = z;
                }
            }
            halfSize.x = minX - maxX;
            halfSize.y = minY - maxY;
            halfSize.z = minZ - maxZ;
            FixVecScale(&halfSize, &halfSize, 0x8000);
            pRecord->centre[slot].v[0] = maxX + halfSize.x;
            pRecord->centre[slot].v[1] = maxY + halfSize.y;
            pRecord->centre[slot].v[2] = maxZ + halfSize.z;
            pRecord->halfSize[slot].v[0] = maxX - pRecord->centre[slot].v[0];
            pRecord->halfSize[slot].v[1] = maxY - pRecord->centre[slot].v[1];
            pRecord->halfSize[slot].v[2] = maxZ - pRecord->centre[slot].v[2];
            pRecord->meshCount++;
        }
    }
}

void OptionPreview_SetMeshRenderFlags(int index, int param2, BYTE param3);

// Converts the source vertex data of every mesh of a stage mesh record back into
// its 0x30-byte mesh vertices, rebuilds the meshes, marks the option state of
// each slot and resets the 8 option meshes of the record.
// FUNCTION: CMR2 0x005074d0
void OptionPreview_RebuildStageMeshes(int index)
{
    Unk0x0082d220 *pRecord;
    FixVector position;
    FixVector normal;
    int i;
    int j;

    pRecord = &g_unk0x0082d220[index];
    if (pRecord->field_0x2a8 == 0)
        return;

    for (i = 0; i < (int)pRecord->meshCount; i++) {
        for (j = 0; j < (int)pRecord->vertexCount[i]; j++) {
            position = pRecord->pVertexData[i][j].position;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].x =
                (float)position.x * CGraphics::m_oneOver65536;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].y =
                (float)position.y * CGraphics::m_oneOver65536;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].z =
                (float)position.z * CGraphics::m_oneOver65536;
            normal = pRecord->pVertexData[i][j].normal;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].nx =
                (float)normal.x * CGraphics::m_oneOver65536;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].ny =
                (float)normal.y * CGraphics::m_oneOver65536;
            ((Unk0x0082d220VertexF *)pRecord->pMeshes[i]->pVertexData)[j].nz =
                (float)normal.z * CGraphics::m_oneOver65536;
        }
        if (pRecord->pNodes[i]->pObject != NULL)
            Mesh_Rebuild((Mesh *)pRecord->pNodes[i]->pObject);
        pRecord->field_0x26c[i] = 1;
    }
    for (i = 0; i < 8; i++)
        OptionPreview_SetMeshRenderFlags(index, 0, i);
}

void OptionPreview_LoadSkyColourRecord(BYTE *pColour);
void OptionPreview_UpdateStageSkyColours(int index);
void StageTiming_SelectRecordMeshBrightness(int index);
void OptionPreview_UpdateSkyFadeShape(int index);
void OptionPreview_UnpackColourFractions(int index);

// Mesh slot of the stage mesh record (g_unk0x0082d220) each of the four deform
// options deforms.
// GLOBAL: CMR2 0x00527364
BYTE g_unk0x00527364[4] = { 4, 5, 6, 7 };

// Applies the option record of a stage: commits the deformations of its mesh
// record, rebuilds the stage sky with the colours of the record's linked list of
// 13-byte entries and clears the option state of the meshes it marks as applied.
// FUNCTION: CMR2 0x00507650
void OptionPreview_ApplyStageDeformations(int index)
{
    Unk0x0082d220 *pRecord;
    BYTE *pRally;
    BYTE *pEntry;
    char next;
    int i;

    pRecord = &g_unk0x0082d220[index];
    OptionPreview_RebuildStageMeshes(index);
    pRally = (BYTE *)RallyData_GetDriverGroupRecord(index);
    pEntry = pRally + pRally[0x105] * 13;
    if (pRally[0x104] != 0) {
        while (pEntry != NULL) {
            OptionPreview_LoadSkyColourRecord(pEntry);
            OptionPreview_UpdateStageSkyColours(index);
            next = (char)pEntry[0xc];
            if (next == -1)
                break;
            pEntry = pRally + next * 13;
        }
    }
    StageTiming_SelectRecordMeshBrightness(index);
    for (i = 0; i < 4; i++) {
        if (((int *)(pRally + 0x12c))[i] != 0)
            pRecord->field_0x26c[g_unk0x00527364[i]] = 0;
    }
    OptionPreview_UpdateSkyFadeShape(index);
    OptionPreview_UnpackColourFractions(index);
}

// GLOBAL: CMR2 0x0082d120
FixVector g_unk0x0082d120;
// GLOBAL: CMR2 0x0082d12c
FixVector g_unk0x0082d12c;
// GLOBAL: CMR2 0x0082d138
FixVector g_unk0x0082d138;
// GLOBAL: CMR2 0x0082d144
int g_unk0x0082d144;
// GLOBAL: CMR2 0x0082d148
int g_unk0x0082d148;
// GLOBAL: CMR2 0x0082d14c
BYTE g_unk0x0082d14c;

// Loads a 13-byte car colour record into the globals the stage sky uses: three
// 16.16 vectors scaled by 10/127 and 1/127 and two 16.16 scalars.
// FUNCTION: CMR2 0x00507710
void OptionPreview_LoadSkyColourRecord(BYTE *pColour)
{
    g_unk0x0082d120.x = (int)(signed char)pColour[9] << 16;
    g_unk0x0082d120.y = (int)(signed char)pColour[10] << 16;
    g_unk0x0082d120.z = (int)(signed char)pColour[0xb] << 16;
    FixVecScale(&g_unk0x0082d120, &g_unk0x0082d120, FixMul(0xa0000, FixDiv(0x10000, 0x7f0000)));
    g_unk0x0082d12c.x = (int)(signed char)pColour[3] << 16;
    g_unk0x0082d12c.y = (int)(signed char)pColour[4] << 16;
    g_unk0x0082d12c.z = (int)(signed char)pColour[5] << 16;
    // The original expands the 1/127 scale as a 64-bit division (its compiler
    // keeps the constant divisor in a register, like at 0x4689c8).
    FixVecScaleRecip(&g_unk0x0082d12c, &g_unk0x0082d12c, 0x7f0000);
    g_unk0x0082d138.x = (int)(signed char)pColour[6] << 16;
    g_unk0x0082d138.y = (int)(signed char)pColour[7] << 16;
    g_unk0x0082d138.z = (int)(signed char)pColour[8] << 16;
    FixVecScaleRecip(&g_unk0x0082d138, &g_unk0x0082d138, 0x7f0000);
    g_unk0x0082d148 = (int)pColour[0] << 16;
    g_unk0x0082d148 = FixDiv(g_unk0x0082d148, 0xff0000);
    g_unk0x0082d14c = pColour[1];
    if (pColour[1] == 1) {
        g_unk0x0082d144 = (int)pColour[2] << 16;
        g_unk0x0082d144 = FixMul(g_unk0x0082d144, FixMul(0xa0000, FixDiv(0x10000, 0xff0000)));
    }
}

// Number of credit entries (pairs of quoted strings) in the credits file
// GLOBAL: CMR2 0x0081a728
int g_unk0x0081a728;
// GLOBAL: CMR2 0x0081a72c
void *g_unk0x0081a72c;
// GLOBAL: CMR2 0x0081a730
void *g_unk0x0081a730;
// GLOBAL: CMR2 0x0081a734
void *g_unk0x0081a734;

// Credits file names per region (index = CGameInfo::GetGameLanguage())
// GLOBAL: CMR2 0x00525aa4
char g_strCreditsPathFormat[12] = "%s%s%s.txt";
// GLOBAL: CMR2 0x00525ab0
char g_strCreditsPolish[16] = "credits_polish";
// GLOBAL: CMR2 0x00525ac0
char g_strCreditsJapanese[20] = "credits_japanese";
// GLOBAL: CMR2 0x00525ad4
char g_strCreditsSpanishUsa[20] = "credits_spanishusa";
// GLOBAL: CMR2 0x00525ae8
char g_strCreditsFrenchUsa[20] = "credits_frenchusa";
// GLOBAL: CMR2 0x00525afc
char g_strCreditsUsa[12] = "credits_usa";
// GLOBAL: CMR2 0x00525b08
char g_strCreditsGerman[16] = "credits_german";
// GLOBAL: CMR2 0x00525b18
char g_strCreditsItalian[16] = "credits_italian";
// GLOBAL: CMR2 0x00525b28
char g_strCreditsSpanish[16] = "credits_spanish";
// GLOBAL: CMR2 0x00525b38
char g_strCreditsFrench[16] = "credits_french";
// GLOBAL: CMR2 0x00525b48
char g_strCreditsEnglish[16] = "credits_english";

bool FrontendCredits_ReleaseText(void);

// Loads the credits text of the current language and splits it into
// entries of two quoted strings ("role", "name"); each quote-delimited
// string is terminated in place. Optionally registers the release callback.
// FUNCTION: CMR2 0x004f4910
void FrontendCredits_LoadText(char registerRelease)
{
    char *europe[5];
    char *usa[3];
    char *japan[1];
    char *poland[1];
    char **names;
    char line[256];
    char *p;
    int size;
    int i, n;

    europe[0] = g_strCreditsEnglish;
    europe[1] = g_strCreditsFrench;
    europe[2] = g_strCreditsSpanish;
    europe[3] = g_strCreditsItalian;
    europe[4] = g_strCreditsGerman;
    usa[0] = g_strCreditsUsa;
    usa[1] = g_strCreditsFrenchUsa;
    usa[2] = g_strCreditsSpanishUsa;
    japan[0] = g_strCreditsJapanese;
    poland[0] = g_strCreditsPolish;

    names = NULL;
    switch (CGameInfo::GetGameRegion()) {
    case 0:
        names = europe;
        break;
    case 1:
        names = usa;
        break;
    case 2:
        names = japan;
        break;
    case 3:
        names = poland;
        break;
    }

    sprintf(CFrontend::m_stringDest, g_strCreditsPathFormat, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(), names[CGameInfo::GetGameLanguage()]);
    g_unk0x0081a730 = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, TRUE);

    size = CGenericFileLoader::GetGenericFileSize();
    for (i = 0; i < size; i++) {
        if (((char *)g_unk0x0081a730)[i] == '"')
            g_unk0x0081a728++;
    }
    g_unk0x0081a728 = g_unk0x0081a728 / 4;

    g_unk0x0081a734 = CFileBuffer::AllocateLockedBuffer(g_unk0x0081a728 * 8);
    g_unk0x0081a72c = CFileBuffer::AllocateLockedBuffer(g_unk0x0081a728 * 4);
    if (g_unk0x0081a734 != NULL) {
        p = (char *)g_unk0x0081a730;
        for (i = 0; i < g_unk0x0081a728; i++) {
            p = strchr(p, '"') + 1;
            ((char **)g_unk0x0081a734)[i * 2] = p;
            for (n = 0; *p != '"'; n++, p++)
                line[n] = *p;
            *p = '\0';
            line[n] = '\0';

            p = strchr(p + 1, '"') + 1;
            ((char **)g_unk0x0081a734)[i * 2 + 1] = p;
            for (n = 0; *p != '"'; n++, p++)
                line[n] = *p;
            *p = '\0';
            p++;
        }
    } else {
        g_unk0x0081a728 = 0;
    }

    if (registerRelease)
        CGame::RegisterCallback(FrontendCredits_ReleaseText, NULL);
}

// FUNCTION: CMR2 0x004f4b10
void *FrontendCredits_GetNameTable(void)
{
    return g_unk0x0081a734;
}

// FUNCTION: CMR2 0x004f4b20
int FrontendCredits_GetEntryCount(void)
{
    return g_unk0x0081a728;
}

// FUNCTION: CMR2 0x004f4b30
void *FrontendCredits_GetRoleTable(void)
{
    return g_unk0x0081a72c;
}

// Frontend text files per region (index = language, see CGameInfo::InitRegionLanguageCount)
// GLOBAL: CMR2 0x00525b58
char g_strTextPolish[8] = "fpolish";
// GLOBAL: CMR2 0x00525b60
char g_strTextEngUsa[8] = "fengusa";
// GLOBAL: CMR2 0x00525b68
char g_strTextGerman[8] = "fgerman";
// GLOBAL: CMR2 0x00525b70
char g_strTextItalian[12] = "fitalian";
// GLOBAL: CMR2 0x00525b7c
char g_strTextSpanish[12] = "fspanish";
// GLOBAL: CMR2 0x00525b88
char g_strTextFrench[8] = "ffrench";
// GLOBAL: CMR2 0x00525b90
char g_strTextEnglish[12] = "fenglish";

// Frontend fonts (Fonts\<name>.tga + .pcf); FrontendText_ReloadFonts walks them as one array
// GLOBAL: CMR2 0x00525b9c
char g_frontendFontNames[4][20] = { "general\\hel_12pt", "general\\hel_15pt", "general\\hel_36pt", "general\\dot" };
#define g_strFontHel12 g_frontendFontNames[0]
#define g_strFontHel15 g_frontendFontNames[1]
#define g_strFontHel36 g_frontendFontNames[2]
#define g_strFontDot g_frontendFontNames[3]

// Text buffers of the frontend languages and whether each one lives inside
// its archive (then it is not freed on release)
// GLOBAL: CMR2 0x0081a738
void *g_languageTexts[5];
// GLOBAL: CMR2 0x0081a74c
BYTE g_languageTextInArchive[5];

extern char g_strTxtFormat[];

// Release callback of FrontendText_LoadRegionLanguages.
// FUNCTION: CMR2 0x004f4cc0
BYTE FrontendText_ReleaseRegionLanguages(void)
{
    BYTE i;

    for (i = 0; i < CGameInfo::m_unk0x0081a754; i++) {
        if (g_languageTextInArchive[i] == 0) {
            CFileBuffer::FreeGenericFileBuffer(g_languageTexts[i]);
            g_languageTexts[i] = NULL;
        }
        g_languageTexts[i] = NULL;
    }
    CFrontend::FreeLocalizedTextStrings();
    return 1;
}

// Loads the text file of every frontend language of this region from its
// language archive and hands them to the frontend text tables.
// FUNCTION: CMR2 0x004f4b90
BYTE FrontendText_LoadRegionLanguages(void)
{
    char *japan[1];
    char *poland[1];
    char **names;
    char *usa[3];
    char *europe[5];
    BYTE i;

    europe[0] = g_strTextEnglish;
    europe[1] = g_strTextFrench;
    europe[2] = g_strTextSpanish;
    europe[3] = g_strTextItalian;
    europe[4] = g_strTextGerman;
    usa[0] = g_strTextEngUsa;
    usa[1] = g_strTextFrench;
    usa[2] = g_strTextSpanish;
    japan[0] = g_strTextEnglish;
    poland[0] = g_strTextPolish;

    switch (CGameInfo::GetGameRegion()) {
    case 0:
        names = europe;
        break;
    case 1:
        names = usa;
        break;
    case 2:
        names = japan;
        break;
    case 3:
        names = poland;
        break;
    }

    for (i = 0; i < CGameInfo::m_unk0x0081a754; i++) {
        g_languageTexts[i] = NULL;
        sprintf(CFrontend::m_stringDest, g_strTxtFormat, names[i]);
        g_languageTexts[i] = CGenericFileLoader::FindFile(CFrontend::GetLocalizedFrontendArchive(i), CFrontend::m_stringDest,
                                                        &g_languageTextInArchive[i], NULL, 0);
    }
    if (g_languageTexts[0] != NULL) {
        CFrontend::BuildLocalizedTextStringTable(CGameInfo::m_unk0x0081a754, 0x215, (BYTE **)g_languageTexts);
        CGame::RegisterCallback(FrontendText_ReleaseRegionLanguages, NULL);
        return 1;
    }
    return 0;
}

// Loads the four frontend fonts from the frontend archive.
// FUNCTION: CMR2 0x004f4d20
BYTE FrontendText_LoadFonts(void)
{
    if (Font_InitTable(4) != -1) {
        Font_Load(g_strFontHel12, CGenericFileLoader::GetGenericFile(), 0);
        Font_Load(g_strFontHel15, CGenericFileLoader::GetGenericFile(), 1);
        Font_Load(g_strFontHel36, CGenericFileLoader::GetGenericFile(), 2);
        Font_Load(g_strFontDot, CGenericFileLoader::GetGenericFile(), 3);
        return 1;
    }
    return 0;
}

// Reloads the four frontend fonts (after the display was re-created).
// FUNCTION: CMR2 0x004f4d80
void FrontendText_ReloadFonts(void)
{
    char *pName;
    int i;

    i = 0;
    pName = g_frontendFontNames[0];
    do {
        Font_Reload(pName, CGenericFileLoader::GetGenericFile(), i);
        pName += 20;
        i++;
    } while ((int)pName < (int)g_frontendFontNames[4]);
}

// FUNCTION: CMR2 0x004f48b0
bool FrontendCredits_ReleaseText(void)
{
    if (g_unk0x0081a730 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081a730);
        g_unk0x0081a730 = NULL;
    }
    if (g_unk0x0081a734 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081a734);
        g_unk0x0081a734 = NULL;
    }
    if (g_unk0x0081a72c != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081a72c);
    g_unk0x0081a730 = NULL;
    g_unk0x0081a734 = NULL;
    g_unk0x0081a72c = NULL;
    g_unk0x0081a728 = 0;
    return true;
}

// GLOBAL: CMR2 0x00819128
int g_unk0x00819128;

// FUNCTION: CMR2 0x004f2360
void FrontendMenu_EnterDisplayMode(BYTE *p, int param2)
{
    g_unk0x00819128 = CGraphics::GetSelectedDisplayDeviceIx();
    if (g_unk0x00819128 + 10 > (int)CGraphics::GetDisplayCount()) {
        g_unk0x00819128 = (int)CGraphics::GetDisplayCount() - 10;
        if (g_unk0x00819128 < 0)
            g_unk0x00819128 = 0;
    }
    p[0x1e] = (BYTE)CGraphics::GetDisplayCount();
    p[0x1f] = (BYTE)CGraphics::GetSelectedDisplayDeviceIx();
    CGame::SetGameInputFocusState(0);
}

// FUNCTION: CMR2 0x004f2b00
void FrontendMenu_UpdateSoundOptions(BYTE *p)
{
    CGameInfo::SetMasterSoundVolume(p[0x1f] * 10);
    CGameInfo::SetEffectsSoundVolume(p[0x33] * 10);
    CGameInfo::SetCoDriverSoundVolume(p[0x47] * 10);
    CInput::SetInputRepeatTimingState((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100 / 4);
    CSound::NoOpSoundDeviceCallback();
}

// GLOBAL: CMR2 0x0081b14c
void *g_unk0x0081b14c;
// GLOBAL: CMR2 0x0081b150
void **g_unk0x0081b150;
// GLOBAL: CMR2 0x0081b154
int g_unk0x0081b154;

// FUNCTION: CMR2 0x004f4db0
int SavedGames_GetCount(void)
{
    return g_unk0x0081b154;
}

// Saved games list: records of 0x7f4 bytes
// FUNCTION: CMR2 0x004f4dc0
BYTE *SavedGames_GetRecordData(int index)
{
    return (BYTE *)g_unk0x0081b14c + 0x10 + index * 0x7f4;
}

// FUNCTION: CMR2 0x004f4de0
unsigned int SavedGames_GetDifficulty(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x30 + index * 0x7f4) >> 10 & 0xf;
}

// FUNCTION: CMR2 0x004f4e00
unsigned int SavedGames_GetDateLowBits(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x7d8 + index * 0x7f4) & 0x1f;
}

// FUNCTION: CMR2 0x004f4e20
unsigned int SavedGames_GetDateMiddleBits(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x7d8 + index * 0x7f4) >> 5 & 0x1f;
}

// FUNCTION: CMR2 0x004f4e50
BYTE *SavedGames_GetRecord(int index)
{
    return (BYTE *)g_unk0x0081b14c + index * 0x7f4;
}

// FUNCTION: CMR2 0x004f4e70
void *SavedGames_GetFileName(int index)
{
    return g_unk0x0081b150[index];
}

// FUNCTION: CMR2 0x004f4e80
bool SavedGames_ReleaseRecords(void)
{
    int i;

    for (i = 0; i < g_unk0x0081b154; i++) {
        if (g_unk0x0081b150[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x0081b150[i]);
            g_unk0x0081b150[i] = NULL;
        }
    }
    if (g_unk0x0081b14c != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081b14c);
        g_unk0x0081b14c = NULL;
    }
    if (g_unk0x0081b150 != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081b150);
    g_unk0x0081b14c = NULL;
    g_unk0x0081b150 = NULL;
    g_unk0x0081b154 = 0;
    return true;
}

// GLOBAL: CMR2 0x00525bec
char g_strSaveGamePattern[8] = "*.rcs";
// GLOBAL: CMR2 0x00525bf4
char g_strSaveGameDirFormat[16] = "%s\\gamesave\\";
// GLOBAL: CMR2 0x00525c04
char g_strSaveGameFileFormat[28] = "%s\\gamesave\\game%.5d.rcs";


// Formats into the buffer the first save game file name that does not exist
// yet (<install>\gamesave\gameNNNNN.rcs), starting from index 0.
// FUNCTION: CMR2 0x004f5150
void SavedGames_MakeUnusedFileName(char *pName)
{
    bool exists = true;
    int i;

    for (i = 0; exists; i++) {
        sprintf(pName, g_strSaveGameFileFormat, CInstallInfo::GetGameHDPath(), i);
        // PORT: _access through Sys.
        if (!Sys_FileExists(pName))
            exists = false;
    }
}

// Copies the string into CFrontend::m_stringDest and writes it back into the
// buffer in groups of four characters, one space between groups (the last
// character is dropped).
// FUNCTION: CMR2 0x004f8a90
void FrontendText_FormatCodeGroups(char *pText)
{
    int out = 0;
    strcpy(CFrontend::m_stringDest, pText);
    if ((int)strlen(CFrontend::m_stringDest) > 0) {
        int i = 1;
        char *source = CFrontend::m_stringDest - 1;
        do {
            pText[out++] = source[i];
            if (i % 4 == 0)
                pText[out++] = ' ';
            i++;
        } while (i - 1 < (int)strlen(CFrontend::m_stringDest));
    }
    pText[out] = 0;
}

// Reads every saved game (<install>\gamesave\*.rcs) into the saved games
// list: one 0x7f4-byte record per file plus its file name.
// FUNCTION: CMR2 0x004f4ef0
// PORT: Sys_ListFiles instead of SetCurrentDirectory + FindFirstFile; each
// save is read by its full game path, and the list keeps the file name.
static int SavedGames_AddListedFile(const char *name, void *pDir)
{
    char path[520];
    void *pRecord;

    sprintf(path, "%s%s", (const char *)pDir, name);
    pRecord = CFileBuffer::GetGenericFileBuffer(path, TRUE);
    if (pRecord != NULL) {
        g_unk0x0081b14c = CFileBuffer::ReallocateLockedBuffer(g_unk0x0081b14c, (g_unk0x0081b154 + 1) * 0x7f4);
        memcpy((BYTE *)g_unk0x0081b14c + g_unk0x0081b154 * 0x7f4, pRecord, 0x7f4);
        g_unk0x0081b150 = (void **)CFileBuffer::ReallocateLockedBuffer(g_unk0x0081b150, g_unk0x0081b154 * 4 + 4);
        g_unk0x0081b150[g_unk0x0081b154] = CFileBuffer::AllocateLockedBuffer(0x100);
        strcpy((char *)g_unk0x0081b150[g_unk0x0081b154], name);
        g_unk0x0081b154++;
        CFileBuffer::FreeGenericFileBuffer(pRecord);
    }
    return 1;
}

void SavedGames_LoadRecords(void)
{
    char saveDir[260];

    SavedGames_ReleaseRecords();
    g_unk0x0081b154 = 0;
    sprintf(saveDir, g_strSaveGameDirFormat, CInstallInfo::GetGameHDPath());
    Sys_ListFiles(saveDir, g_strSaveGamePattern, SavedGames_AddListedFile, saveDir);
}

// GLOBAL: CMR2 0x00818ac8
int g_unk0x00818ac8;

// FUNCTION: CMR2 0x004ea480
void Frontend_SetOverlayMode(int param1)
{
    // Any other value (and every refused case) clears the mode.
    if (param1 == 1) {
        if (!CGameInfo::IsConfiguredMultiplayer() && CGameInfo::GetConfiguredGameMode() != 4) {
            g_unk0x00818ac8 = 1;
            return;
        }
    } else if (param1 == 2) {
        if (!CGameInfo::IsConfiguredMultiplayer() && CGameInfo::GetConfiguredGameMode() != 4) {
            g_unk0x00818ac8 = 2;
            return;
        }
    } else if (param1 == 3) {
        if (!CGameInfo::IsConfiguredMultiplayer()) {
            if (CGameInfo::GetConfiguredGameMode() != 4) {
                g_unk0x00818ac8 = 3;
                return;
            }
            g_unk0x00818ac8 = 3;
        }
    }
    g_unk0x00818ac8 = 0;
}

// FUNCTION: CMR2 0x004ea500
int Frontend_GetOverlayMode(void)
{
    return g_unk0x00818ac8;
}

// FUNCTION: CMR2 0x004ea8e0
void GameInfo_SetConfiguredGameMode(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 0x7f) << 3) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffffc07);
    if (param1 >= 8)
        CGameInfo::m_gameInfo.field_0x3980 = CGameInfo::m_gameInfo.field_0x14 >> 3 & 0x7f;
}

// FUNCTION: CMR2 0x004ea970
void GameInfo_SetGameModeOptionBits20To22(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7) << 20) | (CGameInfo::m_gameInfo.field_0x14 & 0xff8fffff);
}

// FUNCTION: CMR2 0x004ea990
void GameInfo_GetPlayerOptionNibbles(int *pOut1, int *pOut2)
{
    *pOut1 = CGameInfo::m_gameInfo.field_0x14 >> 23 & 0xf;
    *pOut2 = CGameInfo::m_gameInfo.field_0x14 >> 27 & 0xf;
}

// FUNCTION: CMR2 0x004ea8a0
void GameInfo_SetGameModeBits0To2(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7)) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffffff8U);
}

// FUNCTION: CMR2 0x004ea8c0
void GameInfo_SetConfiguredPlayerCount(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 0xf) << 13) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffe1fffU);
}

// FUNCTION: CMR2 0x004ea920
BYTE GameInfo_GetLastNetworkGameMode(void)
{
    return CGameInfo::m_gameInfo.field_0x3980;
}

// FUNCTION: CMR2 0x004ea930
void GameInfo_SetConfiguredDifficulty(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7) << 10) | (CGameInfo::m_gameInfo.field_0x14 & 0xffffe3ffU);
}

// FUNCTION: CMR2 0x004ea950
void GameInfo_SetConfiguredMultiplayer(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 1) << 17) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffdffffU);
}

// FUNCTION: CMR2 0x004ea9c0
void GameInfo_SetPlayerOptionNibbles(unsigned int param1, unsigned int param2)
{
    CGameInfo::m_gameInfo.field_0x14 = (((param2 & 0xf) << 4 | param1 & 0xf) << 23) | (CGameInfo::m_gameInfo.field_0x14 & 0x807fffffU);
}

// GLOBAL: CMR2 0x0052af90
DWORD g_unk0x0052af90;

// FUNCTION: CMR2 0x004ea9f0
void GameInfo_ResetSessionTimestamp(void)
{
    g_unk0x0052af90 = Sys_GetTicks();
}

// FUNCTION: CMR2 0x004eaa00
DWORD GameInfo_GetSessionTimestamp(void)
{
    return g_unk0x0052af90;
}

// FUNCTION: CMR2 0x004eaa10
void GameInfo_SetSoundOptionBit22(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x18 = ((param1 & 1) << 22) | (CGameInfo::m_gameInfo.field_0x18 & 0xffbfffffU);
}

// FUNCTION: CMR2 0x004eaa30
char GameInfo_AreOptionsAvailable(void)
{
    return 1;
}

// FUNCTION: CMR2 0x004eaa40
void GameInfo_SetNetworkOptionBits1To2(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 3) << 1) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffffff9U);
}

// FUNCTION: CMR2 0x004eaa60
void GameInfo_SetNetworkOptionBits8To10(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 7) << 8) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffff8ffU);
}

// FUNCTION: CMR2 0x004eaa80
unsigned int GameInfo_GetNetworkOptionBits8To10(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 8 & 7;
}

// FUNCTION: CMR2 0x004eaa90
void GameInfo_SetNetworkOptionBits3To7(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0x1f) << 3) | (CGameInfo::m_gameInfo.field_0x1c & 0xffffff07U);
}

// FUNCTION: CMR2 0x004eaab0
unsigned int GameInfo_GetNetworkOptionBits3To7(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 3 & 0x1f;
}

// Number of bits set.
// FUNCTION: CMR2 0x004eaac0
int GameInfo_CountSetBits(unsigned int value)
{
    int count = 0;
    int i = 32;

    do {
        if (value & 1)
            count++;
        value >>= 1;
    } while (--i != 0);
    return count;
}



// Reads back the current camera defaults: fills a 3 dword vector out of
// 0x52b028/0x52b02c, a short out of 0x52b034 and an int out of 0x52b030.
// FUNCTION: CMR2 0x004eaae0
void GameInfo_GetDefaultCameraParameters(int *param_1, short *param_2, int *param_3)
{
    param_1[0] = 0;
    param_1[1] = CGameInfo::m_gameInfo.field_0x88;
    param_1[2] = CGameInfo::m_gameInfo.field_0x8c;
    *param_2 = CGameInfo::m_gameInfo.field_0x94;
    *param_3 = CGameInfo::m_gameInfo.field_0x90;
}

// FUNCTION: CMR2 0x004eab20
void GameInfo_SetSoundOptionBit30(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x18 = ((param1 & 1) << 30) | (CGameInfo::m_gameInfo.field_0x18 & 0xbfffffffU);
}

// Turns cheat `bit` on or off in the game options.
// FUNCTION: CMR2 0x004eab40
void GameInfo_SetConfiguredCheat(int bit, int value)
{
    BYTE mask = 1 << bit;

    if (value != 0) {
        *(unsigned int *)((BYTE *)&CGameInfo::m_gameInfo + 0x20) |= (unsigned int)mask << 8;
        return;
    }
    *(unsigned int *)((BYTE *)&CGameInfo::m_gameInfo + 0x20) &= (unsigned int)(BYTE)~mask << 8 | 0xffff00ff;
}

// FUNCTION: CMR2 0x004eab80
void GameInfo_SaveConfiguredCheats(void)
{
    CGameInfo::m_gameInfo.field_0x20 = ((CGameInfo::m_gameInfo.field_0x20 & 0xff00) << 16) | (CGameInfo::m_gameInfo.field_0x20 & 0xffffff);
}

// FUNCTION: CMR2 0x004eaba0
void GameInfo_RestoreConfiguredCheats(void)
{
    CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 >> 24) << 8 | (CGameInfo::m_gameInfo.field_0x20 & 0xffff00ffU);
}

// FUNCTION: CMR2 0x004eac80
void GameInfo_SetNetworkOptionBit11(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 1) << 11) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffff7ffU);
}

// FUNCTION: CMR2 0x004eaca0
int GameInfo_GetField98(void)
{
    return *(int *)&CGameInfo::m_gameInfo.field_0x98;
}

// FUNCTION: CMR2 0x004eacb0
void GameInfo_SetNetworkOptionBits12To15(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0xf) << 12) | (CGameInfo::m_gameInfo.field_0x1c & 0xffff0fffU);
}

// FUNCTION: CMR2 0x004eacd0
void GameInfo_SetNetworkOptionBits16To19(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0xf) << 16) | (CGameInfo::m_gameInfo.field_0x1c & 0xfff0ffffU);
}

// FUNCTION: CMR2 0x004eacf0
unsigned int GameInfo_GetNetworkOptionBits12To15(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 12 & 0xf;
}

// FUNCTION: CMR2 0x004ead00
unsigned int GameInfo_GetNetworkOptionBits16To19(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 16 & 0xf;
}

void RallyData_SetSelectionBits12To13(BYTE param1);
void RallyData_SetStageSelectionAndRefreshFlags(BYTE param1);
void RallyData_SetCountrySelectionBits(BYTE param1);

// Pushes the frontend option settings to the modules that use them.
// FUNCTION: CMR2 0x004ead10
void GameInfo_RestoreFrontendOptionSettings(void)
{
    GameInfo_SetConfiguredGameMode((BYTE)CGameInfo::m_unk0x0052af94);
    GameInfo_SetConfiguredPlayerCount((BYTE)CGameInfo::m_unk0x0052ea44);
    GameInfo_SetConfiguredDifficulty((BYTE)CGameInfo::m_unk0x0052af9c);
    RallyData_SetSecondarySelectionNibble((BYTE)CGameInfo::m_unk0x0052af80);
    RallyData_SetSelectionBits10To11((BYTE)CGameInfo::m_unk0x0052af84);
    RallyData_SetSelectionBits12To13((BYTE)CGameInfo::m_unk0x0052af8c);
    RallyData_SetSelectionBits16To19((BYTE)CGameInfo::m_unk0x0052e93c);
    RallyData_SetStageSelectionAndRefreshFlags((BYTE)CGameInfo::m_unk0x0052ea48);
    RallyData_SetCountrySelectionBits((BYTE)CGameInfo::m_unk0x0052e940);
    CGameInfo::m_gameInfo.field_0x14 = ((CGameInfo::m_unk0x0052af88 & 0xf) << 4 | CGameInfo::m_unk0x0052af98 & 0xf) << 0x17
        | CGameInfo::m_gameInfo.field_0x14 & 0x807fffff;
}

// FUNCTION: CMR2 0x004eabc0
void GameInfo_ApplyCheatsForGameMode(void)
{
    if (CGameInfo::GetGameInfoSessionFlag() || CGameInfo::GetConfiguredGameMode() == 0 ||
        CGameInfo::GetConfiguredGameMode() == 4) {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0x400) << 8);
    } else if (CGameInfo::GetConfiguredGameMode() == 5 || CGameInfo::GetConfiguredGameMode() == 6) {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0xd00) << 8);
    } else {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0xff00) << 8);
    }
}

// GLOBAL: CMR2 0x00511300
double g_unk0x00511300 = 4096.0 / (360.0 * 65536.0);   // 16.16 degrees -> sine table index
// One RGBA colour: OptionMenu_GetStateByteAddress returns all four bytes, which callers
// read as a dword. Independent globals do not preserve this layout.
// GLOBAL: CMR2 0x0082b1b8
BYTE g_unk0x0082b1b8[4];
#define g_unk0x0082b1b9 g_unk0x0082b1b8[1]
#define g_unk0x0082b1ba g_unk0x0082b1b8[2]
#define g_unk0x0082b1bb g_unk0x0082b1b8[3]


// GLOBAL: CMR2 0x00511cd8
int g_unk0x00511cd8[4] = {
    0x1bff2d87, 0x11d24cb7, 0x60004cb1, 0xe0014e08,
};

typedef HRESULT (__stdcall *DPMethod5GI)(void *pThis, DWORD a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5);

// Adds a session found by DirectPlay to the list (at most 20, ignoring the
// blank-named ones): a copy of its description with its own name buffer.
// FUNCTION: CMR2 0x004a0ca0
void Session_AddToList(DPSESSIONDESC2 *pDesc)
{
    unsigned int n;

    if (CGameInfo::m_unk0x005a01bc >= 20)
        return;
    if (strcmp(CMain::m_logFileBlankLine, pDesc->lpszSessionNameA) == 0)
        return;
    n = CGameInfo::m_unk0x005a01bc;
    ((DPSESSIONDESC2 *)CGameInfo::m_unk0x0059fa20)[n] = *pDesc;
    strcpy(CGameInfo::m_sessionNames[n], pDesc->lpszSessionNameA);
    ((DPSESSIONDESC2 *)CGameInfo::m_unk0x0059fa20)[n].lpszSessionNameA = CGameInfo::m_sessionNames[n];
    CGameInfo::m_unk0x005a01bc++;
}

// DirectPlay EnumSessions callback: stops on time-out, otherwise lists the session.
// FUNCTION: CMR2 0x004a12b0
BOOL FAR PASCAL Session_EnumCallback(LPCDPSESSIONDESC2 pDesc, LPDWORD pTimeOut, DWORD flags, LPVOID pContext)
{
    if (flags & DPESC_TIMEDOUT)
        return FALSE;
    Session_AddToList((DPSESSIONDESC2 *)pDesc);
    return TRUE;
}


// Fills the session descriptor at 0x5a0068 and enumerates the DirectPlay
// sessions. Returns 1 on DP_OK, -1 and -2 for two DirectPlay errors (the
// caller reports them), 0 otherwise.
// FUNCTION: CMR2 0x004a13b0
int CGameInfo::EnumerateNetworkSessions(void)
{
    IDirectPlay4A *pDP;

    if (m_unk0x005a1814 != 0)
        return 0;
    ResetNetworkSessionState();
    memset(g_unk0x005a0068, 0, 0x50);
    *(int *)(g_unk0x005a0068 + 0x18) = g_unk0x00511cd8[0];
    *(int *)(g_unk0x005a0068 + 0x30) = (int)&m_unk0x005a00b8;
    *(int *)(g_unk0x005a0068) = 0x50;
    *(int *)(g_unk0x005a0068 + 0x1c) = g_unk0x00511cd8[1];
    *(int *)(g_unk0x005a0068 + 0x20) = g_unk0x00511cd8[2];
    *(int *)(g_unk0x005a0068 + 0x24) = g_unk0x00511cd8[3];
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    switch (((DPMethod5GI)(*(void ***)pDP)[0x34 / 4])(pDP, (DWORD)g_unk0x005a0068, 0,
                                                       (DWORD)Session_EnumCallback, 0, 0x20)) {
    case DP_OK:
        return 1;
    case 0x8877015e:
        return -1;
    case 0x88770118:
        return -2;
    case 0x8877005a:
        return 0;
    case 0x88770082:
        return 0;
    case 0x887700aa:
        return 0;
    case 0x88770140:
        return 0;
    }
    return 0;
}

// Same as EnumerateNetworkSessions, but also stores param1 in the descriptor (0x5a009c)
// and enumerates with flags 0x51.
// FUNCTION: CMR2 0x004a12d0
int CGameInfo::EnumerateNetworkSessionsWithUserData(int param1)
{
    IDirectPlay4A *pDP;

    if (m_unk0x005a1814 != 0)
        return 0;
    ResetNetworkSessionState();
    memset(g_unk0x005a0068, 0, 0x50);
    *(int *)(g_unk0x005a0068 + 0x34) = param1;
    *(int *)(g_unk0x005a0068 + 0x18) = g_unk0x00511cd8[0];
    *(int *)(g_unk0x005a0068 + 0x30) = (int)&m_unk0x005a00b8;
    *(int *)(g_unk0x005a0068) = 0x50;
    *(int *)(g_unk0x005a0068 + 0x1c) = g_unk0x00511cd8[1];
    *(int *)(g_unk0x005a0068 + 0x20) = g_unk0x00511cd8[2];
    *(int *)(g_unk0x005a0068 + 0x24) = g_unk0x00511cd8[3];
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    switch (((DPMethod5GI)(*(void ***)pDP)[0x34 / 4])(pDP, (DWORD)g_unk0x005a0068, 0,
                                                       (DWORD)Session_EnumCallback, 0, 0x51)) {
    case DP_OK:
        return 1;
    case 0x8877015e:
        return -1;
    case 0x88770118:
        return -2;
    case 0x8877005a:
    case 0x88770082:
    case 0x887700aa:
    case 0x88770140:
        return 0;
    }
    return 0;
}



// Cambia el modo activo 0x82ca1c (intercambiando 0x3c con el modo anterior) y
// reinicia el temporizador.
// FUNCTION: CMR2 0x00505a60
void CGameInfo::SwitchOptionPreviewMode(int param1)
{
    int current;
    int *pNew;
    int *pOld;
    int tmp;

    current = (signed char)g_unk0x0082ca1c;
    if (current == param1)
        return;
    if (g_unk0x0082ca1c != 0xff && param1 != -1) {
        pOld = (int *)((char *)g_unk0x0082c6c8 + current * 0x50);
        pNew = (int *)((char *)g_unk0x0082c6c8 + param1 * 0x50);
        tmp = *(int *)((char *)pOld + 0x4c);
        *(int *)((char *)pOld + 0x4c) = *(int *)((char *)pNew + 0x4c);
        *(int *)((char *)pNew + 0x4c) = tmp;
        tmp = *(int *)((char *)pOld + 0x1c);
        *(int *)((char *)pOld + 0x1c) = *(int *)((char *)pNew + 0x1c);
        *(int *)((char *)pNew + 0x1c) = tmp;
        tmp = *(int *)((char *)pOld + 0x18);
        *(int *)((char *)pOld + 0x18) = *(int *)((char *)pNew + 0x18);
        *(int *)((char *)pNew + 0x18) = tmp;
        tmp = *(int *)((char *)pOld + 0x28);
        *(int *)((char *)pOld + 0x28) = *(int *)((char *)pNew + 0x28);
        *(int *)((char *)pNew + 0x28) = tmp;
        tmp = *(int *)((char *)pOld + 0x14);
        *(int *)((char *)pOld + 0x14) = *(int *)((char *)pNew + 0x14);
        *(int *)((char *)pNew + 0x14) = tmp;
        tmp = *(int *)((char *)pOld + 0x20);
        *(int *)((char *)pOld + 0x20) = *(int *)((char *)pNew + 0x20);
        *(int *)((char *)pNew + 0x20) = tmp;
        tmp = *(int *)((char *)pOld + 0x24);
        *(int *)((char *)pOld + 0x24) = *(int *)((char *)pNew + 0x24);
        *(int *)((char *)pNew + 0x24) = tmp;
        tmp = *(int *)((char *)pOld + 0x44);
        *(int *)((char *)pOld + 0x44) = *(int *)((char *)pNew + 0x44);
        *(int *)((char *)pNew + 0x44) = tmp;
        tmp = *(int *)((char *)pOld + 0x40);
        *(int *)((char *)pOld + 0x40) = *(int *)((char *)pNew + 0x40);
        *(int *)((char *)pNew + 0x40) = tmp;
        tmp = *(int *)((char *)pOld + 0x3c);
        *(int *)((char *)pOld + 0x3c) = *(int *)((char *)pNew + 0x3c);
        *(int *)((char *)pNew + 0x3c) = tmp;

    }
    g_unk0x0082c6c0 = (int)CMain::GetFrameDelta();
    g_unk0x0082cb44 = 0;
    g_unk0x0082ca1c = (BYTE)param1;
}


#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// FUNCTION: CMR2 0x005039d0
void FixInterp_StartToOne(FixInterp *p)
{
    if (p->current != 0x10000) {
        p->start = p->current;
        p->end = 0x10000;
        p->active = 1;
        p->distance = FIX_ABS(FixMul(p->end, 0x320000) - FixMul(p->start, 0x320000));
        p->startTime = CMain::GetFrameDelta();
    }
}

// FUNCTION: CMR2 0x00503aa0
void FixInterp_StartToZero(FixInterp *p)
{
    if (p->current != 0) {
        p->start = p->current;
        p->end = 0;
        p->active = 1;
        p->distance = FIX_ABS(FixMul(p->end, 0x320000) - FixMul(p->start, 0x320000));
        p->startTime = CMain::GetFrameDelta();
    }
}

// GLOBAL: CMR2 0x0082b668
BYTE g_unk0x0082b668[sizeof(Menu)];

// FUNCTION: CMR2 0x00502500
BYTE *OptionMenu_GetControlSetupMenu(void)
{
    return g_unk0x0082b668;
}

// GLOBAL: CMR2 0x0082b848
BYTE g_unk0x0082b848[sizeof(Menu)];

// FUNCTION: CMR2 0x00502510
BYTE *OptionMenu_GetValueSliderMenu(void)
{
    return g_unk0x0082b848;
}

BYTE *RallyData_GetDriverSkillRecord(int index);

// Copies the seven option bytes of every rally data record into rows 4..7 of
// the default table, mirrors them into rows 0..3 and clears the option flags.
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00502570
void OptionMenu_InitDefaultRecordTables(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        BYTE *p = RallyData_GetDriverSkillRecord(i);

        g_unk0x0082bee8[i + 4][4] = p[4];
        g_unk0x0082bee8[i + 4][5] = p[5];
        g_unk0x0082bee8[i + 4][1] = p[1];
        g_unk0x0082bee8[i + 4][6] = p[6];
        g_unk0x0082bee8[i + 4][3] = p[3];
        g_unk0x0082bee8[i + 4][2] = p[2];
        g_unk0x0082bee8[i + 4][0] = p[0];
        g_unk0x0082bee8[i][4] = g_unk0x0082bf04[i * 7 + 4];
        g_unk0x0082bee8[i][5] = g_unk0x0082bf04[i * 7 + 5];
        g_unk0x0082bee8[i][1] = g_unk0x0082bf04[i * 7 + 1];
        g_unk0x0082bee8[i][6] = g_unk0x0082bf04[i * 7 + 6];
        g_unk0x0082bee8[i][3] = g_unk0x0082bf04[i * 7 + 3];
        g_unk0x0082bee8[i][2] = g_unk0x0082bf04[i * 7 + 2];
        g_unk0x0082bee8[i][0] = g_unk0x0082bf04[i * 7 + 0];
        memset(&g_unk0x0082bf20[i][0], 0, 7);
    }
}

void OptionMenu_ResetRecords(void);

// Resets the mode menu cursor and the value byte of its first two items.
// FUNCTION: CMR2 0x00502520
void OptionMenu_ResetModeCursor(void)
{
    ((Menu *)g_unk0x0082b668)->cursor = 0;
    ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1)].max = 0;
    ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2)].max = 0;
    OptionMenu_ResetRecords();
}

// GLOBAL: CMR2 0x0052aa60
int g_unk0x0052aa60;
// GLOBAL: CMR2 0x0052aa68
int g_unk0x0052aa68;
// GLOBAL: CMR2 0x0052aa70
Menu g_menu0x0052aa70;
// GLOBAL: CMR2 0x0052af41
BYTE g_unk0x0052af41;
// GLOBAL: CMR2 0x0052af44
Menu *g_pMenu0x0052af44;
// GLOBAL: CMR2 0x0052af4c
int g_unk0x0052af4c;
// GLOBAL: CMR2 0x0052af50
int g_unk0x0052af50;
extern BYTE g_unk0x0052af58[2];

// In-race network menus (built by 0x402c40..0x404000).
// GLOBAL: CMR2 0x00529918
Menu g_menu0x00529918;
// GLOBAL: CMR2 0x00529af8
Menu g_menu0x00529af8;
// GLOBAL: CMR2 0x00529ce8
Menu g_menu0x00529ce8;
// GLOBAL: CMR2 0x00529ed8
Menu g_menu0x00529ed8;
// GLOBAL: CMR2 0x0052a0c0
Menu g_menu0x0052a0c0;
// GLOBAL: CMR2 0x0052a2a8
Menu g_menu0x0052a2a8;
// GLOBAL: CMR2 0x0052a490
Menu g_menu0x0052a490;
// GLOBAL: CMR2 0x0052a670
Menu g_menu0x0052a670;
// GLOBAL: CMR2 0x0052a870
Menu g_menu0x0052a870;
// GLOBAL: CMR2 0x0052ad54
int g_unk0x0052ad54;
// GLOBAL: CMR2 0x0052ad60
Menu g_menu0x0052ad60;
// GLOBAL: CMR2 0x0052af48
Menu *g_pMenu0x0052af48;
// GLOBAL: CMR2 0x0052af6c
int g_unk0x0052af6c;

void InRaceMenu_Close(void);
void Menu_QueueDefaultAction(void);
void Sound_ClearMusicPauseFlag(void);
unsigned char RallyData_GetSelectionFlag26(void);
unsigned char RallyData_GetSelectionFlag28(void);
int RallyData_IsHeadToHeadRaceMode(void);

// FUNCTION: CMR2 0x00401850
void InRaceMenu_ResumeWithSound(Menu *pMenu, int param)
{
    if ((BYTE)RallyData_GetSelectionFlag26())
        Sound_ClearMusicPauseFlag();
    InRaceMenu_Close();
}

// FUNCTION: CMR2 0x00402bb0
void InRaceMenu_RememberOptionCursor(Menu *pMenu, char param)
{
    if (param == 0) {
        pMenu->cursor = (char)Menu_FindItem(pMenu, 7);
        g_unk0x0052af6c = pMenu->cursor;
        return;
    }
    g_unk0x0052af6c = pMenu->cursor;
}

// FUNCTION: CMR2 0x00402bf0
void InRaceMenu_SkipBlankOptionRow(Menu *pMenu)
{
    if (pMenu->cursor == 2)
        pMenu->cursor = g_unk0x0052af6c < 2 ? 3 : 1;
    g_unk0x0052af6c = pMenu->cursor;
}

// FUNCTION: CMR2 0x00402c30
void InRaceMenu_ResumeDrawCallback(Menu *pMenu)
{
    InRaceMenu_Close();
}

// FUNCTION: CMR2 0x00402c40
void InRaceMenu_BuildResumePage(void)
{
    Menu_Init(&g_menu0x00529ce8, 0, 0, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00529ce8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x00529ce8, NULL, NULL, (MenuCallback)InRaceMenu_ResumeDrawCallback, NULL);
}

// Callback 0 of the network options menu: loads the two switches.
// FUNCTION: CMR2 0x00402eb0
void InRaceMenu_EnterNetworkOptions(Menu *pMenu, char param)
{
    if (param == 0) {
        if (CGameInfo::IsSplitBarEnabled() != 0)
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 1;
        else
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 0;
        if ((BYTE)CGameInfo::IsDashOptionEnabled()) {
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
            return;
        }
        pMenu->items[Menu_FindItem(pMenu, 4)].max = 0;
    }
}

// FUNCTION: CMR2 0x00402f70
void InRaceMenu_ReturnToPauseItem(Menu *pMenu, int param)
{
    Menu_SetNextAction((int)&g_menu0x0052ad60);
}

// FUNCTION: CMR2 0x00402f80
void InRaceMenu_ReturnToPauseUpdate(Menu *pMenu)
{
    Menu_SetNextAction((int)&g_menu0x0052ad60);
}

void InRaceMenu_ApplyNetworkOptions(Menu *pMenu, int param);
void InRaceMenu_DrawLabelAndValueRows(Menu *pMenu);
void InRaceMenu_DrawSliderRows(Menu *pMenu);
void InRaceMenu_DrawCarSetupChoices(Menu *pMenu);

// Builds the network options menu and disables the two-state entries when
// the current rally mode does not support them.
void InRaceMenu_ApplyNetworkOptions(Menu *pMenu, int param);
void InRaceMenu_DrawLabelAndValueRows(Menu *pMenu);
void InRaceMenu_DrawSliderRows(Menu *pMenu);
void InRaceMenu_DrawCarSetupChoices(Menu *pMenu);
// FUNCTION: CMR2 0x00402f90
void InRaceMenu_BuildNetworkOptions(void)
{
    int i;

    if ((char)RallyDataState() == 1)
        Menu_Init(&g_menu0x00529ed8, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    else
        Menu_Init(&g_menu0x00529ed8, 0, 0, 0, &g_menu0x00529918, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00529ed8, 0, 0x33, &g_menu0x0052aa70, 0, 0);
    Menu_AddItemType2(&g_menu0x00529ed8, 0, 0x5c, &g_menu0x0052a870, 0, 1);
    Menu_AddItemType3(&g_menu0x00529ed8, 0, 0x35, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x00529ed8, 0, 0x39, 2, 0, 0, 0, 0, 4);
    Menu_AddItemType2(&g_menu0x00529ed8, 0, 0x3b, &g_menu0x0052ad60,
                      (int)(MenuCallback)InRaceMenu_ApplyNetworkOptions, -1);
    Menu_SetCallbacks(&g_menu0x00529ed8, (MenuCallback)InRaceMenu_EnterNetworkOptions, NULL,
                      (MenuCallback)InRaceMenu_DrawLabelAndValueRows, NULL);
    Menu_ValidateCursor(&g_menu0x00529ed8, 0);
    if (RallyData_IsHeadToHeadRaceMode() == 0) {
        for (i = 0; i < g_menu0x00529ed8.itemCount; i++) {
            if (g_menu0x00529ed8.items[i].value == 2) {
                g_menu0x00529ed8.items[i].enabled = 0;
                g_menu0x00529ed8.items[i].visible = 0;
            }
        }
    }
}

// FUNCTION: CMR2 0x00403360
void InRaceMenu_SelectLastItem(Menu *pMenu, int param)
{
    pMenu->cursor = pMenu->itemCount - 1;
}

// FUNCTION: CMR2 0x004035d0
void InRaceMenu_SelectPlayer(Menu *pMenu, int param)
{
    g_unk0x0052af58[1] = pMenu->cursor;
}

void InRaceMenu_ResetCameraDefaults(Menu *pMenu, char unused);
void InRaceMenu_EnterCameraOffsetOptions(Menu *pMenu, char unused);
void InRaceMenu_LeaveCameraOffsetOptions(Menu *pMenu, char cancel);
void InRaceMenu_UpdateCameraOffsets(Menu *pMenu);

// Builds the four-slider camera options menu.
// FUNCTION: CMR2 0x004035e0
void InRaceMenu_BuildCameraOffsetOptions(void)
{
    Menu_Init(&g_menu0x0052a870, 0, 0, 0, &g_menu0x00529ed8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0052a870, 0, 0x5e, 0x15, 10, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0052a870, 0, 0x5f, 0x15, 10, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0052a870, 0, 0x60, 0x15, 10, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0052a870, 0, 0x61, 0x15, 10, 0, 0, 0, 3);
    Menu_AddItemType4(&g_menu0x0052a870, 0, 0x62, (int)InRaceMenu_ResetCameraDefaults, 4);
    Menu_AddItemType2(&g_menu0x0052a870, 0, 0x3b, &g_menu0x00529ed8, 0, 5);
    Menu_SetCallbacks(&g_menu0x0052a870, (MenuCallback)InRaceMenu_EnterCameraOffsetOptions,
                      (MenuCallback)InRaceMenu_UpdateCameraOffsets, (MenuCallback)InRaceMenu_DrawSliderRows,
                      (MenuCallback)InRaceMenu_LeaveCameraOffsetOptions);
    Menu_ValidateCursor(&g_menu0x0052a870, 0);
}

// GLOBAL: CMR2 0x0052a0b8
short g_unk0x0052a0b8;

unsigned short Input_GetControllerSlotMapping(unsigned short slot);

// Returns 1 when the slot's device pressed "back" (button 0x400, or the
// joystick button mapped to action 9, which is then remembered).
// FUNCTION: CMR2 0x00404e10
int InRaceMenu_IsBackPressed(unsigned short slot)
{
    DeviceInfo *pDev = CInput::GetAvailableDeviceRecord(Input_GetControllerSlotMapping(slot));

    if (pDev->field_0x0 == 1 || pDev->field_0x0 == 2) {
        if ((pDev->field_0x8 & 0x400) != 0)
            return 1;
    } else {
        if ((pDev->field_0x8 & (unsigned short)CInput::GetButtonMapping(slot, 9)) != 0) {
            g_unk0x0052a0b8 = CInput::GetButtonMapping(slot, 9);
            return 1;
        }
        if ((CInput::GetAvailableDeviceRecord(0)->field_0x8 & 0x400) != 0)
            return 1;
    }
    return 0;
}

// Camera options chosen in the camera menu (0x4035e0): offset, height, distance.
// GLOBAL: CMR2 0x0052a86c
short g_unk0x0052a86c;
// Camera offset chosen in the camera options menu.
// GLOBAL: CMR2 0x0052aa50
FixVector g_unk0x0052aa50;
#define g_unk0x0052aa54 (g_unk0x0052aa50.y)
#define g_unk0x0052aa58 (g_unk0x0052aa50.z)
// GLOBAL: CMR2 0x0052aa5c
int g_unk0x0052aa5c;

// Callback of the camera options menu: turns the four sliders into values.
// FUNCTION: CMR2 0x00403200
void InRaceMenu_ReadCameraOffsetSliders(Menu *pMenu)
{
    g_unk0x0052aa5c = (int)(pMenu->items[2].max << 19) / (pMenu->items[2].min - 1) + 0x80000;
    g_unk0x0052a86c = (short)((int)(pMenu->items[3].max * 0x11c) / (pMenu->items[3].min - 1));
    g_unk0x0052aa54 = (int)(pMenu->items[0].max * 0xcccd) / (pMenu->items[0].min - 1) + 0x13333;
    g_unk0x0052aa58 = (int)(pMenu->items[1].max * 0x50000) / (pMenu->items[1].min - 1) + 0x50000;
}

int Race_IsFlag24Clear(void);

// Callback of the sound options menu: applies the three volumes.
// FUNCTION: CMR2 0x00401380
void InRaceMenu_ApplySoundVolumes(Menu *pMenu)
{
    int rate;

    CGameInfo::SetMasterSoundVolume(pMenu->items[Menu_FindItem(pMenu, 0)].max * 10);
    CGameInfo::SetEffectsSoundVolume(pMenu->items[Menu_FindItem(pMenu, 1)].max * 10);
    if (Race_IsFlag24Clear())
        CGameInfo::SetCoDriverSoundVolume(pMenu->items[Menu_FindItem(pMenu, 2)].max * 10);
    CInput::SetInputRepeatTimingState(((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100) / 4);
}

// Values the in-race option menus started with (restored on cancel).
// GLOBAL: CMR2 0x005298f8
BYTE g_unk0x005298f8;
// GLOBAL: CMR2 0x00529ecc
int g_unk0x00529ecc;
// GLOBAL: CMR2 0x0052a488
int g_unk0x0052a488;
// GLOBAL: CMR2 0x0052a850
BYTE g_unk0x0052a850;
// GLOBAL: CMR2 0x0052ad50
int g_unk0x0052ad50;

BYTE StageUI_GetRaceEndEventCount(void);
BYTE *StageUI_GetRaceResultTable(void);
void Race_UpdatePlayerViewAndDrivenCars(BYTE, int);
void RallyData_SetDriverByteSetting(BYTE index, BYTE *pValue);
void RallyData_MarkTyresChanged(int index);
int RallyData_IsHeadToHeadRaceMode(void);

/* --------------------------------------------------------------------------
   Stage entry / exit transitions of the frontend cascades (0x401000..0x405470).
   -------------------------------------------------------------------------- */

unsigned char Race_LoadSelectedStage(void);
unsigned char Race_LoadStageGeometry(void);
void StageUI_ClearRaceEndLatch(void);
void RallyData_LoadNavigationSounds(void);
void RallyData_RestoreAllCarRaceRecords(void);
void StageSound_LoadSurfaceVariations(void);
void Race_LoadCollisionSoundBanks(void);
void Race_InitCoDriverSamples(void);
void Race_ReleaseFrameResources(void);
void RallyData_DrawLoadingProgress(int progress, char drawScene, BYTE alpha);
void RallyData_GrowEntryAndNotifyCompletion(BYTE *, unsigned int);
BYTE RallyData_FreeChallengeSceneObjects(void);

// GLOBAL: CMR2 0x005297f0
int g_unk0x005297f0;
// GLOBAL: CMR2 0x00536ac8
BYTE g_unk0x00536ac8;

// Enters a stage group: either builds the group's stage list (loading each
// entry) or runs the fade-in sequence of the next group, then forwards the
// event to the stage-list object.
// FUNCTION: CMR2 0x00401000
void GameInfo_EnterStageGroup(BYTE *param1, int param2)
{
    int i;

    if ((char)param2 == 0) {
        if ((BYTE)Race_LoadSelectedStage() == 0) {
            i = 0;
            if (*param1 > 0) {
                do {
                    CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, i, 1, 3);
                    i++;
                } while (i < (int)*param1);
            }
            CGame::SetFrontendResourceMode(0);
            Race_ReleaseFrameResources();
            StageUI_ClearRaceEndLatch();
            return;
        }
        RallyData_DrawLoadingProgress(0x46, 1, 0xff);
        StageSound_LoadSurfaceVariations();
        Race_LoadCollisionSoundBanks();
        Race_InitCoDriverSamples();
        RallyData_LoadNavigationSounds();
        CGame::RegisterCallback((void *)RallyData_FreeChallengeSceneObjects, 0);
        RallyData_DrawLoadingProgress(0x55, 1, 0xff);
    }
    CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 0, 2);
}

// Leaves a stage group: tears the stage list down or runs the fade-out
// sequence, then forwards the event to the stage-list object.
// FUNCTION: CMR2 0x004010a0
void GameInfo_LeaveStageGroup(BYTE *param1, int param2)
{
    int i;

    if ((char)param2 == 0) {
        RallyData_RestoreAllCarRaceRecords();
        if ((BYTE)Race_LoadStageGeometry() == 0) {
            i = 0;
            if (*param1 > 0) {
                do {
                    CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, i, 1, 3);
                    i++;
                } while (i < (int)*param1);
            }
            CGame::SetFrontendResourceMode(0);
            Race_ReleaseFrameResources();
            StageUI_ClearRaceEndLatch();
            return;
        }
        RallyData_DrawLoadingProgress(0x64, 1, 0xff);
        if (CGameInfo::IsConfiguredMultiplayer())
            CGraphics::SetClearColour(0, 0, 0, 0);
        else
            CGraphics::SetClearColour(0, 0x9c, 0xb4, 0xac);
    }
    g_unk0x005297f0 = 1;
    g_unk0x00536ac8 = 1;
    CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 0, 2);
}

void NetPlayers_SendCarClass(BYTE);
void NetRace_SetRaceControlFlag(BYTE);
void NetRace_ResetType8And9PlayerIndices(void);
void Race_SetFlag37F08(void);
void NetRace_ResetStateAndTriangleTable(void);
void StageTiming_RestoreDeviceStageResources(void);
void ForceFeedback_ActivateIdleSlots(void);
void View_SetCameraType(int, int, BYTE, int);
BYTE StageUI_GetRaceEndEventCount(void);
int RallyData_GetDriverCarSelection(BYTE);
int RallyData_IsChampionshipFinalStage(void);
void RallyData_CopyCountryPlayerDefaults(void);
void RallyData_ClearDriverGroupRecords(void);
int View_IsModeAvailable(BYTE, int);
extern BYTE g_unk0x00537fd4;
// GLOBAL: CMR2 0x0053811d
BYTE g_unk0x0053811d;
// GLOBAL: CMR2 0x005298f4
int g_unk0x005298f4;

// Advances the in-race menu cascade one step: refreshes the per-player window
// entries and moves to the next state when the race is over.
// FUNCTION: CMR2 0x00401150
void InRaceMenu_AdvanceCascade(int param1, int param2)
{
    int i;

    if (CGameInfo::GetGameModeOptionBit19() != 0)
        NetPlayers_SendCarClass(2);
    NetRace_SetRaceControlFlag(0);
    g_unk0x0053811d = 0;
    NetRace_ResetType8And9PlayerIndices();
    CSound::NoOpSoundDeviceCallback();
    g_unk0x00537fd4 = 0;
    Race_SetFlag37F08();
    if ((char)param2 == 0) {
        if (g_unk0x005297f0 != 0) {
            NetRace_ResetStateAndTriangleTable();
            StageTiming_RestoreDeviceStageResources();
            g_unk0x005298f4 = 1;
            g_unk0x005297f0 = 0;
            if (**(char **)(StageUI_GetRaceResultTable() + 4) != '\r')
                ForceFeedback_ActivateIdleSlots();
        }
        i = 0;
        if ((BYTE)RallyDataState() > 0) {
            do {
                if ((BYTE)CGameInfo::GetGameInfoSessionFlag() != 0 || (BYTE)RallyData_IsChampionshipFinalStage() != 0) {
                    View_SetCameraType(i, 7, i, 0);
                } else if (View_IsModeAvailable(i, RallyData_GetDriverCarSelection(StageUI_GetRaceEndEventCount() + i)) != 0) {
                    View_SetCameraType(i, RallyData_GetDriverCarSelection(StageUI_GetRaceEndEventCount() + i), i, 0);
                } else {
                    View_SetCameraType(i, 4, i, 0);
                }
                i++;
            } while (i < (BYTE)RallyDataState());
        }
        g_unk0x005298f4--;
        Race_UpdatePlayerViewAndDrivenCars(**(BYTE **)(param1 + 4), 0);
    }
    if (g_unk0x005298f4 <= 0) {
        if ((BYTE)CGameInfo::GetGameInfoSessionFlag() != 0) {
            CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 6, 2);
            return;
        }
        if (CGameInfo::GetConfiguredGameMode() == 4) {
            CGameInfo::SetInputAndGamePaused(1);
            CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 5, 2);
            return;
        }
        if ((BYTE)RallyData_IsChampionshipFinalStage() != 0) {
            RallyData_CopyCountryPlayerDefaults();
            RallyData_ClearDriverGroupRecords();
            CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 1, 2);
            return;
        }
        CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, param2, 0, 2);
    }
}

// Callback 0 of the sound options menu: loads the three volumes.
// FUNCTION: CMR2 0x004012d0
void InRaceMenu_EnterSoundOptions(Menu *pMenu, int param)
{
    g_unk0x00529ecc = CGameInfo::GetMasterSoundVolume();
    g_unk0x0052a488 = CGameInfo::GetEffectsSoundVolume();
    g_unk0x0052ad50 = CGameInfo::GetCoDriverSoundVolume();
    pMenu->items[Menu_FindItem(pMenu, 0)].max = g_unk0x00529ecc / 10;
    pMenu->items[Menu_FindItem(pMenu, 1)].max = g_unk0x0052a488 / 10;
    if (Race_IsFlag24Clear())
        pMenu->items[Menu_FindItem(pMenu, 2)].max = g_unk0x0052ad50 / 10;
}

// Callback 0 of the car setup menu: loads the four switches from the driver's
// setting byte (resetting them to on).
void InRaceMenu_EnterCarSetup(Menu *pMenu, int param);
void InRaceMenu_LeaveCarSetup(Menu *pMenu, char cancel);
void InRaceMenu_EncodeCarSetup(Menu *pMenu);

// Builds the car-setup menu used from the in-race network menu.
// match 80%: below the 90% bar; kept as FUNCTION so reccmp measures it.
// FUNCTION: CMR2 0x00404000
void InRaceMenu_BuildCarSetup(void)
{
    int settingId;

    Menu_Init(&g_menu0x0052aa70, 0, 0, 0, &g_menu0x00529ed8, NULL, 1, 0, 1);
    if ((BYTE)RallyData_GetFlag24() == 0 && (BYTE)RallyData_GetSelectionFlag28() == 0)
        Menu_AddItemType3(&g_menu0x0052aa70, 0, (short)0x64, 2, 0, 0, 0, 0, 0);
    else
        Menu_AddItemType3(&g_menu0x0052aa70, 0, (short)0x30, 2, 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0052aa70, 0, 0x65, 2, 0, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0052aa70, 0, 0x66, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0052aa70, 0, 0x67, 2, 0, 0, 0, 0, 3);
    if (Race_IsFlag24Clear()) {
        if (RallyData_IsHeadToHeadRaceMode() != 0)
            Menu_AddItemType3(&g_menu0x0052aa70, 0, 0x68, (BYTE)2, 0, 0, 0, 0, 4);
        else
            Menu_AddItemType3(&g_menu0x0052aa70, 0, 0x68, (BYTE)3, 0, 0, 0, 0, 4);
    }
void InRaceMenu_ApplyDefaultCarSetup(Menu *pMenu, int param);

    Menu_AddItemType4(&g_menu0x0052aa70, 0, 0x62, (int)InRaceMenu_ApplyDefaultCarSetup, 5);
    Menu_AddItemType2(&g_menu0x0052aa70, 0, 0x3b, &g_menu0x00529ed8, 0, 6);
    Menu_SetCallbacks(&g_menu0x0052aa70, (MenuCallback)InRaceMenu_EnterCarSetup,
                      (MenuCallback)InRaceMenu_EncodeCarSetup, (MenuCallback)InRaceMenu_DrawCarSetupChoices,
                      (MenuCallback)InRaceMenu_LeaveCarSetup);
    Menu_ValidateCursor(&g_menu0x0052aa70, 0);
}

// FUNCTION: CMR2 0x00404130
void InRaceMenu_ApplyDefaultCarSetup(Menu *pMenu, int param)
{
    g_unk0x005298f8 = (g_unk0x005298f8 & 0xfc) | 0x3c;
    RallyData_SetDriverByteSetting(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1], &g_unk0x005298f8);
    pMenu->items[0].max = (g_unk0x005298f8 >> 2) & 1;
    pMenu->items[1].max = (g_unk0x005298f8 >> 3) & 1;
    pMenu->items[2].max = (g_unk0x005298f8 >> 4) & 1;
    pMenu->items[3].max = (g_unk0x005298f8 >> 5) & 1;
    if (RallyData_IsHeadToHeadRaceMode() != 0) {
        if (Race_IsFlag24Clear())
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
    } else if (Race_IsFlag24Clear()) {
        pMenu->items[Menu_FindItem(pMenu, 4)].max = 2;
    }
}

// Callback of the car setup menu: encodes the selected switches and tyres.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00404d00
void InRaceMenu_EncodeCarSetup(Menu *pMenu)
{
    g_unk0x005298f8 = (g_unk0x005298f8 & ~4) | ((pMenu->items[0].max & 1) << 2);
    g_unk0x005298f8 = (g_unk0x005298f8 & ~8) | ((pMenu->items[1].max & 1) << 3);
    g_unk0x005298f8 = (g_unk0x005298f8 & ~0x20) | ((pMenu->items[3].max & 1) << 5);

    if (Race_IsFlag24Clear()) {
        if (RallyData_IsHeadToHeadRaceMode()) {
            if (pMenu->items[Menu_FindItem(pMenu, 4)].max == 0)
                g_unk0x005298f8 = (g_unk0x005298f8 & ~1) | 2;
            else
                g_unk0x005298f8 &= ~3;
        } else {
            g_unk0x005298f8 ^= ((0xfe - pMenu->items[Menu_FindItem(pMenu, 4)].max) ^ g_unk0x005298f8) & 3;
        }
    }

    if (RallyData_IsHeadToHeadRaceMode()) {
        if (pMenu->items[Menu_FindItem(pMenu, 2)].max == 0)
            g_unk0x005298f8 &= ~0x10;
        else
            g_unk0x005298f8 |= 0x10;
    } else {
        if (pMenu->items[Menu_FindItem(pMenu, 2)].max == 0)
            g_unk0x005298f8 &= ~0x10;
        else
            g_unk0x005298f8 |= 0x10;
    }
    RallyData_SetDriverByteSetting(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1], &g_unk0x005298f8);
}

// Callback 1 of the car setup menu: stores the setting byte (or the other
// one when cancelling) and applies the switches.
// FUNCTION: CMR2 0x00404c50
void InRaceMenu_LeaveCarSetup(Menu *pMenu, char cancel)
{
    if (cancel != 0) {
        RallyData_SetDriverByteSetting(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1], &g_unk0x0052a850);
        return;
    }
    RallyData_SetDriverByteSetting(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1], &g_unk0x005298f8);
    CGameInfo::SetSoundOptionBit26((g_unk0x005298f8 >> 2) & 1);
    CGameInfo::SetSoundOptionBit27((g_unk0x005298f8 >> 3) & 1);
    CGameInfo::SetSoundOptionBit28((g_unk0x005298f8 >> 4) & 1);
    CGameInfo::SetSoundOptionBit29((g_unk0x005298f8 >> 5) & 1);
    CGameInfo::SetSoundOptionBits24To25(g_unk0x005298f8 & 3);
    RallyData_MarkTyresChanged((StageUI_GetRaceEndEventCount() & 0xff) + g_unk0x0052af58[1]);
}

// Callback 2 of the network options menu.
// FUNCTION: CMR2 0x00403880
void InRaceMenu_UpdateCameraOffsets(Menu *pMenu)
{
    InRaceMenu_ReadCameraOffsetSliders(pMenu);
}

void NetRace_InitializeRaceSoundVolumes(void);

// Callback 1 of the sound options menu: applies the volumes (or restores the
// ones the menu started with when cancelled).
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00401420
void InRaceMenu_LeaveSoundOptions(Menu *pMenu, char cancel)
{
    if (cancel != 0) {
        CGameInfo::SetMasterSoundVolume(g_unk0x00529ecc);
        CGameInfo::SetEffectsSoundVolume(g_unk0x0052a488);
        CGameInfo::SetCoDriverSoundVolume(g_unk0x0052ad50);
    } else {
        CGameInfo::SetMasterSoundVolume(pMenu->items[Menu_FindItem(pMenu, 0)].max * 10);
        CGameInfo::SetEffectsSoundVolume(pMenu->items[Menu_FindItem(pMenu, 1)].max * 10);
        if (Race_IsFlag24Clear())
            CGameInfo::SetCoDriverSoundVolume(pMenu->items[Menu_FindItem(pMenu, 2)].max * 10);
    }
    CInput::SetInputRepeatTimingState((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100 / 4);
    NetRace_InitializeRaceSoundVolumes();
}

BYTE *RallyData_GetDriverKnockoutOrTeamRecord(BYTE index);

// Callback 0 of the car setup menu: loads the switches from the driver's
// setting byte (keeping a copy to restore on cancel).
// FUNCTION: CMR2 0x00404b80
void InRaceMenu_EnterCarSetup(Menu *pMenu, int param)
{
    g_unk0x0052a850 = *RallyData_GetDriverKnockoutOrTeamRecord(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1]);
    g_unk0x005298f8 = *RallyData_GetDriverKnockoutOrTeamRecord(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1]);
    pMenu->items[0].max = (g_unk0x005298f8 >> 2) & 1;
    pMenu->items[1].max = (g_unk0x005298f8 >> 3) & 1;
    pMenu->items[2].max = (g_unk0x005298f8 >> 4) & 1;
    pMenu->items[3].max = (g_unk0x005298f8 >> 5) & 1;
    if (Race_IsFlag24Clear()) {
        if (RallyData_IsHeadToHeadRaceMode()) {
            if ((g_unk0x005298f8 & 3) == 2) {
                pMenu->items[Menu_FindItem(pMenu, 4)].max = 0;
                return;
            }
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
            return;
        }
        pMenu->items[Menu_FindItem(pMenu, 4)].max = 2 - (g_unk0x005298f8 & 3);
    }
}

// Offset, height and distance the camera menu started with (restored on cancel).
// GLOBAL: CMR2 0x0052af60
FixVector g_unk0x0052af60;
// GLOBAL: CMR2 0x00529914
int g_unk0x00529914;
// GLOBAL: CMR2 0x0052a48c
short g_unk0x0052a48c;

void HudDash_SetCameraOffset(BYTE index, FixVector *pOffset);
void HudDash_SetGaugeTarget(BYTE index, short value);
void HudDash_SetPlayerGaugeValue(BYTE index, int value);
void RallyData_SetDriverCameraOffsets(int *pPos, short heading, int value, int index);

// Callback 1 of the camera options menu: applies the chosen offset (and
// stores it for the driver), or restores the old one when cancelled.
// FUNCTION: CMR2 0x004037c0
void InRaceMenu_LeaveCameraOffsetOptions(Menu *pMenu, char cancel)
{
    if (cancel != 0) {
        HudDash_SetCameraOffset(g_unk0x0052af58[1], &g_unk0x0052af60);
        HudDash_SetPlayerGaugeValue(g_unk0x0052af58[1], g_unk0x00529914);
        HudDash_SetGaugeTarget(g_unk0x0052af58[1], g_unk0x0052a48c);
        return;
    }
    HudDash_SetPlayerGaugeValue(g_unk0x0052af58[1], g_unk0x0052aa5c);
    HudDash_SetCameraOffset(g_unk0x0052af58[1], &g_unk0x0052aa50);
    HudDash_SetGaugeTarget(g_unk0x0052af58[1], g_unk0x0052a86c);
    CGameInfo::SetDefaultCameraParameters((DWORD *)&g_unk0x0052aa50, g_unk0x0052a86c, g_unk0x0052aa5c);
    int id = (StageUI_GetRaceEndEventCount() & 0xff) + g_unk0x0052af58[1];

    RallyData_SetDriverCameraOffsets((int *)&g_unk0x0052aa50, g_unk0x0052a86c, g_unk0x0052aa5c, id);
}

// FUNCTION: CMR2 0x00404ea0
void InRaceMenu_Open(BYTE param1)
{
    g_unk0x0052af58[0] = param1;
    CGameInfo::m_unk0x0052af40 = 1;
    g_unk0x0052ad54 = 0;
    CGameInfo::SetInputAndGamePaused(1);
    g_unk0x0052af41 = 1;
    g_pMenu0x0052af44 = NULL;
    g_pMenu0x0052af48 = &g_menu0x0052ad60;
    Menu_QueueDefaultAction();
}

// FUNCTION: CMR2 0x00404f10
void InRaceMenu_ClearOpeningFlag(void)
{
    g_unk0x0052af41 = 0;
}

// FUNCTION: CMR2 0x00404f20
unsigned char CGameInfo::IsInRaceMenuOpen(void)
{
    return m_unk0x0052af40;
}

// FUNCTION: CMR2 0x00404f30
BYTE InRaceMenu_IsOpening(void)
{
    return g_unk0x0052af41;
}

// Scratch camera state of the in-race step (0x404f40): current offset, height
// and distance interpolated from the menu values.
// GLOBAL: CMR2 0x00529ec8
int g_unk0x00529ec8;
// GLOBAL: CMR2 0x00529cd8
FixVector g_unk0x00529cd8;
#define g_unk0x00529cdc (g_unk0x00529cd8.y)
#define g_unk0x00529ce0 (g_unk0x00529cd8.z)
// GLOBAL: CMR2 0x00529ed0
int g_unk0x00529ed0;
// GLOBAL: CMR2 0x0052a0bc
int g_unk0x0052a0bc;
// GLOBAL: CMR2 0x0052a2a0
short g_unk0x0052a2a0;
// GLOBAL: CMR2 0x0052aa64
int g_unk0x0052aa64;
// GLOBAL: CMR2 0x0052aa6c
short g_unk0x0052aa6c;
extern int g_unk0x00537f34[2];

extern BYTE g_unk0x0053811e;
extern BYTE g_unk0x0053811f;
extern BYTE g_unk0x00538120;
extern BYTE g_unk0x00538100;
extern int g_unk0x00537f30;
extern int g_unk0x00516090;

void NetPlayers_UpdateBestRaceTime(unsigned int time);
void NetPlayers_SendCarClass(BYTE carClass);
unsigned int NetPlayers_IsPlayerPresent(int index);
unsigned int NetPlayers_HasPlayerFinished(int index);
void View_SnapCameras(BYTE index);
void View_BlendCameraStates(BYTE index, int param);
void View_SwitchCamera(BYTE index, int a, int b, BYTE c, int d);
void ForceFeedback_StopSlotForces(void);
int StageTiming_EstimateElapsedStageTime(void);
int StageTiming_GetValidStartTime(int index);
void NetRace_SendStageAndOverallTimes(int time);
void NetRace_UpdatePlayerFlashTimer(unsigned int player, int check);
void NetRace_SetPlayerFlashIntensity(unsigned int player, int t, int check);
short Car_GetOrderCount(void);
int Race_ReadState37F60(void);
int View_GetActiveCameraMode(BYTE index);
BYTE View_GetActiveCameraFlags(BYTE index);
int HudDash_GetPlayerGaugeValue(BYTE index);
void HudDash_GetCameraUpVector(FixVector *pOut, BYTE index);
short HudDash_GetGaugeTarget(BYTE index);
void HudDash_SetCameraOffset(BYTE index, FixVector *pOffset);
void HudDash_SetGaugeTarget(BYTE index, short value);
void HudDash_SetPlayerGaugeValue(BYTE index, int value);
int StageUI_GetRaceResultValue(void);
BYTE StageUI_GetRaceEndEventCount(void);
void GameMenu_FinishNetworkResultsQuitFade(BYTE index);
typedef void (*FadeCallback)(BYTE index);
void NetRace_FadeOutPlayerScreen(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
void View_SetShake(int view, int start);
void RallyData_SetDriverCarSelection(BYTE index, int value);

// One frame of the in-race state machine: while the race menu is up it pushes
// the camera settings and the driver names of the active player, then updates
// the car order, the input menu and the race-start/restart transitions.
// FUNCTION: CMR2 0x00404f40
void InRaceMenu_UpdateStateMachineFrame(Unk0049c2c0 *param1)
{
    int first;
    int second;
    int result;
    int i;
    DeviceInfo *pDev;

    ForceFeedback_StopSlotForces();
    if (g_pMenu0x0052af44 != &g_menu0x0052a870)
        goto updateMenus;

    result = View_GetActiveCameraMode(g_unk0x0052af58[1]);
    if (result == 5 || result == 8)
        goto pushCamera;

    View_SwitchCamera(g_unk0x0052af58[1], 5, 0xffff, View_GetActiveCameraFlags(g_unk0x0052af58[1]), 1);
    switch (StageUI_GetRaceResultValue()) {
    case 4:
        first = (StageUI_GetRaceEndEventCount() & 0xff) + *(BYTE *)&g_unk0x0052af58[1];
        break;
    case 0:
    case 1:
        first = g_unk0x0052af58[1];
        break;
    case 2:
        RallyData_GetRoundDrivers((unsigned int *)&first, (unsigned int *)&second);
        break;
    case 3:
        if (g_unk0x0052af58[1] == 0)
            RallyData_GetRoundDrivers((unsigned int *)&first, (unsigned int *)&second);
        else
            RallyData_GetRoundDrivers((unsigned int *)&second, (unsigned int *)&first);
        break;
    }
    RallyData_SetDriverCarSelection(first, 5);

pushCamera:
    g_unk0x00529ed0 = HudDash_GetPlayerGaugeValue(g_unk0x0052af58[1]);
    HudDash_GetCameraUpVector(&g_unk0x00529cd8, g_unk0x0052af58[1]);
    g_unk0x0052a2a0 = HudDash_GetGaugeTarget(g_unk0x0052af58[1]);
    g_unk0x0052aa64 = g_unk0x0052aa5c - g_unk0x00529ed0;
    g_unk0x00529ec8 = g_unk0x0052aa54 - g_unk0x00529cdc;
    g_unk0x0052aa6c = g_unk0x0052a86c - g_unk0x0052a2a0;
    g_unk0x0052a0bc = g_unk0x0052aa58 - g_unk0x00529ce0;
    g_unk0x0052a86c = g_unk0x0052a2a0 + FixMul(g_unk0x0052aa6c, 0x1999);
    g_unk0x0052aa50.x = 0;
    g_unk0x0052aa54 = g_unk0x00529cdc + FixMul(g_unk0x00529ec8, 0x1999);
    g_unk0x0052aa58 = g_unk0x00529ce0 + FixMul(g_unk0x0052a0bc, 0x1999);
    HudDash_SetPlayerGaugeValue(g_unk0x0052af58[1], g_unk0x0052aa5c);
    HudDash_SetCameraOffset(g_unk0x0052af58[1], &g_unk0x0052aa50);
    HudDash_SetGaugeTarget(g_unk0x0052af58[1], g_unk0x0052a86c);

    if (g_unk0x0052ad54 != 0) {
        View_SnapCameras(g_unk0x0052af58[1]);
        result = Race_ReadState37F60();
        View_BlendCameraStates(g_unk0x0052af58[1], result);
        result = g_pMenu0x0052af44 == &g_menu0x0052a870 &&
                 g_pMenu0x0052af44->cursor == 2 ? 1 : 0;
        View_SetShake(g_unk0x0052af58[1], result);
    }

updateMenus:
    i = 0;
    if (Car_GetOrderCount() > 0) {
        do {
            NetRace_UpdatePlayerFlashTimer(i, 1);
            NetRace_SetPlayerFlashIntensity(i, Race_ReadState37F60(), 1);
            i++;
        } while (i < Car_GetOrderCount());
    }
    g_pMenu0x0052af44 = g_pMenu0x0052af48;
    CInput::UpdateAllAvailableDevices();
    Input_TranslatePedalsToMenuKeys();
    pDev = CInput::GetAvailableDeviceRecord(g_unk0x0052af58[0]);
    Input_MergeAssignedJoystickButtons(*(unsigned int *)&g_unk0x0052af58[0] & 0xff, pDev);
    if ((pDev->field_0x8 & (unsigned short)g_unk0x0052a0b8) != 0) {
        g_unk0x0052a0b8 = 0;
        if (g_pMenu0x0052af44 != NULL)
            Menu_CallCallback3(g_pMenu0x0052af44);
        InRaceMenu_Close();
    } else {
        if (g_pMenu0x0052af44 != NULL)
            Menu_SetFlags(g_pMenu0x0052af44, 1, 1, 1, 1);
        result = Menu_Update(g_pMenu0x0052af44, pDev->field_0x8);
        if (result != 0)
            g_pMenu0x0052af48 = (Menu *)result;
    }

    if (CGameInfo::GetGameModeOptionBit19() == 0)
        return;
    g_unk0x0053811e = 0;
    g_unk0x0053811f = 0;
    if (GameInfo_GetSessionField398C() > -1 && CGameInfo::GetConfiguredGameMode() != 0xa && CGameInfo::GetConfiguredGameMode() != 0xc) {
        if (g_unk0x0053811d != 0) {
            if ((unsigned int)(CMain::GetFrameDelta() - g_unk0x00537f30) >
                (unsigned int)(GameInfo_GetSessionField398C() * 100))
                g_unk0x0053811e = 1;
        } else {
            i = 0;
            do {
                if ((char)NetPlayers_IsPlayerPresent(i) != 0 && (char)NetPlayers_HasPlayerFinished(i) != 0) {
                    g_unk0x0053811d = 1;
                    if (g_unk0x00538120 == 0) {
                        g_unk0x00538120 = 1;
                        g_unk0x00537f30 = CMain::GetFrameDelta();
                    }
                }
                i++;
            } while (i < 7);
        }
    }
    if (CGameInfo::GetConfiguredGameMode() == 0xa || CGameInfo::GetConfiguredGameMode() == 0xc) {
        CGameInfo::GetConfiguredGameMode();
        i = CMain::GetFrameDelta();
        result = NetPlayers_GetInputFrameDelta();
        if ((unsigned int)(i - result) >
                (unsigned int)(GameInfo_GetSessionField397C() * 6000) &&
            Network_GetSessionStateFlag() != 0) {
            if (g_unk0x00538100 != 0)
                goto playerExit;
            g_unk0x0053811f = 1;
            NetPlayers_SendType16Notification();
            g_unk0x00538100 = 1;
            NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkResultsQuitFade, 1, 0, g_unk0x00516090, 1);
        }
        if (g_unk0x00538100 != 0) {
playerExit:
            Race_UpdatePlayerViewAndDrivenCars(*(BYTE *)param1->unk, 1);
            return;
        }
    }
    if (g_unk0x0053811e != 0 || g_unk0x0053811f != 0) {
        if (CGameInfo::GetGameModeOptionBit19() != 0)
            NetPlayers_SendCarClass(3);
        g_unk0x00537f34[0] = CMain::GetFrameDelta();
    }
    CGameInfo::GetConfiguredGameMode();
    Race_UpdatePlayerViewAndDrivenCars(*(BYTE *)param1->unk, 0);
    if (g_unk0x0053811e != 0) {
        View_SwitchCamera(0, 7, 0xffff, View_GetActiveCameraFlags(0), 0);
        if (g_unk0x0053811f == 0) {
            if (g_unk0x0053811e != 0)
                result = StageTiming_EstimateElapsedStageTime();
            else
                result = StageTiming_GetValidStartTime(0);
            NetRace_SendStageAndOverallTimes(result);
            if (g_unk0x00538120 == 0 && GameInfo_GetSessionField398C() > -1) {
                g_unk0x00538120 = 1;
                g_unk0x00537f30 = CMain::GetFrameDelta();
            }
            result = StageTiming_GetValidStartTime(0);
            NetPlayers_UpdateBestRaceTime(result);
        }
        if (g_unk0x0053811e != 0) {
            InRaceMenu_Close();
            CGame::PromoteCallbackEntryByRule(param1, 0, 0, 2);
        }
    }
}

void RallyData_GrowEntryAndNotifyCompletion(BYTE *, unsigned int);

// Advances every entry of a menu screen one step.
// FUNCTION: CMR2 0x00405470
void InRaceMenu_AdvanceScreenEntries(BYTE *param1)
{
    BYTE i;

    i = 0;
    if (*param1 > 0) {
        do {
            // the original passes the byte counter as a full dword (the callee
            // keeps only its low byte)
            RallyData_GrowEntryAndNotifyCompletion(param1, *(unsigned int *)&i);
            i++;
        } while (i < *param1);
    }
}

// FUNCTION: CMR2 0x004054a0
void InRaceMenu_DrawActivePage(void)
{
    Menu_CallCallback2(g_pMenu0x0052af44);
}

// FUNCTION: CMR2 0x004054b0
int InRaceMenu_IsPlayerEditingCarSetup(unsigned int param1)
{
    if (CGameInfo::IsInRaceMenuOpen() && g_unk0x0052af58[1] == param1 && g_pMenu0x0052af44 == &g_menu0x0052aa70)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x004055e0
int InRaceMenu_GetUpArrowTexture(void)
{
    return g_unk0x0052aa60;
}

// FUNCTION: CMR2 0x004055f0
int InRaceMenu_GetDownArrowTexture(void)
{
    return g_unk0x0052aa68;
}

// FUNCTION: CMR2 0x00405600
int InRaceMenu_GetRoundBoxTexture(void)
{
    return g_unk0x0052af4c;
}

// FUNCTION: CMR2 0x00405610
int InRaceMenu_GetCurtainTexture(void)
{
    return g_unk0x0052af50;
}

// Text buffer for the truncated draw below (256 bytes up to the next global).
// GLOBAL: CMR2 0x0082b1c0
char g_unk0x0082b1c0[0x100];

// Draws the record's text; in mode 1 it truncates the string at the 16.16
// fraction of its length and draws the remainder separately.
// FUNCTION: CMR2 0x00501f80
void OptionMenu_DrawTransitionTextShortCoords(int index, int font1, int font2, char *text, short x, short y,
                  int *pColour1, int *pColour2, unsigned int flags)
{
    Unk0x0082b2c0 *pRec;
    int len;
    int count;
    int width;
    int i;

    pRec = &g_unk0x0082b2c0[index];
    if (pRec->field_0xc == 2) {
        Font_DrawText(font1, text, x, y, pColour1, flags);
        return;
    }
    if (pRec->field_0xc == 1) {
        len = (int)strlen(text);
        count = FixMulShift32(len << 16, pRec->field_0x0);
        for (i = 0; i < count; i++)
            g_unk0x0082b1c0[i] = text[i];
        g_unk0x0082b1c0[count] = 0;
        Font_DrawText(font1, g_unk0x0082b1c0, x, y, pColour1, flags);
        if ((flags == 9 || flags == 0x11 || flags == 0x21) && count < len) {
            width = Font_GetTextWidth(font1, (BYTE *)g_unk0x0082b1c0);
            g_unk0x0082b1c0[0] = text[count];
            g_unk0x0082b1c0[1] = 0;
            Font_DrawText(font2, g_unk0x0082b1c0, width + x, y, pColour2, flags);
        }
    }
}

#include "Sprite.h"
#include "Graphics.h"

// Draws the four one pixel edges of a rectangle given as x, y, w, h.
// FUNCTION: CMR2 0x00403ef0
void DrawRectOutline(short *pRect, BYTE *pColour)
{
    short edge[4];

    edge[0] = pRect[0];
    edge[1] = pRect[1];
    edge[2] = pRect[2];
    edge[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
    edge[0] = pRect[2] + pRect[0];
    edge[1] = pRect[1];
    edge[2] = 1;
    edge[3] = pRect[3] + 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
    edge[0] = pRect[0];
    edge[1] = pRect[3] + pRect[1];
    edge[2] = pRect[2];
    edge[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
    edge[0] = pRect[0];
    edge[1] = pRect[1];
    edge[2] = 1;
    edge[3] = pRect[3];
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
}

void View_SetShake(int view, int start);
extern BYTE g_unk0x0052af58[2];

// ---------------------------------------------------------------------------
// In-race network menu: the drawing helpers and the menu constructors.

// Colours the in-race menu text and the selected/unselected sprites use.
// GLOBAL: CMR2 0x00516074
int g_unk0x00516074 = 0xfffafafa;
// GLOBAL: CMR2 0x00516078
int g_unk0x00516078 = 0xffdbaca7;
// GLOBAL: CMR2 0x00516084
int g_unk0x00516084 = 0x80fafafa;
// GLOBAL: CMR2 0x0051608c
int g_unk0x0051608c = 0xbfae8072;

void GameMenu_RequestStageRestart(Menu *pMenu, int param);
void GameMenu_RequestNetworkRestart(Menu *pMenu, int param);
void GameMenu_RequestNetworkQuit(Menu *pMenu, int param);
void GameMenu_RequestNetworkLeave(Menu *pMenu, int param);
void InRaceMenu_DrawMenuRows(Menu *pMenu);
void InRaceMenu_DrawPlayerClassRows(Menu *pMenu);
void InRaceMenu_DrawNetworkPlayerList(Menu *pMenu);
// Scratch rectangle the in-race menu draws reuse (0x52ad58, right below the main
// menu at 0x52ad60).
// GLOBAL: CMR2 0x0052ad58
short g_unk0x0052ad58[4];
extern char g_classRowHeaderFormat[];
void OptionMenu_ConsumeRefreshRequest(int unused, int unused2);
void OptionMenu_UpdateMessagesAndRecordStates(Menu *);
void OptionMenu_LoadGameItem(Menu *, MenuItem *);
void OptionMenu_ApplyHighlightedValue(Menu *, MenuItem *);
void OptionMenu_AdvanceSelectedOption(int unused, int unused2);
void OptionMenu_ResetValueAnimation(int param1, int param2);
void OptionMenu_RestartCountdown(int unused, int unused2);
void OptionMenu_RestartEmptyItemRecords(BYTE *pItem, int unused);
void OptionMenu_IncrementSharedValue(Menu *pMenu, char param);
void OptionMenu_BuildStatusMenu(void);
void OptionMenu_BuildAdvancedOptions(void);
void OptionMenu_BuildControlSetup(void);
void OptionMenu_BuildValueSliders(void);
void OptionMenu_LoadPartTextures(void);
void OptionPreview_UpdateAllSlots(void);
void OptionMenu_DispatchRecordEditState(unsigned int);
void OptionMenu_RebuildOptions(void);

// Draws the header of the in-race menu bar: the car count above the bar and
// the bar itself.
// FUNCTION: CMR2 0x00401b60
void InRaceMenu_DrawHeaderBar(void)
{
    short rect[4];
    int x;
    int y;
    int count;

    x = (int)(g_pGraphics->resX * 0x1f) / 0x280;
    count = (StageUI_GetRaceEndEventCount() & 0xff) + (*(int *)g_unk0x0052af58 & 0xff) + 1;
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x98), count);
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x45) / 0x1e0,
                  &g_unk0x00516074, 0x11);
    y = (int)(g_pGraphics->resY * 10) / 0x1e0 + x
        + Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    rect[0] = (short)y;
    rect[2] = 1;
    rect[1] = (short)((int)(g_pGraphics->resY * 0x25) / 0x1e0);
    rect[3] = (short)((int)(g_pGraphics->resY * 0x2d) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x00516074, 2);
    rect[0] = rect[0] + 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x00516084, 2);
    Font_DrawText(2, CFrontend::GetTextString(0x18), y + (int)(g_pGraphics->resY * 10) / 0x1e0,
                  (int)(g_pGraphics->resY * 0x45) / 0x1e0, &g_unk0x00516074, 0x11);
}

// Draws the list of an in-race menu (0x402c90) with the selected row in a
// brighter colour; the third row is left blank.
// FUNCTION: CMR2 0x00401870
void InRaceMenu_DrawOptionList(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    int i;
    int y;
    int string1;
    int string2;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x0051608c, 2);
    InRaceMenu_DrawHeaderBar();
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    rect[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0;
    i = 0;
    if (pMenu->itemCount > 0) {
        pItem = pMenu->items;
        do {
            rect[1] = (short)(y - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            if (i != 2) {
                string1 = pItem->stringId;
                if (string1 != 0) {
                    string2 = string1;
                } else {
                    string1 = (int)CFrontend::GetTextString(pItem->id);
                    string2 = (int)CFrontend::GetTextString(pItem->id + 1);
                }
                if (pMenu->cursor == i) {
                    Font_DrawText(1, (char *)string1, (int)(g_pGraphics->resX * 0x86) / 0x280, y,
                                  &g_unk0x00516074, 0x11);
                    Font_DrawText(0, (char *)string2, (int)(g_pGraphics->resX * 0x86) / 0x280,
                                  (int)(g_pGraphics->resY * 0xf) / 0x1e0 + y, &g_unk0x00516074, 0x11);
                    Sprite_Queue((SpriteRect *)(g_unk0x0052aa60 + 0x11c), (SpriteRect *)rect,
                                 (Texture *)g_unk0x0052aa60, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516074, 8);
                } else {
                    Font_DrawText(1, (char *)string1, (int)(g_pGraphics->resX * 0x86) / 0x280, y,
                                  &g_unk0x00516078, 0x11);
                    Font_DrawText(0, (char *)string2, (int)(g_pGraphics->resX * 0x86) / 0x280,
                                  (int)(g_pGraphics->resY * 0xf) / 0x1e0 + y, &g_unk0x00516078, 0x11);
                    Sprite_Queue((SpriteRect *)(g_unk0x0052aa68 + 0x11c), (SpriteRect *)rect,
                                 (Texture *)g_unk0x0052aa68, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516078, 8);
                }
                y = y + (int)(g_pGraphics->resY * 0x36) / 0x1e0;
            }
            i++;
            pItem++;
        } while (i < pMenu->itemCount);
    }
    Font_SetBlendMode(2);
}

// Draws every row of an in-race menu (no blank row), selected row brighter.
// FUNCTION: CMR2 0x00401d20
void InRaceMenu_DrawMenuRows(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    int i;
    int y;
    int string1;
    int string2;
    int *pColour;
    int texture;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x0051608c, 2);
    InRaceMenu_DrawHeaderBar();
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    rect[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0;
    pItem = pMenu->items;
    for (i = 0; i < pMenu->itemCount; i++, pItem++) {
        rect[1] = (short)(y - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
        string1 = pItem->stringId;
        string2 = string1;
        if (string1 == 0) {
            string1 = (int)CFrontend::GetTextString(pItem->id);
            string2 = (int)CFrontend::GetTextString(pItem->id + 1);
        }
        if (pMenu->cursor == i) {
            Font_DrawText(1, (char *)string1, (int)(g_pGraphics->resX * 0x86) / 0x280, y,
                          &g_unk0x00516074, 0x11);
            Font_DrawText(0, (char *)string2, (int)(g_pGraphics->resX * 0x86) / 0x280,
                          (int)(g_pGraphics->resY * 0xf) / 0x1e0 + y, &g_unk0x00516074, 0x11);
            pColour = &g_unk0x00516074;
            texture = g_unk0x0052aa60;
        } else {
            Font_DrawText(1, (char *)string1, (int)(g_pGraphics->resX * 0x86) / 0x280, y,
                          &g_unk0x00516078, 0x11);
            Font_DrawText(0, (char *)string2, (int)(g_pGraphics->resX * 0x86) / 0x280,
                          (int)(g_pGraphics->resY * 0xf) / 0x1e0 + y, &g_unk0x00516078, 0x11);
            pColour = &g_unk0x00516078;
            texture = g_unk0x0052aa68;
        }
        Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect, (Texture *)texture,
                     2, 0, NULL, NULL, (BYTE *)pColour, 8);
        y = y + (int)(g_pGraphics->resY * 0x36) / 0x1e0;
    }
    Font_SetBlendMode(2);
}

// Draws a player list of the in-race network menu: each row shows the player
// name, the secondary line and the class split entries.
// match 67%: the original reads an uninitialised local in the value != 0 path
// and keeps the item pointer in a register; kept as FUNCTION so reccmp measures it.
// FUNCTION: CMR2 0x00402460
void InRaceMenu_DrawNetworkPlayerList(Menu *pMenu)
{
    int i;
    int j;
    int x;
    char *str1;
    char *str2;
    char *sub[3];
    char *text;
    int *pA;
    int *pB;
    char single;
    bool twoLines;
    int *pColour;
    int texture;

    g_unk0x0052ad58[0] = 0;
    g_unk0x0052ad58[1] = 0;
    g_unk0x0052ad58[2] = (short)g_pGraphics->resX;
    g_unk0x0052ad58[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x0052ad58, (BYTE *)&g_unk0x0051608c, 2);
    InRaceMenu_DrawHeaderBar();
    g_unk0x0052ad58[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    g_unk0x0052ad58[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    g_unk0x0052ad58[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    for (i = 0; i < pMenu->itemCount; i++) {
        MenuItem *pItem = &pMenu->items[i];
        if (pItem->visible) {
            g_unk0x0052ad58[1] = (short)((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                         + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i
                                         - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            str1 = CFrontend::GetTextString(pItem->id);
            str2 = CFrontend::GetTextString(pItem->id + 1);
            if (pMenu->cursor == i) {
                pColour = &g_unk0x00516074;
                texture = g_unk0x0052aa60;
            } else {
                pColour = &g_unk0x00516078;
                texture = g_unk0x0052aa68;
            }
            Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)g_unk0x0052ad58,
                         (Texture *)texture, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516078, 8);
            twoLines = true;
            if (pItem->value != 0) {
                sub[0] = CFrontend::GetTextString(pItem->id);
                twoLines = false;
            } else {
                for (j = 0; j < (int)(BYTE)pItem->min; j++)
                    sub[j] = CFrontend::GetTextString(j + 0x9c);
                single = 0;
            }
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            x = (int)(g_pGraphics->resX * 0x86) / 0x280;
            Font_DrawText(1, str1, x,
                          (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                              + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,
                          pColour, 0x11);
            if (twoLines)
                Font_DrawText(0, str2, x,
                              (int)(g_pGraphics->resY * 0xf) / 0x1e0
                                  + (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                  + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,
                              pColour, 0x11);
            pB = (int *)sub + 1;
            pA = (int *)sub;
            for (j = 0; j < (int)(BYTE)pItem->min; j++) {
                text = single == 0 ? (char *)*pA : (char *)*pB;
                x = (int)(g_pGraphics->resX * 10) / 0x280 + x
                    + Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                if (pItem->max != j)
                    Font_DrawText(1, text, x,
                                  (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                      + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i, &g_unk0x00516078, 0x11);
                else
                    Font_DrawText(1, text, x,
                                  (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                      + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i, &g_unk0x00516074, 0x11);
                strcpy(CFrontend::m_stringDest, text);
                pB--;
                pA++;
            }
        }
    }
    Font_SetBlendMode(2);
}

// Draws the player/class rows of the option menu: the class header (name plus
// its percentage) and the sprite behind each row.
// FUNCTION: CMR2 0x004028d0
void InRaceMenu_DrawPlayerClassRows(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    int i;
    int *pColour;
    int type;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x0051608c, 2);
    InRaceMenu_DrawHeaderBar();
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    rect[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    pItem = pMenu->items;
    for (i = 0; i < pMenu->itemCount; i++, pItem++) {
        pColour = pMenu->cursor == i ? &g_unk0x00516074 : &g_unk0x00516078;
        type = pItem->value;
        if (type >= 0) {
            if (type > 2) {
                if (type == 4) {
                    Font_DrawText(1, CFrontend::GetTextString(pItem->id),
                                  (int)(g_pGraphics->resX * 0x86) / 0x280,
                                  (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                      + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,
                                  pColour, 0x11);
                }
            } else {
                sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat,
                        CFrontend::GetTextString(pItem->id), pItem->max * 10);
                Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x86) / 0x280,
                              (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                  + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,
                              pColour, 0x11);
            }
        }
        rect[1] = (short)(((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                           + ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i)
                          - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
        if (pMenu->cursor == i) {
            Sprite_Queue((SpriteRect *)(g_unk0x0052aa60 + 0x11c), (SpriteRect *)rect,
                         (Texture *)g_unk0x0052aa60, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516074, 8);
        } else {
            Sprite_Queue((SpriteRect *)(g_unk0x0052aa68 + 0x11c), (SpriteRect *)rect,
                         (Texture *)g_unk0x0052aa68, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516078, 8);
        }
    }
    Font_SetBlendMode(2);
}

// Rebuilds the main in-race menu. When param is non-zero it keeps the value of
// the item currently under the cursor and re-selects it.
// FUNCTION: CMR2 0x00402c90
void InRaceMenu_RebuildMainPage(int param)
{
    int value;
    int index;
    short id;

    InRaceMenu_BuildResumePage();
    g_unk0x0052a0b8 = 0;
    value = param;
    if ((BYTE)param != 0)
        value = (int)g_menu0x0052ad60.items[g_menu0x0052ad60.cursor].value;
    Menu_Init(&g_menu0x0052ad60, 0, 0, 0, NULL, NULL, 1, 0, 1);
    if ((char)RallyDataState() == 1)
        Menu_AddItemType2(&g_menu0x0052ad60, 0, 9, &g_menu0x00529ed8, 0, 0);
    else
        Menu_AddItemType2(&g_menu0x0052ad60, 0, 9, &g_menu0x00529918, 0, 0);
    Menu_AddItemType2(&g_menu0x0052ad60, 0, 0xb, &g_menu0x0052a670, 0, 1);
    Menu_AddItemType2(&g_menu0x0052ad60, 0, 0xd, &g_menu0x0052a490, 0, 2);
    if (CGameInfo::GetGameModeOptionBit19() != 0) {
        if ((Network_GetSessionStateFlag() != 0 && CGameInfo::GetConfiguredGameMode() != 0xc) ||
            (Network_GetSessionStateFlag() == 0 && CGameInfo::GetConfiguredGameMode() == 0xa))
            Menu_AddItemType2(&g_menu0x0052ad60, 0, 0xf, &g_menu0x00529af8, 0, 3);
        if (Network_GetSessionStateFlag() != 0)
            Menu_AddItemType2(&g_menu0x0052ad60, 0, 0xf6, &g_menu0x0052a2a8, 0, 5);
    } else {
        Menu_AddItemType2(&g_menu0x0052ad60, 0, 0xf, &g_menu0x00529af8, 0, 3);
    }
    Menu_AddItemType2(&g_menu0x0052ad60, 0, 0x11, &g_menu0x0052a0c0, 0, 6);
    Menu_AddItemType4(&g_menu0x0052ad60, 0, 0x13, (int)InRaceMenu_ResumeWithSound, 7);
    Menu_SetCallbacks(&g_menu0x0052ad60, (MenuCallback)InRaceMenu_RememberOptionCursor, (MenuCallback)InRaceMenu_SkipBlankOptionRow,
                      (MenuCallback)InRaceMenu_DrawOptionList, NULL);
    if ((BYTE)param != 0)
        Menu_SelectItem(&g_menu0x0052ad60, value);
    else
        Menu_ValidateCursor(&g_menu0x0052ad60, 0);
    switch (CGameInfo::GetConfiguredGameMode()) {
    case 0:
    case 1:
    case 8:
        id = 0x25;
        break;
    case 2:
    case 3:
    case 9:
    case 10:
        id = 0x23;
        break;
    case 4:
        id = 0x27;
        break;
    case 5:
        id = 0x2b;
        break;
    case 6:
    case 7:
    case 0xb:
    case 0xc:
        id = 0x29;
        break;
    default:
        id = (short)param;
        break;
    }
    index = Menu_FindItem(&g_menu0x0052ad60, 3);
    g_menu0x0052ad60.items[index].id = id;
    index = Menu_FindItem(&g_menu0x00529af8, 3);
    g_menu0x00529af8.items[index].id = id;
}

// Builds the camera options submenu of the in-race menu.
// FUNCTION: CMR2 0x00403090
void InRaceMenu_BuildCameraModeOptions(void)
{
    Menu_Init(&g_menu0x0052a490, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0052a490, 0, 0x3c, 2, 0, 0, 0, 0, 0);;
    Menu_AddItemType4(&g_menu0x0052a490, 0, 0x3b, (int)InRaceMenu_ReturnToPauseItem, -1);
    Menu_SetCallbacks(&g_menu0x0052a490, (MenuCallback)CGame::NoOpSecondaryStateCallback,
                      (MenuCallback)InRaceMenu_ReturnToPauseUpdate, (MenuCallback)InRaceMenu_DrawNetworkPlayerList, NULL);
    Menu_ValidateCursor(&g_menu0x0052a490, 0);
}

// Builds the sound options submenu (the three volume sliders).
// FUNCTION: CMR2 0x004032a0
void InRaceMenu_BuildSoundOptions(void)
{
    Menu_Init(&g_menu0x0052a670, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0052a670, 0, 0x19, 0xb, 10, 0, 0, 0, 0);;
    Menu_AddItemType3(&g_menu0x0052a670, 0, 0x1a, 0xb, 10, 0, 0, 0, 1);;
    if (Race_IsFlag24Clear())
        Menu_AddItemType3(&g_menu0x0052a670, 0, 0x1b, 0xb, 10, 0, 0, 0, 2);;
    Menu_AddItemType2(&g_menu0x0052a670, 0, 0x3b, &g_menu0x0052ad60, 0, 4);
    Menu_SetCallbacks(&g_menu0x0052a670, (MenuCallback)InRaceMenu_EnterSoundOptions, (MenuCallback)InRaceMenu_ApplySoundVolumes,
                      (MenuCallback)InRaceMenu_DrawPlayerClassRows, (MenuCallback)InRaceMenu_LeaveSoundOptions);
    Menu_ValidateCursor(&g_menu0x0052a670, 0);
}

// Builds the in-race pause menu.
// FUNCTION: CMR2 0x00403370
void InRaceMenu_BuildPausePage(void)
{
    Menu_Init(&g_menu0x00529af8, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00529af8, 0, 0xf, (int)GameMenu_RequestStageRestart, 3);
    if (CGameInfo::GetConfiguredGameMode() == 2 || CGameInfo::GetConfiguredGameMode() == 3 ||
        CGameInfo::GetConfiguredGameMode() == 0xa || CGameInfo::GetConfiguredGameMode() == 9)
        Menu_AddItemType4(&g_menu0x00529af8, 0, 0x8b, (int)GameMenu_RequestNetworkRestart, 4);
    Menu_AddItemType1(&g_menu0x00529af8, 0, 0xa6, 0, -1);
    Menu_SetCallbacks(&g_menu0x00529af8, (MenuCallback)InRaceMenu_SelectLastItem, NULL, (MenuCallback)InRaceMenu_DrawMenuRows,
                      NULL);
    Menu_ValidateCursor(&g_menu0x00529af8, 0);
}

// Builds the "quit" submenu of the pause menu.
// FUNCTION: CMR2 0x00403420
void InRaceMenu_BuildQuitPage(void)
{
    Menu_Init(&g_menu0x0052a2a8, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0052a2a8, 0, 0xf6, (int)GameMenu_RequestNetworkLeave, 3);
    Menu_AddItemType1(&g_menu0x0052a2a8, 0, 0xa6, 0, -1);
    Menu_SetCallbacks(&g_menu0x0052a2a8, (MenuCallback)InRaceMenu_SelectLastItem, NULL, (MenuCallback)InRaceMenu_DrawMenuRows,
                      NULL);
    Menu_ValidateCursor(&g_menu0x0052a2a8, 0);
}

// Builds the "leave" submenu of the pause menu.
// FUNCTION: CMR2 0x00403490
void InRaceMenu_BuildLeavePage(void)
{
    Menu_Init(&g_menu0x0052a0c0, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0052a0c0, 0, 0xa8, (int)GameMenu_RequestNetworkQuit, 3);
    Menu_AddItemType1(&g_menu0x0052a0c0, 0, 0xaa, 0, -1);
    Menu_SetCallbacks(&g_menu0x0052a0c0, (MenuCallback)InRaceMenu_SelectLastItem, NULL, (MenuCallback)InRaceMenu_DrawMenuRows,
                      NULL);
    Menu_ValidateCursor(&g_menu0x0052a0c0, 0);
}

void InRaceMenu_BuildPlayerOptions(void);

// Resets the pause-menu state and rebuilds all of its menus.
// FUNCTION: CMR2 0x00403500
void InRaceMenu_ResetAndBuildPages(void)
{
    CGameInfo::m_unk0x0052af40 = 0;
    g_unk0x0052af58[0] = 0;
    g_unk0x0052af58[1] = 0;
    InRaceMenu_BuildPausePage();
    InRaceMenu_BuildQuitPage();
    InRaceMenu_BuildLeavePage();
    InRaceMenu_RebuildMainPage(0);
    InRaceMenu_BuildNetworkOptions();
    InRaceMenu_BuildPlayerOptions();
    InRaceMenu_BuildCameraOffsetOptions();
    InRaceMenu_BuildCarSetup();
    InRaceMenu_BuildSoundOptions();
    InRaceMenu_BuildCameraModeOptions();
}

// Builds the options submenu of the pause menu.
// FUNCTION: CMR2 0x00403550
void InRaceMenu_BuildPlayerOptions(void)
{
    Menu_Init(&g_menu0x00529918, 0, 0, 0, &g_menu0x0052ad60, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00529918, 0, 0x58, &g_menu0x00529ed8, (int)InRaceMenu_SelectPlayer, 0);
    Menu_AddItemType2(&g_menu0x00529918, 0, 0x5a, &g_menu0x00529ed8, (int)InRaceMenu_SelectPlayer, 1);
    Menu_SetCallbacks(&g_menu0x00529918, NULL, NULL, (MenuCallback)InRaceMenu_DrawOptionList, NULL);
    Menu_ValidateCursor(&g_menu0x00529918, 0);
}

// Printed when the weather/damage/car setup was re-initialised.
// GLOBAL: CMR2 0x00527004
char g_strNeedReinitWeather[] = "Need to re-initialise weather, damage and car setup\n";

void RallyData_CopyCountryPlayerDefaults(void);
BYTE RallyData_GetSelectionStateByte(void);
void RallyData_SetSelectionStateByte(char param1);
void RallyData_ClearDriverGroupRecords(void);
void RallyData_PickOpponentLineups(void);

// Re-initialises the weather/damage/car setup when the country or stage index
// changed, unless the race is already running.
// FUNCTION: CMR2 0x00500c00
void OptionMenu_RefreshStageSetup(void)
{
    short b;
    short c;

    b = (BYTE)RallyDataCountryIndex();
    c = (char)RallyData_GetSelectionStateByte();
    if (b != c) {
        puts(g_strNeedReinitWeather);
        if (CGameInfo::GetConfiguredGameMode() != 2 && CGameInfo::GetConfiguredGameMode() != 8 &&
            CGameInfo::GetConfiguredGameMode() != 9 && CGameInfo::GetConfiguredGameMode() != 0xa &&
            CGameInfo::GetConfiguredGameMode() != 0xb && CGameInfo::GetConfiguredGameMode() != 0xc)
            RallyData_PickOpponentLineups();
        RallyData_CopyCountryPlayerDefaults();
        RallyData_ClearDriverGroupRecords();
        RallyData_SetSelectionStateByte((char)b);
    }
}

// Rebuilds the options menu (record list plus the owner's help text).
// FUNCTION: CMR2 0x00502310
void OptionMenu_RebuildOptions(void)
{
    Menu_Init((Menu *)g_unk0x0082b668, 0, -1, 0, (Menu *)g_unk0x0082b668, NULL, 1, 0, 0);
    Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0x48, (BYTE)CGameInfo::GetPreviewLayoutMode(), 0, 0, 0,
                      (int)OptionMenu_ResetValueAnimation, 0);;
    Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0x47, 7, 0, 1, 0, (int)OptionMenu_AdvanceSelectedOption, 1);;
    Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0x46, 0xb, 0, 1, 0, (int)OptionMenu_ApplyHighlightedValue, 2);;
    Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0x3c, 1, 0, 0, 0, (int)OptionMenu_RestartCountdown, 3);;
    if (CGameInfo::GetConfiguredGameMode() == 0)
        Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0xd3, 2, 0, 0, 0, (int)OptionMenu_LoadGameItem, 4);;
    Menu_AddItemType3((Menu *)g_unk0x0082b668, 0, 0x88, 2, 1, 1, 0, (int)OptionMenu_RestartEmptyItemRecords, 5);;
    Menu_SetCallbacks((Menu *)g_unk0x0082b668, (MenuCallback)OptionMenu_ConsumeRefreshRequest, (MenuCallback)OptionMenu_UpdateMessagesAndRecordStates,
                      (MenuCallback)OptionMenu_DispatchRecordEditState, (MenuCallback)OptionMenu_IncrementSharedValue);
    Menu_ValidateCursor((Menu *)g_unk0x0082b668, 0);
    OptionMenu_ResetRecords();
    OptionMenu_LoadPartTextures();
    OptionPreview_UpdateAllSlots();
    Menu_SetFlags((Menu *)g_unk0x0082b668, 0, 0, 0, 0);
}

// Builds the in-race option menus and switches the active menu to the first one.
// FUNCTION: CMR2 0x0050f180
void OptionMenu_BuildAndActivatePages(void)
{
    Menu_QueueDefaultAction();
    OptionMenu_RebuildOptions();
    OptionMenu_BuildValueSliders();
    OptionMenu_BuildControlSetup();
    OptionMenu_BuildStatusMenu();
    OptionMenu_BuildAdvancedOptions();
    g_pMenu0x00831778 = NULL;
    g_pMenu0x0083177c = (Menu *)OptionMenu_GetControlSetupMenu();
}


// FUNCTION: CMR2 0x00404ef0
void InRaceMenu_Close(void)
{
    CGameInfo::SetInputAndGamePaused(0);
    View_SetShake(g_unk0x0052af58[1], 0);
    CGameInfo::m_unk0x0052af40 = 0;
}

// Callback 0 of the camera options menu: turns the current values into the
// four slider positions.
// FUNCTION: CMR2 0x00403110
void InRaceMenu_FillCameraOffsetSliders(Menu *pMenu)
{
    int distance = FixDiv((pMenu->items[2].min - 1) * (g_unk0x0052aa5c - 0x80000), 0x80000);
    int angle = FixDiv((pMenu->items[3].min - 1) * g_unk0x0052a86c, 0x11c);
    int height = FixDiv((pMenu->items[0].min - 1) * (g_unk0x0052aa54 - 0x13333), 0xcccd);
    int depth = FixDiv((pMenu->items[1].min - 1) * (g_unk0x0052aa58 - 0x50000), 0x50000);
    pMenu->items[2].max = (BYTE)((distance + 0x8000) >> 16);
    pMenu->items[3].max = (BYTE)((angle + 0x8000) >> 16);
    pMenu->items[0].max = (BYTE)((height + 0x8000) >> 16);
    pMenu->items[1].max = (BYTE)((depth + 0x8000) >> 16);
}

// GLOBAL: CMR2 0x005160a0
char g_strCrtnaTga[] = "%s\\game\\menus\\crtna640.tga";
// GLOBAL: CMR2 0x005160bc
char g_strRoCxTga[] = "%s\\game\\menus\\RoCx_640.tga";
// GLOBAL: CMR2 0x005160d8
char g_strAr640DTga[] = "%s\\game\\menus\\Ar_640D.tga";
// GLOBAL: CMR2 0x005160f4
char g_strAr640ATga[] = "%s\\game\\menus\\Ar_640A.tga";

#include "Texture.h"
#include "StageTiming.h"

void InRaceMenu_ApplyDefaultCarSetup(Menu *pMenu, int param);

// Loads the in-race menu textures (arrows, curtain and, in championships,
// the round box).
// FUNCTION: CMR2 0x004054f0
void InRaceMenu_LoadTextures(void)
{
    bool loaded;

    sprintf(CFrontend::m_stringDest, g_strAr640ATga, CInstallInfo::GetGameCDPath());
    g_unk0x0052aa60 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest,
                                                     &loaded, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strAr640DTga, CInstallInfo::GetGameCDPath());
    g_unk0x0052aa68 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest,
                                                     &loaded, 0, 0, 0);
    if (CGameInfo::GetConfiguredGameMode() == 4) {
        sprintf(CFrontend::m_stringDest, g_strRoCxTga, CInstallInfo::GetGameCDPath());
        g_unk0x0052af4c = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                         CFrontend::m_stringDest, &loaded, 0, 0, 0);
    }
    sprintf(CFrontend::m_stringDest, g_strCrtnaTga, CInstallInfo::GetGameCDPath());
    g_unk0x0052af50 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest,
                                                     &loaded, 0, 0, 0);
}

// Driver position record the camera menu started with.
// GLOBAL: CMR2 0x00529900
int g_unk0x00529900[5];
// GLOBAL: CMR2 0x0052a858
int g_unk0x0052a858[5];

void HudDash_GetCameraUpVector(FixVector *pOut, BYTE index);
int HudDash_GetPlayerGaugeValue(BYTE index);
short HudDash_GetGaugeTarget(BYTE index);
BYTE *RallyData_GetDriverTyreChoiceRecord(BYTE index);

// Callback 0 of the camera options menu: remembers the current camera and
// shows it on the sliders.
// FUNCTION: CMR2 0x00403700
void InRaceMenu_EnterCameraOffsetOptions(Menu *pMenu, char unused)
{
    BYTE index;

    memcpy(g_unk0x00529900, RallyData_GetDriverTyreChoiceRecord(StageUI_GetRaceEndEventCount() + g_unk0x0052af58[1]), sizeof(g_unk0x00529900));
    index = g_unk0x0052af58[1];
    memcpy(g_unk0x0052a858, g_unk0x00529900, sizeof(g_unk0x0052a858));
    HudDash_GetCameraUpVector(&g_unk0x0052af60, index);
    g_unk0x00529914 = HudDash_GetPlayerGaugeValue(g_unk0x0052af58[1]);
    g_unk0x0052a48c = HudDash_GetGaugeTarget(g_unk0x0052af58[1]);
    g_unk0x0052aa50.x = g_unk0x0052af60.x;
    g_unk0x0052aa54 = g_unk0x0052af60.y;
    g_unk0x0052aa58 = g_unk0x0052af60.z;
    g_unk0x0052aa5c = g_unk0x00529914;
    g_unk0x0052a86c = g_unk0x0052a48c;
    InRaceMenu_FillCameraOffsetSliders(pMenu);
    g_unk0x0052ad54 = 1;
}

// Item callback of the camera menu "default" item.
// FUNCTION: CMR2 0x004036c0
void InRaceMenu_ResetCameraDefaults(Menu *pMenu, char unused)
{
    g_unk0x0052aa5c = 0xe0000;
    g_unk0x0052a86c = 0x2d;
    g_unk0x0052aa54 = 0x18000;
    g_unk0x0052aa58 = 0x68000;
    InRaceMenu_FillCameraOffsetSliders(pMenu);
}

// ===== Agent3 batch 1: GameInfo option records (>= 0x4f0000) =====

struct Unk0x0052ebc0;
extern struct Unk0x0052ebc0 *RallyData_GetDriverGroupRecord(int index);

// Draws the four one pixel edges of the option record's box (x, y, w, h);
// the vertical edges start one pixel inside the horizontal ones.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0050cb30
void OptionMenu_DrawBoxEdges(short *pRect, BYTE *pColour)
{
    short edge[4];

    edge[0] = pRect[0];
    edge[2] = pRect[2];
    edge[3] = 1;
    edge[1] = pRect[1];
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[1] = pRect[3] + pRect[1] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[1] = pRect[1];
    edge[3] = pRect[3];
    edge[2] = 1;
    edge[0] = pRect[0];
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[0] = pRect[2] + pRect[0] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
}

// Returns the byte at column type of the 7-byte option record index,
// sign extended; 0 for an unknown column.
// FUNCTION: CMR2 0x00502c60
int OptionMenu_GetRawWorkingValue(int index, int type)
{
    switch (type) {
    case 0: return g_unk0x0082bf04[index * 7];
    case 1: return g_unk0x0082bf04[index * 7 + 1];
    case 6: return g_unk0x0082bf04[index * 7 + 6];
    case 2: return g_unk0x0082bf04[index * 7 + 2];
    case 3: return g_unk0x0082bf04[index * 7 + 3];
    case 4: return g_unk0x0082bf04[index * 7 + 4];
    case 5: return g_unk0x0082bf04[index * 7 + 5];
    }
    return 0;
}

// Eases the option record's bar towards its target: while the record is faded
// out (mode 0) both bar sizes are cleared; in mode 1 the current fraction is
// scaled by field_0x0 when direction is set, otherwise the remaining distance.
// FUNCTION: CMR2 0x00501d50
void OptionMenu_EaseRecordBar(int index, short *pBar, int direction, int unused)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];

    if (p->field_0xc == 1) {
        if (direction != 0) {
            pBar[2] = (short)FixMulShift32(pBar[2] << 16, p->field_0x0);
            return;
        }
        pBar[3] = (short)FixMulShift32(pBar[3] << 16, p->field_0x0);
        return;
    }
    if (p->field_0xc == 0) {
        pBar[3] = 0;
        pBar[2] = 0;
    }
}

// Returns the byte at column type of the 7-byte option record index; the
// signed columns are divided by 10.
// FUNCTION: CMR2 0x005028a0
BYTE OptionMenu_GetScaledWorkingValue(int index, int type)
{
    switch (type) {
    case 0: return (BYTE)g_unk0x0082bee8[index][0];
    case 1: return (BYTE)g_unk0x0082bee8[index][1];
    case 6: return (char)g_unk0x0082bee8[index][6] / 10;
    case 2: return (char)g_unk0x0082bee8[index][2] / 10;
    case 3: return (char)g_unk0x0082bee8[index][3] / 10;
    case 4: return (char)g_unk0x0082bee8[index][4] / 10;
    case 5: return (char)g_unk0x0082bee8[index][5] / 10;
    }
    return 0;
}

// Index of the first option slot that is not a plain list entry, or 5 when
// the slot at index 4 holds one; 0 when there is none.
// FUNCTION: CMR2 0x004ff550
int OptionMenu_FindFirstDetailedSlot(void)
{
    int i;
    int found;

    found = 0;
    for (i = 0; i < (char)OptionMenu_GetControlSetupMenu()[6]; i++) {
        if (g_unk0x0082a90c[i] != 3) {
            found = 1;
            break;
        }
    }
    if (found) {
        if (CGameInfo::GetConfiguredGameMode() != 0) {
            if (i == 4)
                i = 5;
        }
        return i;
    }
    return 0;
}

// Draws the option record's text: mode 2 draws it whole; in mode 1 the part
// inside the 16.16 fraction of its length is drawn with the first font and
// the remainder with the second one, at that width.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x005020a0
void OptionMenu_DrawTransitionText(int index, int font1, int font2, char *text, short x, short y,
                  int *pColour1, int *pColour2, unsigned int flags)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];
    int len;
    int count;
    int width;
    int i;
    int j;

    if (p->field_0xc == 2) {
        Font_DrawText(font1, text, x, y, pColour1, flags);
        return;
    }
    if (p->field_0xc == 1) {
        len = (int)strlen(text);
        count = FixMulShift32(len << 16, p->field_0x0);
        for (i = 0; i < count; i++)
            g_unk0x0082b1c0[i] = text[i];
        g_unk0x0082b1c0[count] = 0;
        Font_DrawText(font1, g_unk0x0082b1c0, x, y, pColour1, flags);
        if (count < len) {
            x += Font_GetTextWidth(font1, (BYTE *)g_unk0x0082b1c0);
            for (i = count; i < len; i++)
                g_unk0x0082b1c0[i - count] = text[i];
            g_unk0x0082b1c0[len - count] = 0;
            Font_DrawText(font2, g_unk0x0082b1c0, x, y, pColour2, flags);
        }
    }
}

// Option record value as a percentage 0..100: picks the byte selected by
// type, letting a byte that follows it win when it is larger, and a later
// byte win when the value is still smaller.
// FUNCTION: CMR2 0x00502df0
unsigned int OptionMenu_GetRecordPercentage(int index, int type, int dynamic)
{
    BYTE *p;
    int value;
    int other;

    value = 0;
    if (dynamic != 0)
        p = (BYTE *)RallyData_GetDriverGroupRecord(index);
    else
        p = g_unk0x0082c070 + index * 0x148;
    switch (type) {
    case 1:
        value = p[0x116];
        break;
    case 2:
        value = p[0x117];
        break;
    case 3:
        value = p[0x10e];
        if (p[0x10f] > value)
            value = p[0x10f];
        if (p[0x110] > value)
            value = p[0x110];
        other = p[0x111];
        if (other > value)
            value = other;
        break;
    case 4:
        value = p[0x118];
        break;
    case 5:
        value = p[0x112];
        if (p[0x113] > value)
            value = p[0x113];
        if (p[0x114] > value)
            value = p[0x114];
        other = p[0x115];
        if (other > value)
            value = other;
        break;
    case 6:
        value = p[0x119];
        break;
    case 7:
        value = p[0x10c];
        other = p[0x10d];
        if (other > value)
            value = other;
        break;
    case 8:
        value = p[0x11f];
        break;
    case 9:
        value = p[0x120];
        break;
    case 10:
        value = p[0x121];
        break;
    case 11:
        value = p[0x108];
        if (p[0x109] > value)
            value = p[0x109];
        if (p[0x10a] > value)
            value = p[0x10a];
        other = p[0x10b];
        if (other > value)
            value = other;
        break;
    }
    other = (value * 100) / 0xff;
    if (other == 0) {
        if (value != 0)
            return 1;
    } else if (other > 100) {
        other = 100;
    }
    return other;
}

// Scales both bar sizes of the option record by field_0x0 while it fades in
// (mode 1), or clears them when it is fully out (mode 0).
// FUNCTION: CMR2 0x00501f00
void OptionMenu_ScaleRecordBar(int index, short *pBar)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];

    if (p->field_0xc == 1) {
        pBar[2] = (short)(FixMul(pBar[2] << 16, p->field_0x0) >> 16);
        pBar[3] = (short)(FixMul(pBar[3] << 16, p->field_0x0) >> 16);
        return;
    }
    if (p->field_0xc == 0) {
        pBar[3] = 0;
        pBar[2] = 0;
    }
}

// Address of the byte set by the option menu (0x82b1b8).
// FUNCTION: CMR2 0x00501ab0
BYTE *OptionMenu_GetStateByteAddress(void)
{
    return g_unk0x0082b1b8;
}

// Advances the option menu's overlay pulse: a sine running over a minute is
// mapped to a grey level and stored in the fade colour bytes.
// FUNCTION: CMR2 0x00501ac0
void OptionMenu_UpdateOverlayPulse(void)
{
    int value;
    BYTE shade;

    value = CMain::GetFrameDelta() % 0x3c;
    value = FixDiv(value << 16, 0x3c0000);
    value = FixMul(value, 0x1680000);
    value = g_sinTable[(unsigned short)(__int64)(value * g_unk0x00511300) & 0xfff] + 0x10000;
    value = FixMul(value, 0x8000);
    if (value < 0)
        value = 0;
    else if (value > 0x10000)
        value = 0x10000;
    shade = (BYTE)((FixMul(value, 0x7f0000) + 0x800000) >> 16);
    g_unk0x0082b1bb = 0xff;
    g_unk0x0082b1ba = shade;
    g_unk0x0082b1b9 = shade;
    g_unk0x0082b1b8[0] = shade;
}

// Advances the 16.16 transition value of every option record: records that
// passed their duration switch to mode 2 at full scale, the rest receive the
// proportion of the elapsed time squared in mode 1 or square-rooted in mode 2.
// FUNCTION: CMR2 0x00501b90
void OptionMenu_UpdateRecordTransitions(void)
{
    int i;

    for (i = 0; i < g_unk0x0082b1bc; i++) {
        int elapsed;

        if (g_unk0x0082b2c0[i].field_0xc != 1)
            continue;
        elapsed = CMain::GetFrameDelta() - g_unk0x0082b2c0[i].field_0x8;
        if (elapsed >= g_unk0x0082b2c0[i].field_0x4) {
            g_unk0x0082b2c0[i].field_0xc = 2;
            g_unk0x0082b2c0[i].field_0x0 = 0x10000;
        } else {
            int value = FixDiv(elapsed << 16, g_unk0x0082b2c0[i].field_0x4 << 16);
            int kind = g_unk0x0082b2c0[i].field_0x10;
            g_unk0x0082b2c0[i].field_0x0 = value;
            switch (kind) {
            case 2:
                g_unk0x0082b2c0[i].field_0x0 = FixSqrt(g_unk0x0082b2c0[i].field_0x0);
                break;
            case 1:
                g_unk0x0082b2c0[i].field_0x0 =
                    FixMul(g_unk0x0082b2c0[i].field_0x0, g_unk0x0082b2c0[i].field_0x0);
                break;
            }
        }
    }
    OptionMenu_UpdateOverlayPulse();
}

// True when the option slot is enabled: mode 2/4 are checked against their
// selectors, everything else is accepted.
// FUNCTION: CMR2 0x00501280
int OptionMenu_IsSlotEnabled(int mode, int index)
{
    int result = 1;

    if (mode == 2) {
        if (CFrontend::GetArchiveSecondaryFlagEntry(index) == 0)
            result = 0;
    } else if (mode == 4) {
        if (CFrontend::GetArchivePrimaryFlagEntry(index) == 0)
            result = 0;
    }
    return result;
}

// Whether the option slot has been raised above its base value.
// FUNCTION: CMR2 0x00502630
int OptionMenu_IsSlotValueAboveBase(int index, int type)
{
    int value;

    value = OptionMenu_GetSlotSelector(index);
    if (g_unk0x00527098[type] <= value)
        return 1;
    return g_unk0x0082bf20[index][type] != 0;
}

// Whether the option slot value equals its default, per column.
// FUNCTION: CMR2 0x00502b10
BYTE OptionMenu_IsValueDefault(int index, int type)
{
    switch (type) {
    case 0: return g_unk0x0082bf04[index * 7] == (char)g_unk0x0082bee8[index][0];
    case 1: return g_unk0x0082bf04[index * 7 + 1] == (char)g_unk0x0082bee8[index][1];
    case 6: return g_unk0x0082bf04[index * 7 + 6] == (char)g_unk0x0082bee8[index][6];
    case 2: return g_unk0x0082bf04[index * 7 + 2] == (char)g_unk0x0082bee8[index][2];
    case 3: return g_unk0x0082bf04[index * 7 + 3] == (char)g_unk0x0082bee8[index][3];
    case 4: return g_unk0x0082bf04[index * 7 + 4] == (char)g_unk0x0082bee8[index][4];
    case 5: return g_unk0x0082bf04[index * 7 + 5] == (char)g_unk0x0082bee8[index][5];
    }
    return 0;
}

// Whether the option slot value has risen above its base value.
// FUNCTION: CMR2 0x00502fc0
int OptionMenu_IsRecordPercentageAboveBase(int index, int type)
{
    int value;

    value = OptionMenu_GetSlotSelector(index);
    if (g_unk0x005270b4[type] <= value)
        return OptionMenu_GetRecordPercentage(index, type, 1) != 0;
    return g_unk0x0082c040[index][type] != 0;
}

// Finds the index of the entry with the given id in the list at 0x3c, or -1.
// FUNCTION: CMR2 0x00508f60
int OptionPreview_FindMeshId(unsigned int id, int param2)
{
    int *pEntry;
    int count;
    int i;

    count = *(BYTE *)(param2 + 0x26a);
    i = 0;
    while (i < count) {
        if ((*(unsigned int *)(*(int *)(param2 + 0x3c + i * 4) + 0x30) & 0xff) == id)
            return i;
        i++;
    }
    return -1;
}

// Screen-space viewport used by the stage HUD: x, y, width, height.
// GLOBAL: CMR2 0x0082c9ec
short g_unk0x0082c9ec[4];

// Maps a HUD point from viewport space to screen space; when the transform is
// marked as already scaled (field_0x1c == 0x10000) it only resolves the
// 640x480 reference to the current resolution.
// FUNCTION: CMR2 0x00503b70
int OptionMenu_TransformHudPointToScreen(Unk0x0082c6c8 *p, short *pX, short *pY)
{
    int value;

    if (p->field_0x1c != 0x10000) {
        value = (*pX - g_unk0x0082c9ec[0]) << 16;
        value = FixDiv(value, g_unk0x0082c9ec[2] << 16);
        *pX = (FixMul(value, *(short *)((BYTE *)p + 0x10) << 16) >> 16) + *(short *)((BYTE *)p + 0xc);
        value = (*pY - g_unk0x0082c9ec[1]) << 16;
        value = FixDiv(value, g_unk0x0082c9ec[3] << 16);
        *pY = (FixMul(value, *(short *)((BYTE *)p + 0x12) << 16) >> 16) + *(short *)((BYTE *)p + 0xe);
    }
    *pX = (short)((*pX * (int)g_pGraphics->resX) / 0x280);
    *pY = (short)((*pY * (int)g_pGraphics->resY) / 0x1e0);
}

// Re-runs the option-menu callback stored in the global (0x82b1b4).
// FUNCTION: CMR2 0x00501390
BYTE OptionMenu_UpdateBackgroundRoot(void)
{
    SceneNode_Destroy((SceneNode *)g_unk0x0082b1b4);
    return 1;
}

// Option menu sound names (7 bytes each) and the file name they are loaded
// from, plus the volume/index state of the option menu.
// GLOBAL: CMR2 0x005296c4
char g_unk0x005296c4[5][7] = {"move", "select", "back", "error", "toggle"};
// GLOBAL: CMR2 0x005296e8
char g_str0x005296e8[16] = "%s\\menu\\%s.wav";
// GLOBAL: CMR2 0x00831888
int g_unk0x00831888;
// GLOBAL: CMR2 0x0083188c
int g_unk0x0083188c;

// Loads the five option menu sounds. Returns 0 if any of them failed to load.
// FUNCTION: CMR2 0x0050f3c0
BYTE OptionMenu_LoadSounds(void)
{
    BYTE result;
    char *name;
    int nameEnd;

    result = 1;
    g_unk0x00831888 = 0;
    name = (char *)g_unk0x005296c4;
    nameEnd = (int)g_unk0x005296c4 + sizeof(g_unk0x005296c4);
    do {
        sprintf(CFrontend::m_stringDest, g_str0x005296e8, CInstallInfo::GetSoundsDir(), name);
        if (Sound_LoadSample(CFrontend::m_stringDest, 0, (GenericFile *)OptionMenu_GetCommonArchive()) == 0)
            result = 0;
        name += 7;
    } while ((int)name < nameEnd);
    return result;
}

// Sets the option menu volume and hands the menu sound handles to the input
// system.
// FUNCTION: CMR2 0x0050f420
void OptionMenu_ConfigureSoundHandles(void)
{
    g_unk0x0083188c = -1;
    // option menu volume: the stored 0..0x7f setting scaled to 16.16 and quartered
    CInput::SetInputRepeatTimingState((((int)CGameInfo::GetEffectsSoundVolume() << 16) / 100) / 4);
    CInput::SetInputRepeatTimingParameters(g_unk0x00831888, g_unk0x00831888 + 1, g_unk0x00831888 + 2,
                         g_unk0x00831888 + 3, g_unk0x00831888 + 4);
    Menu_SetInputStateFlag(1);
}

// Option menu archives: every region uses the language directories of its own
// languages (region * 5 + language), indexed like the country codes.
// GLOBAL: CMR2 0x00529794
char g_str0x00529794[20] = "%s\\%d\\Common.bfl";
// GLOBAL: CMR2 0x0052976c
char g_str0x0052976c[20] = "%s\\%d\\%sDay%d.bfl";
// GLOBAL: CMR2 0x00529780
char g_str0x00529780[20] = "%s\\%d\\%sDay%dC.bfl";
// GLOBAL: CMR2 0x0051a100
char g_str0x0051a100[12] = "%s%s%s.bfl";

// Country code of each RallyDataCountryIndex value.
// GLOBAL: CMR2 0x00519260
char g_str0x00519260[4] = "JAP";
// GLOBAL: CMR2 0x00519268
char g_str0x00519268[4] = "ITA";
// GLOBAL: CMR2 0x0051926c
char g_str0x0051926c[4] = "KEN";
// GLOBAL: CMR2 0x00519270
char g_str0x00519270[4] = "AUS";
// GLOBAL: CMR2 0x00519274
char g_str0x00519274[4] = "SWE";
// GLOBAL: CMR2 0x00519278
char g_str0x00519278[4] = "FRA";
// GLOBAL: CMR2 0x0051927c
char g_str0x0051927c[4] = "GRE";
// GLOBAL: CMR2 0x00519280
char g_str0x00519280[4] = "FIN";
// GLOBAL: CMR2 0x005296f8
char *g_unk0x005296f8[9] = {g_str0x00519280, g_str0x0051927c, g_str0x00519278,
                            g_str0x00519274, g_str0x00519270, g_str0x0051926c,
                            g_str0x00519268, CFrontend::m_strUK, NULL};

// Language text directory of every region and language (region * 5 + language).
// Countries without a language of their own keep an empty entry.
// GLOBAL: CMR2 0x0051a00c
char g_str0x0051a00c[12] = "polishtext";
// GLOBAL: CMR2 0x0051a018
char g_str0x0051a018[12] = "engusatext";
// GLOBAL: CMR2 0x0051a024
char g_str0x0051a024[12] = "germantext";
// GLOBAL: CMR2 0x0051a030
char g_str0x0051a030[12] = "italiantext";
// GLOBAL: CMR2 0x0051a03c
char g_str0x0051a03c[12] = "spanishtext";
// GLOBAL: CMR2 0x0051a048
char g_str0x0051a048[12] = "frenchtext";
// GLOBAL: CMR2 0x0051a054
char g_str0x0051a054[12] = "englishtext";
// GLOBAL: CMR2 0x0052971c
char *g_unk0x0052971c[20] = {
    g_str0x0051a054, g_str0x0051a048, g_str0x0051a03c, g_str0x0051a030, g_str0x0051a024,
    g_str0x0051a018, g_str0x0051a048, g_str0x0051a03c, NULL, NULL,
    g_str0x0051a054, NULL, NULL, NULL, NULL,
    g_str0x0051a00c, NULL, NULL, NULL, NULL};

// Loads the three option menu archives: the common one, the day file of the
// stage and the region/language file.
// FUNCTION: CMR2 0x0050f4f0
void OptionMenu_LoadArchives(void)
{
    int resolution;
    int stage;

    stage = RallyDataStageIndex() >> 2;
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::IsTextureWidthSupported(0x400) &&
        CFrontend::IsTextureHeightSupported(0x400))
        resolution = 0x400;
    else
        resolution = 0x280;

    sprintf(CFrontend::m_stringDest, g_str0x00529794, CInstallInfo::GetSetupRepDir(), resolution);
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest);

    if (CFrontend::HasDXT1TextureSupport()) {
        sprintf(CFrontend::m_stringDest, g_str0x00529780, CInstallInfo::GetSetupRepDir(), resolution,
                g_unk0x005296f8[RallyDataCountryIndex() & 0xff], stage + 1);
    } else {
        sprintf(CFrontend::m_stringDest, g_str0x0052976c, CInstallInfo::GetSetupRepDir(), resolution,
                g_unk0x005296f8[RallyDataCountryIndex() & 0xff], stage + 1);
    }
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)OptionMenu_GetStageArchive(), CFrontend::m_stringDest);

    sprintf(CFrontend::m_stringDest, g_str0x0051a100, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(),
            g_unk0x0052971c[CGameInfo::GetGameRegion() * 5 + CGameInfo::GetGameLanguage()]);
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)OptionMenu_GetLanguageArchive(), CFrontend::m_stringDest);

    CGame::RegisterCallback(OptionMenu_ReleaseTextures, NULL);
}

// Format string printed when the option menu's background world is created.
// GLOBAL: CMR2 0x00527050
char g_str0x00527050[] = "*** WORLD CREATED: 0x%08X ***\n";
// Camera node of the option menu's background world (0x82b1b0).
// GLOBAL: CMR2 0x0082b1b0
int g_unk0x0082b1b0;

void Scene_SetAmbient(BYTE *pColour, int boost);
SceneNode *Scene_CreateLight(int type, int r, int g, int b, FixVector *pPosition, FixAngles *pAngles, SceneNode *pParent);
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);

// Builds the option menu's background world: root node, camera, ambient and key
// light, then registers the release callback.
// FUNCTION: CMR2 0x005013a0
void OptionMenu_CreateBackgroundWorld(void)
{
    SceneNode *pNode;
    BYTE colour[4];
    FixAngles angles;
    short view[4];
    FixVector translation;
    FixVector lightPosition;
    int i;

    view[0] = 0;
    view[1] = 0;
    view[2] = (short)((int)g_pGraphics->resX * 2 / 3);
    view[3] = (short)((int)g_pGraphics->resY * 2 / 3);
    translation.x = 0x23d7;
    translation.y = 0xffffe8f6;
    translation.z = 0xfffb0000;
    lightPosition.x = 0x50000;
    lightPosition.y = 0x50000;
    lightPosition.z = 0xfffb0000;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0;
    angles.pad = 0;
    colour[0] = 0xc8;
    colour[1] = 0xc8;
    colour[2] = 0xc8;
    colour[3] = 0xff;
    g_unk0x0082b1b4 = (int)SceneNode_CreateRoot();
    sprintf(CFrontend::m_stringDest, g_str0x00527050, g_unk0x0082b1b4);
    puts(CFrontend::m_stringDest);
    g_unk0x0082b1b0 = (int)SceneType2_Create(&translation, &angles, NULL, (SceneNode *)g_unk0x0082b1b4);
    for (pNode = (SceneNode *)g_unk0x0082b1b4; pNode != NULL; pNode = pNode->pParent)
        pNode->dirty = 1;
    Scene_SetAmbient(colour, 0);
    Scene_CreateLight(2, 0x10000, 0x10000, 0x10000, &lightPosition, &angles, (SceneNode *)g_unk0x0082b1b4);
    CGraphics::SetProjection(0x30978, 0x4326e, 0xfa0000, 0x10000);
    Game_PrepareScene((SceneNode *)g_unk0x0082b1b4, (SceneNode *)g_unk0x0082b1b0, (int)view, 0);
    CGame::RegisterCallback(OptionMenu_UpdateBackgroundRoot, NULL);
}
// Releases the three option menu textures and clears their handles.
// FUNCTION: CMR2 0x0050f480
int OptionMenu_ReleaseTextures(void)
{
    if (g_unk0x00831a90[0] != 0) {
        CFileBuffer::FreeGenericFileBuffer((void *)g_unk0x00831a90[0]);
        g_unk0x00831a90[0] = 0;
    }
    g_unk0x00831a90[2] = 0;
    g_unk0x00831a90[1] = 0;
    if (g_unk0x00831aa0[0] != 0) {
        CFileBuffer::FreeGenericFileBuffer((void *)g_unk0x00831aa0[0]);
        g_unk0x00831aa0[0] = 0;
    }
    g_unk0x00831aa0[2] = 0;
    g_unk0x00831aa0[1] = 0;
    if (g_unk0x00831ab0[0] != 0) {
        CFileBuffer::FreeGenericFileBuffer((void *)g_unk0x00831ab0[0]);
        g_unk0x00831ab0[0] = 0;
    }
    g_unk0x00831ab0[2] = 0;
    g_unk0x00831ab0[1] = 0;
    return 1;
}

// Fills the 0x23c vector of the option record with the 16.16 fractions of its
// four bytes at +0x108.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00509d30
void OptionPreview_UnpackColourFractions(int index)
{
    Unk0x0082d220 *pRec;
    BYTE *p;
    int i;

    pRec = &g_unk0x0082d220[index];
    p = (BYTE *)RallyData_GetDriverGroupRecord(index);
    for (i = 0; i < 4; i++)
        pRec->field_0x23c.v[i] = FixDiv(p[0x108 + i] << 16, 0xff0000);
}

// Sets the fade/shape values of the option record's sky colours (field_0x22c).
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00509be0
void OptionPreview_UpdateSkyFadeShape(int index)
{
    Unk0x0082d220 *pRec;
    BYTE *p;
    int i;

    pRec = &g_unk0x0082d220[index];
    p = (BYTE *)RallyData_GetDriverGroupRecord(index);
    for (i = 0; i < 4; i++)
        pRec->field_0x22c.v[i] = FixDiv(p[0x10e + i] << 16, 0xff0000);
    pRec->field_0x22c.v[0] = FixMul(pRec->field_0x22c.v[0], FixMul(0x3333, 0xffff0000));
    pRec->field_0x22c.v[1] = FixMul(pRec->field_0x22c.v[1], FixMul(0x3333, 0xffff0000));
    pRec->field_0x22c.v[2] = FixMul(pRec->field_0x22c.v[2], FixMul(0x3333, 0x8000));
    pRec->field_0x22c.v[3] = FixMul(pRec->field_0x22c.v[3], FixMul(0x3333, 0xffff8000));
}

// Applies the option menu's fade to the stage meshes of the given category:
// each mesh in the record's list takes the target value for its bit.
void Game_SetTriangleField2CByGroup(Mesh *pMesh, int mask, int value);
void Game_SetTriangleField30ByGroup(Mesh *pMesh, int mask, int value);
// FUNCTION: CMR2 0x00508fa0
void OptionPreview_SetMeshRenderFlags(int index, int param2, BYTE param3){
    BYTE *pRecord = (BYTE *)&g_unk0x0082d220[index];
    int target;
    int mask;
    int i;

    switch (param2) {
    case 0:
        target = 0;
        mask = 1;
        break;
    case 1:
        target = 3;
        mask = 4;
        break;
    default:
        if (param3 == 4 || param3 == 5) {
            target = 3;
            mask = 4;
        } else {
            target = 5;
            mask = 7;
        }
        break;
    }
    switch (param3) {
    case 0:
        i = OptionPreview_FindMeshId(7, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 0x100, target);
            Game_SetTriangleField30ByGroup(*(Mesh **)(pRecord + i * 4), 0x100, mask);
            return;
        }
        break;
    case 1:
        i = OptionPreview_FindMeshId(0xc, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 0x20, target);
            Game_SetTriangleField30ByGroup(*(Mesh **)(pRecord + i * 4), 0x20, mask);
        }
        i = OptionPreview_FindMeshId(7, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 0x20, target);
            Game_SetTriangleField30ByGroup(*(Mesh **)(pRecord + i * 4), 0x20, mask);
            return;
        }
        break;
    case 2:
        i = OptionPreview_FindMeshId(7, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 0x40, target);
            Game_SetTriangleField30ByGroup(*(Mesh **)(pRecord + i * 4), 0x40, mask);
            return;
        }
        break;
    case 3:
        i = OptionPreview_FindMeshId(7, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 0x80, target);
            Game_SetTriangleField30ByGroup(*(Mesh **)(pRecord + i * 4), 0x80, mask);
            return;
        }
        break;
    case 4:
        i = OptionPreview_FindMeshId(0xe, (int)pRecord);
        if (i >= 0) {
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 4, target);
            return;
        }
        break;
    case 5:
        i = OptionPreview_FindMeshId(0xe, (int)pRecord);
        if (i >= 0)
            Game_SetTriangleField2CByGroup(*(Mesh **)(pRecord + i * 4), 8, target);
        break;
    }
}

// Chooses the highlight value for the selected option item and stores it at
// +0x1e, together with the option's current value at +0x1f.
// (CFrontend::FreeLocalizedTextStrings)

// GLOBAL: CMR2 0x00831880
int g_unk0x00831880;
// GLOBAL: CMR2 0x00831884
BYTE g_unk0x00831884;

// FUNCTION: CMR2 0x004ff5b0
void OptionMenu_FillValueSlider(void)
{
    BYTE *pMode;
    int index;
    unsigned int value;

    index = Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1);
    index *= 5;
    pMode = OptionMenu_GetControlSetupMenu();
    value = pMode[0x1f + index * 4];
    switch (value) {
    case 0:
        OptionMenu_GetValueSliderMenu()[0x1e] = 7;
        break;
    case 1:
        OptionMenu_GetValueSliderMenu()[0x1e] = 5;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        OptionMenu_GetValueSliderMenu()[0x1e] = 0xb;
        break;
    }
    OptionMenu_GetValueSliderMenu()[0x1f] = (BYTE)OptionMenu_GetScaledWorkingValue(CGameInfo::GetActiveOptionSlot(), value);
}

// Frees the option menu sound buffer and stops the streaming sound.
// FUNCTION: CMR2 0x0050f340
bool OptionMenu_ReleaseSoundBuffer(void)
{
    if (&g_unk0x00831880 != NULL) {
        if (g_unk0x00831884 == 0)
            CFileBuffer::FreeGenericFileBuffer((void *)g_unk0x00831880);
        g_unk0x00831880 = 0;
    }
    CFrontend::FreeLocalizedTextStrings();
    return 1;
}

// GLOBAL: CMR2 0x0082ac60
BYTE g_unk0x0082ac60;
// GLOBAL: CMR2 0x0082ac4c
int g_unk0x0082ac4c;
// GLOBAL: CMR2 0x0082ac50
int g_unk0x0082ac50;
// Defined in FrontendScreens.cpp
extern int g_unk0x0082a924;
extern int g_unk0x0082aa3c;
extern int g_unk0x0082aa40;
extern int g_unk0x0082ac48;
// GLOBAL: CMR2 0x0082ab44
BYTE g_unk0x0082ab44;
// GLOBAL: CMR2 0x00527380
int g_unk0x00527380[3] = { -11579569, -11579569, -11579569 };
// GLOBAL: CMR2 0x0052738c
int g_unk0x0052738c[3] = { -1, -2130706433, 1078939471 };
// Text of the option menu's status line (written elsewhere before it is drawn).
// GLOBAL: CMR2 0x0082a93c
char g_unk0x0082a93c[0x100];

// Requests a refresh of the option records.
// FUNCTION: CMR2 0x00502230
void OptionMenu_RequestRefresh(int unused, int unused2)
{
    g_unk0x0082ac60 = 1;
}

// GLOBAL: CMR2 0x0082bc08
BYTE g_unk0x0082bc08[0x1e0];

void OptionMenu_DrawStatusLine(int unused);
void OptionMenu_SelectItemAndRefresh(char *pItem, int unused);
void OptionMenu_LoadSelectedSavedGame(unsigned int, unsigned int);
void OptionMenu_DrawValueSelectionRows(unsigned int);
void OptionMenu_DrawRaisedRallyInfo(unsigned int);
void OptionMenu_IncrementSharedValue(Menu *pMenu, char param);
void OptionMenu_ApplyFirstModeItem(Menu *pMenu, int param);
void OptionMenu_DrawRallyInfoScreen(unsigned int);

// Builds the option menu's status line: the record-list refresh callback and
// the owner's help-text draw.
// FUNCTION: CMR2 0x00502240
void OptionMenu_BuildStatusMenu(void)
{
    Menu_Init((Menu *)g_unk0x0082ba28, 0, -1, 0, (Menu *)g_unk0x0082b668, NULL, 1, 0, 0);
    Menu_AddItemType1((Menu *)g_unk0x0082ba28, 0, -1, 0, -1);
    Menu_SetCallbacks((Menu *)g_unk0x0082ba28, (MenuCallback)OptionMenu_RequestRefresh, NULL, (MenuCallback)OptionMenu_DrawStatusLine, NULL);
    Menu_ValidateCursor((Menu *)g_unk0x0082ba28, 0);
}

// Builds the option menu's advanced-options list with its device and accept
// entries.
// FUNCTION: CMR2 0x005022a0
void OptionMenu_BuildAdvancedOptions(void)
{
    Menu_Init((Menu *)g_unk0x0082b488, 0, -1, 0, (Menu *)g_unk0x0082b668, NULL, 0, 0, 1);
    Menu_AddItemType1((Menu *)g_unk0x0082b488, 0, 0x100, (int)OptionMenu_LoadSelectedSavedGame, -1);
    Menu_AddItemType1((Menu *)g_unk0x0082b488, 0, 0x101, 0, -1);
    Menu_SetCallbacks((Menu *)g_unk0x0082b488, (MenuCallback)OptionMenu_SelectItemAndRefresh, NULL, (MenuCallback)OptionMenu_DrawValueSelectionRows, NULL);
    Menu_ValidateCursor((Menu *)g_unk0x0082b488, 0);
}

// Builds the option menu's control-setup screen.
// FUNCTION: CMR2 0x00502440
void OptionMenu_BuildControlSetup(void)
{
    Menu_Init((Menu *)g_unk0x0082bc08, 0, -1, 0, (Menu *)g_unk0x0082b668, NULL, 1, 0, 0);
    Menu_SetCallbacks((Menu *)g_unk0x0082bc08, NULL, (MenuCallback)RallyData_ValidateIndex, (MenuCallback)OptionMenu_DrawRaisedRallyInfo,
                      (MenuCallback)OptionMenu_IncrementSharedValue);
    Menu_ValidateCursor((Menu *)g_unk0x0082bc08, 0);
    Menu_SetFlags((Menu *)g_unk0x0082bc08, 0, 0, 0, 1);
}

// Builds the option menu's slider screen.
// FUNCTION: CMR2 0x005024a0
void OptionMenu_BuildValueSliders(void)
{
    Menu_Init((Menu *)g_unk0x0082b848, 0, -1, 0, (Menu *)g_unk0x0082b668, NULL, 1, 0, 1);
    Menu_AddItemType3((Menu *)g_unk0x0082b848, 0, -1, 0x65, 0, 0, 0, (int)OptionMenu_ApplyFirstModeItem, 0);;
    Menu_SetCallbacks((Menu *)g_unk0x0082b848, NULL, (MenuCallback)RallyData_ValidateIndex, (MenuCallback)OptionMenu_DrawRallyInfoScreen, NULL);
    Menu_ValidateCursor((Menu *)g_unk0x0082b848, 0);
}

// Marks the given item selected and requests a refresh.
// FUNCTION: CMR2 0x004ffa50
void OptionMenu_SelectItemAndRefresh(char *pItem, int unused)
{
    pItem[7] = 1;
    g_unk0x0082ac60 = 1;
}

// Restarts the option records and arms their countdown.
// FUNCTION: CMR2 0x00500110
void OptionMenu_RestartCountdown(int unused, int unused2)
{
    OptionMenu_FinishRecordTransitions();
    g_unk0x0082ac4c = 1;
}

// Rebuilds the option records once the refresh request is consumed.
// FUNCTION: CMR2 0x004ffa70
void OptionMenu_ConsumeRefreshRequest(int unused, int unused2)
{
    g_unk0x0082aa40 = SavedGames_GetCount();
    if (g_unk0x0082ac60 == 0) {
        g_unk0x0082ab44 = 0;
        g_unk0x0082a924 = 0;
        g_unk0x0082aa3c = 0;
        g_unk0x0082ac48 = 0;
        return;
    }
    g_unk0x0082ac60 = 0;
}

// Restarts the option records when the selected item has no value.
// FUNCTION: CMR2 0x00500190
void OptionMenu_RestartEmptyItemRecords(BYTE *pItem, int unused)
{
    int index;

    index = Menu_FindItem((Menu *)pItem, 5);
    if (pItem[index * 0x14 + 0x1f] == 0) {
        OptionMenu_FinishRecordTransitions();
        g_unk0x0082ac50 = 1;
    }
}

// Draws the option menu's status line centred on the screen.
// FUNCTION: CMR2 0x0050e740
void OptionMenu_DrawStatusLine(int unused)
{
    OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, g_unk0x0082a93c, (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                 g_unk0x00527380, g_unk0x0052738c, 0x12);
}

// Fills pRect with `count` vertical strips shading from colour 0 to colour 1
// over the first half and from colour 1 to colour 2 over the second half
// (pColours: three RGBA colours; count must be at least 2). The strip widths
// are rounded so that together they cover the rectangle exactly.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0050ee50
void OptionMenu_DrawThreeColourGradientStrips(short *pRect, int unused, unsigned int count, BYTE *pColours)
{
    BYTE colour[4];
    short strip[4];
    FixVector c0;
    FixVector c1;
    FixVector d01;
    FixVector d12;
    FixVector c;
    int r;
    int g;
    int b;
    int n;
    int half;
    int i;
    int t;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    *(int *)&strip[0] = *(int *)&pRect[0];
    *(int *)&strip[2] = *(int *)&pRect[2];
    c0.x = pColours[0] << 16;
    c0.y = pColours[1] << 16;
    c0.z = pColours[2] << 16;
    c1.x = pColours[4] << 16;
    c1.y = pColours[5] << 16;
    c1.z = pColours[6] << 16;
    d01.x = c1.x - c0.x;
    d01.y = c1.y - c0.y;
    d01.z = c1.z - c0.z;
    d12.x = (pColours[8] << 16) - c1.x;
    d12.y = (pColours[9] << 16) - c1.y;
    n = count & 0xff;
    d12.z = (pColours[10] << 16) - c1.z;
    half = (count & 0xff) >> 1;
    for (i = 0, t = 0; i < n; i++, t += 0x10000) {
        strip[2] = (FixMul(t + 0x10000, FixDiv(pRect[2] << 16, n << 16)) >> 16) + pRect[0];
        strip[2] -= strip[0];
        if (i > half) {
            FixVecScale(&c, &d12, FixDiv(t - (half << 16), half << 16));
            c.x += c1.x;
            c.y += c1.y;
            c.z += c1.z;
        } else {
            FixVecScale(&c, &d01, FixDiv(i << 16, half << 16));
            c.x += c0.x;
            c.y += c0.y;
            c.z += c0.z;
        }
        r = c.x >> 16;
        if (r > 0xff)
            r = 0xff;
        else if (r < 0)
            r = 0;
        g = c.y >> 16;
        colour[0] = r;
        if (g > 0xff)
            g = 0xff;
        else if (g < 0)
            g = 0;
        b = c.z >> 16;
        colour[1] = g;
        if (b > 0xff)
            b = 0xff;
        else if (b < 0)
            b = 0;
        colour[2] = b;
        Sprite_FillRect((int)g_pGraphics + 0x150, strip, colour, 3);
        strip[0] += strip[2];
    }
}

// GLOBAL: CMR2 0x0082b0a4
int g_unk0x0082b0a4;
// GLOBAL: CMR2 0x0082a938
BYTE g_unk0x0082a938;

BYTE *RallyData_GetDriverSkillRecord(int index);

// GLOBAL: CMR2 0x00529670
char g_strFontGeneralHel15pt[17] = "general\\hel_15pt";
// GLOBAL: CMR2 0x00529684
char g_strFontGeneralHel12pt[17] = "general\\hel_12pt";
// GLOBAL: CMR2 0x00529698
char g_strFontGeneralDot[12] = "general\\dot";
// GLOBAL: CMR2 0x005296ac
char g_strFontGeneralHandel[15] = "general\\handel";

void Frontend_AccumulateDeviceKeyCounters(void);
Unk0049c2c0 *Game_GetSecondaryCallbackMachine(void);
int Network_CloseSession(void);
void Game_RequestOptionRefresh(void);

// Starts the option menu: rebuilds the frontend, flags the option state, arms
// the fade timer and switches the grouped callback machine to level 2; when the
// menu is active and this is not the "restart" path it runs the transitions.
// FUNCTION: CMR2 0x005001c0
void OptionMenu_Start(char param1)
{
    Unk0049c2c0 *p;

    Frontend_AccumulateDeviceKeyCounters();
    p = Game_GetSecondaryCallbackMachine();
    g_unk0x0082a938 = 1;
    g_unk0x0082b0a4 = CMain::GetFrameDelta();
    CGame::PromoteCallbackEntryByRule(p, 0, 0, 2);
    if (CGameInfo::GetGameModeOptionBit19() != '\0' && param1 == '\0') {
        CGame::DestroyLocalNetworkPlayer();
        Network_CloseSession();
        Game_RequestOptionRefresh();
    }
}

// Starts the fade of the option menu once more than 50 frames have passed.
// FUNCTION: CMR2 0x00501350
void OptionMenu_StartDelayedFade(int param1, int unused)
{
    unsigned int delta = CMain::GetFrameDelta() - g_unk0x0082b0a4;

    if (delta > 0x32) {
        if (g_unk0x0082a938 != 0) {
            CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, 0, 2, 2);
            return;
        }
        CGame::PromoteCallbackEntryByRule((Unk0049c2c0 *)param1, 0, 0, 2);
    }
}

// Menu colour the boot screens fade to/from, r/g/b in its low three bytes.
// GLOBAL: CMR2 0x0052704c
int g_unk0x0052704c = 0x00acb49c;

int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);
int Game_DrawSceneViewport(int, int, void *, int, BYTE);
void Graphics_PresentFrameAndResetCounters(void);

// Clears the screen to a colour fading from the stored menu colour to grey over
// 50 frames, then draws the option menu scene.
// Option menu states 0x5010a0/0x501130/0x5012e0/0x5015d0 (cascade seed batch 1, verified 94-97%).
void OptionMenu_CommitRallyRecords(void);
void OptionMenu_CopyDefaultValues(void);
/* ===== cascade: menu states (drafts, iterated against the scaffold) ===== */
void OptionMenu_SetBackgroundViewport(void);
void Graphics_PresentFrameAndResetCounters(void);
void OptionPreview_SetWheelNodesVisible(int index, char visible);
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);
void OptionMenu_DrawActivePage(void);
int Game_DrawSceneViewport(int a, int b, void *c, int d, BYTE e);


// FUNCTION: CMR2 0x005010a0
void OptionMenu_LeaveAndCommitState(Unk0049c2c0 *p1, BYTE state)
{
    CSound::CloseMusicStreamAndClearPath(0);
    CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    OptionMenu_SetBackgroundViewport();
    Graphics_PresentFrameAndResetCounters();
    OptionMenu_CommitRallyRecords();
    OptionMenu_CopyDefaultValues();
    CGame::UnwindCallbacks(g_unk0x0082af88);
    CGraphics::EvictManagedTextureResources();
    CGraphics::FreeTextureBuffers();
    CGame::PromoteCallbackEntryByRule(p1, state, 0, 2);
    if (g_unk0x0082b0ac != 0) {
        CGame::SetFrontendResourceMode(0);
        CGameInfo::SetInputAndGamePaused(0);
        return;
    }
    CGame::SetFrontendResourceMode(3);
    CGameInfo::SetInputAndGamePaused(0);
}

// FUNCTION: CMR2 0x00501130
void OptionMenu_LeaveWithoutCommitState(Unk0049c2c0 *p1, BYTE state)
{
    CSound::CloseMusicStreamAndClearPath(0);
    CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    OptionMenu_SetBackgroundViewport();
    Graphics_PresentFrameAndResetCounters();
    CGame::UnwindCallbacks(g_unk0x0082af88);
    CGraphics::EvictManagedTextureResources();
    CGraphics::FreeTextureBuffers();
    CGame::PromoteCallbackEntryByRule(p1, state, 0, 2);
    CGame::SetFrontendResourceMode(0);
    CGameInfo::SetInputAndGamePaused(0);
}

// FUNCTION: CMR2 0x005012e0
void OptionMenu_UpdateHiddenPreviewState(Unk0049c2c0 *p1, BYTE state)
{
    int i;

    if (g_unk0x0082af8c != 0) {
        CMain::UpdateFrameTime();
        g_unk0x0082b0a4 = CMain::GetFrameDelta();
        g_unk0x0082af8c = 0;
    }
    for (i = 0; i < (int)(CGameInfo::GetConfiguredPlayerCount() & 0xff); i = i + 1) {
        OptionPreview_SetWheelNodesVisible(i, 0);
    }
    if (0x32 < (unsigned int)(CMain::GetFrameDelta() - g_unk0x0082b0a4))
        CGame::PromoteCallbackEntryByRule(p1, 0, 0, 2);
}

// FUNCTION: CMR2 0x005015d0
void OptionMenu_DrawBackgroundState(Unk0049c2c0 *p1, BYTE state)
{
    short rect[4];

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)((int)g_pGraphics->resX * 2 / 3);
    rect[3] = (short)((int)g_pGraphics->resY * 2 / 3);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    CGraphics::SetProjection(0x30978, 0x4326e, 0xfa0000, 0x10000);
    OptionMenu_DrawActivePage();
    Game_PrepareScene((SceneNode *)g_unk0x0082b1b4, (SceneNode *)g_unk0x0082b1b0, (int)&rect[0], 0);
    Game_DrawSceneViewport(g_unk0x0082b1b4, g_unk0x0082b1b0, &rect[0], 0, 1);
    Graphics_PresentFrameAndResetCounters();
}

// FUNCTION: CMR2 0x00501780
void OptionMenu_FadeBackgroundToGrey(int param1, int unused)
{
    short rect[4];
    BYTE colour[4];
    int t;
    int inv;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)((int)g_pGraphics->resX * 2 / 3);
    rect[3] = (short)((int)g_pGraphics->resY * 2 / 3);
    t = (CMain::GetFrameDelta() * 1000 - g_unk0x0082b0a4 * 1000) / 50;
    inv = 1000 - t;
    // Fades between the menu colour and the grey (141, 151, 159) used by
    // OptionMenu_InitBackground while the level is built.
    colour[0] = (BYTE)(((g_unk0x0052704c & 0xff) * inv + 141 * t) / 1000);
    colour[1] = (BYTE)(((g_unk0x0052704c >> 8 & 0xff) * inv + 151 * t) / 1000);
    colour[2] = (BYTE)(((g_unk0x0052704c >> 16 & 0xff) * inv + 159 * t) / 1000);
    CGraphics::SetClearColour(1, colour[0], colour[1], colour[2]);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    CGraphics::SetProjection(0x30978, 0x4326e, 0xfa0000, 0x10000);
    // The globals are the root and camera scene nodes; the third argument is a
    // temporary 4-short rectangle covering the middle of the screen.
    Game_PrepareScene((SceneNode *)g_unk0x0082b1b4, (SceneNode *)g_unk0x0082b1b0, (int)rect, 0);
    Game_DrawSceneViewport(g_unk0x0082b1b4, g_unk0x0082b1b0, rect, 0, 1);
    Graphics_PresentFrameAndResetCounters();
}

// Same as OptionMenu_FadeBackgroundToGrey but the colour fades from grey to the stored menu colour.
// The extra GetFrameDelta() result is discarded by the original.
// FUNCTION: CMR2 0x00501920
void OptionMenu_FadeToStoredColour(int param1, int unused)
{
    short rect[4];
    BYTE colour[4];
    int t;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)((int)g_pGraphics->resX * 2 / 3);
    rect[3] = (short)((int)g_pGraphics->resY * 2 / 3);
    t = (CMain::GetFrameDelta() * 1000 - g_unk0x0082b0a4 * 1000) / 50;
    colour[0] = (BYTE)((((g_unk0x0052704c & 0xff) - 141) * t + 141000) / 1000);
    colour[1] = (BYTE)((((g_unk0x0052704c >> 8 & 0xff) - 151) * t + 151000) / 1000);
    colour[2] = (BYTE)((((g_unk0x0052704c >> 16 & 0xff) - 159) * t + 159000) / 1000);
    CGraphics::SetClearColour(1, colour[0], colour[1], colour[2]);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    CGraphics::SetProjection(0x30978, 0x4326e, 0xfa0000, 0x10000);
    CMain::GetFrameDelta();
    Game_PrepareScene((SceneNode *)g_unk0x0082b1b4, (SceneNode *)g_unk0x0082b1b0, (int)rect, 0);
    Game_DrawSceneViewport(g_unk0x0082b1b4, g_unk0x0082b1b0, rect, 0, 1);
    Graphics_PresentFrameAndResetCounters();
}

// Copies the option records back into the rally data (undo of OptionMenu_LoadRallyRecords).
// FUNCTION: CMR2 0x00502db0
void OptionMenu_CommitRallyRecords(void)
{
    int *pSrc;
    int *pDst;
    int i;
    int j;

    i = 0;
    if ((BYTE)CGameInfo::GetConfiguredPlayerCount() > 0) {
        pSrc = (int *)g_unk0x0082c070;
        do {
            pDst = (int *)RallyData_GetDriverGroupRecord(i);
            i++;
            memcpy(pDst, pSrc, 0x148);
            pSrc += 0x52;
        } while (i < (int)(CGameInfo::GetConfiguredPlayerCount() & 0xff));
    }
}

// Copies the default option values from the global table into each record.
// FUNCTION: CMR2 0x005029b0
void OptionMenu_CopyDefaultValues(void)
{
    int i;
    BYTE *pDst;
    for (i = 0; i < 4; i++) {
        pDst = RallyData_GetDriverSkillRecord(i);
        pDst[4] = g_unk0x0082bee8[i][4];
        pDst[5] = g_unk0x0082bee8[i][5];
        pDst[1] = g_unk0x0082bee8[i][1];
        pDst[6] = g_unk0x0082bee8[i][6];
        pDst[3] = g_unk0x0082bee8[i][3];
        pDst[2] = g_unk0x0082bee8[i][2];
        pDst[0] = g_unk0x0082bee8[i][0];
    }
}

// Applies the selected option: advances the menu when its value is set, or
// starts the fade otherwise.
// FUNCTION: CMR2 0x005000b0
void OptionMenu_AdvanceSelectedOption(int unused, int unused2)
{
    BYTE *pMode;
    int index;

    OptionMenu_FillValueSlider();
    index = Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1);
    index *= 5;
    pMode = OptionMenu_GetControlSetupMenu();
    if (OptionMenu_IsSlotValueAboveBase(CGameInfo::GetActiveOptionSlot(), pMode[0x1f + index * 4]) != 0) {
        Menu_SetNextAction((int)OptionMenu_GetValueSliderMenu());
        return;
    }
    CGameInfo::StartOptionMenuTimeout();
}

// Loads the four option menu fonts.
// FUNCTION: CMR2 0x0050f370
void OptionMenu_LoadFonts(void)
{
    Font_InitTable(4);
    Font_Load(g_strFontGeneralHel15pt, (GenericFile *)OptionMenu_GetCommonArchive(), 0);
    Font_Load(g_strFontGeneralHel12pt, (GenericFile *)OptionMenu_GetCommonArchive(), 1);
    Font_Load(g_strFontGeneralDot, (GenericFile *)OptionMenu_GetCommonArchive(), 2);
    Font_Load(g_strFontGeneralHandel, (GenericFile *)OptionMenu_GetCommonArchive(), 3);
}

// Sets the whole mesh list of the record to opaque when the record has content
// and the per-mesh flag is set, to fully transparent otherwise.
// FUNCTION: CMR2 0x00509150
void OptionPreview_SetMeshOpacity(int index)
{
    BYTE *pRecord = (BYTE *)&g_unk0x0082d220[index];
    int i;
    if (*(int *)(pRecord + 0x2a8) != 0) {
        for (i = 0; i < pRecord[0x26a]; ++i) {
            if (*(int *)(pRecord + 0x26c + i * 4) != 0)
                *(BYTE *)(*(int *)(pRecord + 0x3c + i * 4) + 0x17c) = 0xff;
            else
                *(BYTE *)(*(int *)(pRecord + 0x3c + i * 4) + 0x17c) = 0;
        }
    }
}

/* ===== integrated from casc/s6 ===== */
// Textures of the option menu (symbols, banners and car parts).
// GLOBAL: CMR2 0x00831360
int g_unk0x00831360;
// GLOBAL: CMR2 0x00831364
int g_unk0x00831364;
// GLOBAL: CMR2 0x00831368
int g_unk0x00831368;
// GLOBAL: CMR2 0x00831668
int g_unk0x00831668;
// GLOBAL: CMR2 0x0083166c
int g_unk0x0083166c;
// GLOBAL: CMR2 0x00831670
int g_unk0x00831670;
// GLOBAL: CMR2 0x008313ac
int g_unk0x008313ac;
// GLOBAL: CMR2 0x00831648
int g_unk0x00831648;
// GLOBAL: CMR2 0x008313b0
int g_unk0x008313b0;
// GLOBAL: CMR2 0x00831674
int g_unk0x00831674;
// GLOBAL: CMR2 0x0083137c
int g_unk0x0083137c[12];
// Country banner codes, in banner order (the last one is CFrontend::m_strUK).
// Names of the car part textures.
// GLOBAL: CMR2 0x00529590
char g_str0x00529590[8] = "AXLES";
// GLOBAL: CMR2 0x00529598
char g_str0x00529598[8] = "DRIVE";
// GLOBAL: CMR2 0x005295a0
char g_str0x005295a0[8] = "EXHAUST";
// GLOBAL: CMR2 0x005295a8
char g_str0x005295a8[8] = "ELEC";
// GLOBAL: CMR2 0x005295b0
char g_str0x005295b0[8] = "STEER";
// GLOBAL: CMR2 0x005295b8
char g_str0x005295b8[8] = "BODY";
// GLOBAL: CMR2 0x005295c0
char g_str0x005295c0[8] = "BRAKES";
// GLOBAL: CMR2 0x005295c8
char g_str0x005295c8[8] = "DIFFER";
// GLOBAL: CMR2 0x005295d0
char g_str0x005295d0[8] = "SUSP";
// GLOBAL: CMR2 0x005295d8
char g_str0x005295d8[8] = "TURBO";
// GLOBAL: CMR2 0x005295e0
char g_str0x005295e0[8] = "GEAR";
// GLOBAL: CMR2 0x005295e8
char g_str0x005295e8[8] = "TYRES";
// GLOBAL: CMR2 0x0052956c
char g_str0x0052956c[] = "%s\\Textures\\Symbols\\%d\\DanRed.tga";
// GLOBAL: CMR2 0x00529548
char g_str0x00529548[] = "%s\\Textures\\Symbols\\%d\\DanOra.tga";
// GLOBAL: CMR2 0x00529524
char g_str0x00529524[] = "%s\\Textures\\Symbols\\%d\\DanYel.tga";
// GLOBAL: CMR2 0x00529508
char g_str0x00529508[] = "%s\\Textures\\Banners\\b%s.tga";
// GLOBAL: CMR2 0x005294f0
char g_str0x005294f0[] = "%s\\Textures\\Ar_640A.tga";
// GLOBAL: CMR2 0x005294d8
char g_str0x005294d8[] = "%s\\Textures\\Ar_640D.tga";
// GLOBAL: CMR2 0x005294b8
char g_str0x005294b8[] = "%s\\Textures\\Symbols\\%d\\Tick.tga";
// GLOBAL: CMR2 0x00529494
char g_str0x00529494[] = "%s\\Textures\\Symbols\\%d\\Cross.tga";
// GLOBAL: CMR2 0x00529474
char g_str0x00529474[] = "%s\\Textures\\Symbols\\%d\\Box.tga";
// GLOBAL: CMR2 0x00529450
char g_str0x00529450[] = "%s\\Textures\\Symbols\\%d\\Circle.tga";
// GLOBAL: CMR2 0x00529434
char g_str0x00529434[] = "%s\\Textures\\Parts\\%d\\%s.tga";

// Loads the option menu textures: the three "Dan" symbols, the country
// banner, the 640 arrows, the tick/cross/box/circle symbols and one texture
// per car part.
// FUNCTION: CMR2 0x0050a080
void OptionMenu_LoadPartTextures(void)
{
    char *pBanners[8] = { g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274,
                          g_str0x00519270, g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK };
    char *pParts[12] = { g_str0x005295e8, g_str0x005295e0, g_str0x005295d8, g_str0x005295d0,
                         g_str0x005295c8, g_str0x005295c0, g_str0x005295b8, g_str0x005295b0,
                         g_str0x005295a8, g_str0x005295a0, g_str0x00529598, g_str0x00529590 };
    int i;

    sprintf(CFrontend::m_stringDest, g_str0x0052956c, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x00831360 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529548, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x00831364 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529524, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x00831368 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529508, CInstallInfo::GetSetupRepDir(),
            pBanners[RallyDataCountryIndex() & 0xff]);
    g_unk0x00831668 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetStageArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x005294f0, CInstallInfo::GetFrontendDir());
    g_unk0x0083166c = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x005294d8, CInstallInfo::GetFrontendDir());
    g_unk0x00831670 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x005294b8, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x008313ac = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529494, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x00831648 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529474, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x008313b0 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x00529450, CInstallInfo::GetSetupRepDir(), 0x280);
    g_unk0x00831674 = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest, 0, 0, 0, 0);
    for (i = 0; i < 12; i++) {
        sprintf(CFrontend::m_stringDest, g_str0x00529434, CInstallInfo::GetSetupRepDir(), 0x280, pParts[i]);
        g_unk0x0083137c[i] = (int)CTexture::FindLoadTexture((GenericFile *)OptionMenu_GetCommonArchive(), CFrontend::m_stringDest,
                                                            0, 0, 0, 0);
    }
}

// Per-index block of the option menu's 3D preview (0x138 bytes each).
// Geometry of the option menu preview deform (0x138 bytes per record, 8 records).
struct Unk0x0082cb78 {
    // The deform code (OptionPreview_BuildDeformGeometry/0x507fe0) reads the first 0x14 bytes as ints and the
    // 0x14/0x24/0x34/0x44 slots as int[4]; both spellings are the same memory.
    union {
        struct { BYTE field_0x0; BYTE field_0x1[3]; SceneNode *pNode5; }; // 0x4 fifth child
        struct { int field_0x00; int field_0x04; };
    };
    union { SceneNode *pNode; int field_0x08; };              // 0x8
    union {
        BYTE field_0xc[8];
        struct { int field_0x0c; int field_0x10; };
    };
    union { SceneNode *pWheels[4]; int matrix[4]; };          // 0x14 anchor matrices
    union { Mesh *pMesh24[4]; int field_0x24[4]; };           // 0x24
    union { Mesh *pMesh34[4]; int field_0x34[4]; };           // 0x34
    union { Mesh *pMesh44[4]; int field_0x44[4]; };           // 0x44
};

// GLOBAL: CMR2 0x0082cb78
Unk0x0082cb78 g_unk0x0082cb78[16];

void OptionPreview_AssignConvertedVertexBuffers(int param1, int param2, int param3);

// Rebuilds the stage mesh record of the option menu preview entry: clears the
// record, adds the mesh of every node of the entry's node tree, compacts the
// empty slots and matches the converted vertex buffers.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00507080
void OptionPreview_RebuildStageMeshRecord(int index)
{
    Unk0x0082d220 *pRecord;
    Unk0x0082cb78 *pEntry;
    SceneNode *pNode;
    SceneNode *pSibling;
    int i;
    int j;
    int moved;

    pRecord = &g_unk0x0082d220[index];
    memset(pRecord, 0, sizeof(Unk0x0082d220));
    pEntry = &g_unk0x0082cb78[index];
    pRecord->meshCount = 0;
    if (pEntry->pNode5 == 0)
        return;
    pRecord->field_0x228 = 0;
    pRecord->field_0x224 = 0;
    pRecord->field_0x220 = 0;
    pRecord->field_0x21c = 0;
    OptionPreview_AddNodeMeshToRecord(pEntry->pNode5, pRecord);
    pNode = pEntry->pNode5->pFirstChild;
    if (pNode != 0) {
        do {
            pSibling = pNode;
            if ((BYTE)pNode->flags != 0x14 && pNode != 0) {
                do {
                    OptionPreview_AddNodeMeshToRecord(pNode, pRecord);
                    pNode = pNode->pFirstChild;
                } while (pNode != 0);
            }
            pNode = pSibling->pNext;
        } while (pNode != 0);
    }
    for (i = 0; i < 15; i++) {
        if (pRecord->pNodes[i] != 0)
            continue;
        moved = 0;
        if (i < 14) {
            for (j = i; j < 14; j++) {
                if (pRecord->pNodes[j] != 0 || pRecord->pNodes[j + 1] != 0)
                    moved = 1;
                pRecord->pMeshes[j] = pRecord->pMeshes[j + 1];
                pRecord->pNodes[j] = pRecord->pNodes[j + 1];
                pRecord->pVertexData[j] = pRecord->pVertexData[j + 1];
                pRecord->vertexCount[j] = pRecord->vertexCount[j + 1];
                pRecord->centre[j] = pRecord->centre[j + 1];
                pRecord->halfSize[j] = pRecord->halfSize[j + 1];
            }
            if (moved)
                i--;
        }
    }
    OptionPreview_AssignConvertedVertexBuffers(index, (int)pEntry->pNode5, (int)pRecord);
}

struct Unk0x0082fd00 {
    Unk0x0082cb78 *pEntry;      // 0x000 stage entry the geometry belongs to
    FixVector corner[8];        // 0x004 bounding-box corners
    FixVector vertex[12];       // 0x064 vertices the deform displaces
    FixVector anchor[4];        // 0x0f4 position of each anchor matrix
    int field_0x124;            // 0x124
    int field_0x128[4];         // 0x128 random wobble of each anchor
};

// GLOBAL: CMR2 0x0082fd00
Unk0x0082fd00 g_unk0x0082fd00[8];


// Timestamp of the previous frame of the option menu preview animation.
// GLOBAL: CMR2 0x0082d118
unsigned int g_unk0x0082d118;

// Animates the four preview nodes of the option record: each one gets an
// identity (or mirrored) basis, and while the menu is fading the forward
// vector is shrunk to 0x9999.
// FUNCTION: CMR2 0x00509dc0
void OptionPreview_AnimatePreviewNodeTransforms(int index)
{
    int *pList;
    Unk0x0082d220 *pRec;
    FixBasis mirror;
    FixBasis basis;
    FixVector v;
    unsigned int elapsed;
    unsigned int now;
    int i;
    int value;

    pList = (int *)&g_unk0x0082fd00[index];
    pRec = &g_unk0x0082d220[index];
    basis.right.x = 0x10000;
    basis.right.y = 0;
    basis.right.z = 0;
    basis.up.x = 0;
    basis.up.y = 0x10000;
    basis.up.z = 0;
    basis.forward.x = 0;
    basis.forward.y = 0;
    basis.forward.z = 0x10000;
    mirror.right.x = -0x10000;
    mirror.right.y = 0;
    mirror.right.z = 0;
    mirror.up.x = 0;
    mirror.up.y = 0x10000;
    mirror.up.z = 0;
    mirror.forward.x = 0;
    mirror.forward.y = 0;
    mirror.forward.z = -0x10000;
    if (g_unk0x0082d118 == 0) {
        g_unk0x0082d118 = Sys_GetTicks();
        elapsed = 0;
    } else {
        now = Sys_GetTicks();
        elapsed = now - g_unk0x0082d118;
        g_unk0x0082d118 = now;
        elapsed = FixDiv(elapsed << 16, 0x280000);
        elapsed = FixMul(0xa0000, elapsed);
    }
    value = pList[0x124 / 4] + elapsed;
    pList[0x124 / 4] = value;
    if (value > 0x1680000)
        pList[0x124 / 4] = value - 0x1680000;
    for (i = 0; i < 4; i++) {
        // the original discards this result
        value = pRec->field_0x23c.v[i];
        value = FixMul(0x50000, value);
        if (i % 2 == 0) {
            FixMatrix_SetRight(&basis.right, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
            FixMatrix_SetUp(&basis.up, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
            FixMatrix_SetForward(&basis.forward, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
        } else {
            FixMatrix_SetRight(&mirror.right, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
            FixMatrix_SetUp(&mirror.up, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
            FixMatrix_SetForward(&mirror.forward, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
        }
        if ((OptionPreview_UsesWheelVariantSix(*(BYTE *)pList[0], (char)OptionMenu_GetScaledWorkingValue(index, 0)) != 0 && CGameInfo::GetPreviewMode() == 2) ||
            CGameInfo::GetConfiguredPlayerCount() > 1) {
            FixMatrix_GetForward(&v, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
            FixVecScale(&v, &v, 0x9999);
            FixMatrix_SetForward(&v, &(*(SceneNode **)(pList[0] + 0x14 + i * 4))->current);
        }
    }
}

// Normalises a vector and flips it when it points down.
#define OPTIONS_FIX_NORMALIZE_FLIP(v)                                               \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            v.x = 0;                                                                \
            v.y = 0;                                                                \
            v.z = 0;                                                                \
        } else {                                                                    \
            FixVecScaleRecip(&v, &v, len);                                          \
            if (v.y < 0)                                                            \
                FixVecScale(&v, &v, -0x10000);                                      \
        }                                                                           \
    }

// Builds the preview node's transform from the four corner heights stored in
// the option record (field_0x22c): the up axis is the sum of the two edge
// normals, the other axes come from Gram-Schmidt and the position from the
// average height.
// FUNCTION: CMR2 0x005091c0
void OptionPreview_BuildCornerHeightTransform(int index)
{
    Unk0x0082d220 *pRec;
    int *pList;
    int f[4];
    FixVector v;
    FixVector v2;
    FixVector cross1;
    FixVector cross2;
    FixVector c[3];
    FixVector *p0;
    FixVector *p2;
    FixBasis basis;
    int a;
    int b;
    int len;
    int i;
    int t;

    pRec = &g_unk0x0082d220[index];
    pList = (int *)&g_unk0x0082fd00[index];
    if (OptionMenu_FindFirstDetailedSlot() == 1 && OptionMenu_GetControlSetupMenu()[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14 + 0x1f] == 2) {
        t = (unsigned char)OptionMenu_GetValueSliderMenu()[0x1f] << 16;
        t = FixMul(t, 0x1999);
    } else {
        t = (int)(char)OptionMenu_GetScaledWorkingValue(index, 2) << 16;
        t = FixMul(t, 0x1999);
    }
    t = FixMul(0x10000 - t, 0x1999);
    for (i = 0; i < 4; i++) {
        f[i] = pRec->field_0x22c.v[i] + t;
        if (f[i] < -0x1999)
            f[i] = -0x1999;
    }
    a = FixMul(0x8000, f[2] + f[3]);
    b = FixMul(0x8000, f[0] + f[1]);
    // The corners are copied whole (y included) before taking the differences.
    p0 = (FixVector *)&pList[1];
    c[0] = *p0;
    c[1] = *(FixVector *)&pList[4];
    p2 = (FixVector *)&pList[7];
    c[2] = *p2;
    v.x = c[1].x - c[0].x;
    v.y = f[1] - f[0];
    v.z = c[1].z - c[0].z;
    v2.x = c[2].x - c[0].x;
    v2.y = a - f[0];
    v2.z = -c[0].z;
    FixVecCross(&cross1, &v, &v2);
    OPTIONS_FIX_NORMALIZE_FLIP(cross1);
    c[0] = *p2;
    c[1] = *(FixVector *)&pList[10];
    c[2] = *p0;
    v.x = c[1].x - c[0].x;
    v.y = f[3] - f[2];
    v.z = c[1].z - c[0].z;
    v2.x = c[2].x - c[0].x;
    v2.y = b - f[2];
    v2.z = -c[0].z;
    FixVecCross(&cross2, &v, &v2);
    OPTIONS_FIX_NORMALIZE_FLIP(cross2);
    basis.right.x = 0x10000;
    basis.right.y = 0;
    basis.right.z = 0;
    basis.up.x = 0;
    basis.up.y = 0x10000;
    basis.up.z = 0;
    basis.forward.x = 0;
    basis.forward.y = 0;
    basis.forward.z = 0x10000;
    v.x = cross1.x + cross2.x;
    v.y = cross1.y + cross2.y;
    v.z = cross1.z + cross2.z;
    FIX_NORMALIZE_INTO(cross1, v)
    basis.up.x = cross1.x;
    basis.up.y = cross1.y;
    basis.up.z = cross1.z;
    len = FixVecDot(&basis.right, &basis.up);
    FixVecScale(&v, &basis.up, len);
    v.x = basis.right.x - v.x;
    v.y = basis.right.y - v.y;
    v.z = basis.right.z - v.z;
    FIX_NORMALIZE_INTO(basis.right, v)
    FixVecCross(&v, &basis.right, &basis.up);
    {
        FixVector *pForward = &basis.forward;

        len = FixVecLength(&v);
        if (len == 0) {
            pForward->x = 0;
            pForward->y = 0;
            pForward->z = 0;
        } else {
            FixVecScaleRecip(pForward, &v, len);
        }
    }
    t = FixMul(0x8000, b + a);
    v.x = 0;
    v.y = t;
    v.z = 0;
    (*(SceneNode **)(pList[0] + 4))->useParentWorld = 0;
    FixMatrix_SetRight(&basis.right, &(*(SceneNode **)(pList[0] + 4))->current);
    FixMatrix_SetUp(&basis.up, &(*(SceneNode **)(pList[0] + 4))->current);
    FixMatrix_SetForward(&basis.forward, &(*(SceneNode **)(pList[0] + 4))->current);
    FixMatrix_SetPosition(&v, &(*(SceneNode **)(pList[0] + 4))->current);
}

/* ===== integrated from casc/s2 ===== */
// ---------------------------------------------------------------------------
// Per-slot rally tables used by the wheel-mesh selection and the stage
// timing data of each slot.
// ---------------------------------------------------------------------------

// Number of filled slots (1 or 2), from CGameInfo::GetPreviewLayoutMode.
// GLOBAL: CMR2 0x0082c694
int g_unk0x0082c694;
// Active slot read from the current mode entry's field 0x48.
// GLOBAL: CMR2 0x0082c710
BYTE g_unk0x0082c710;
// Texture set index of each slot (100 = no data); g_unk0x0082ca18 holds how
// many of them are valid.
// GLOBAL: CMR2 0x0082ca04
BYTE g_unk0x0082ca04[0x14];
// GLOBAL: CMR2 0x0082ca18
BYTE g_unk0x0082ca18;

// 0x54-byte per-slot entry of the table at 0x82cb78: the scene node whose
// mesh is shown and the three wheel-mesh variants that can be assigned to it.

// GLOBAL: CMR2 0x0082d15c
int g_unk0x0082d15c[2];

// 8-byte entry (four 16-bit values) of the timing tables: current
// 0x82d0b8/0x82d0f8, animated 0x82d0d8 and target 0x82fce0, four slots each.
struct Unk0x0082d0b8 {
    short field_0x0;            // 0x0
    short field_0x2;            // 0x2
    short field_0x4;            // 0x4
    short field_0x6;            // 0x6
};

// GLOBAL: CMR2 0x0082d0b8
Unk0x0082d0b8 g_unk0x0082d0b8[4];
// GLOBAL: CMR2 0x0082d0d8
Unk0x0082d0b8 g_unk0x0082d0d8[4];
// GLOBAL: CMR2 0x0082d0f8
Unk0x0082d0b8 g_unk0x0082d0f8[4];
// GLOBAL: CMR2 0x0082fce0
Unk0x0082d0b8 g_unk0x0082fce0[4];
// GLOBAL: CMR2 0x00831080
int g_unk0x00831080;
// GLOBAL: CMR2 0x00831148
int g_unk0x00831148[20];
// GLOBAL: CMR2 0x0083131c
BYTE g_unk0x0083131c[4];
// Per-slot converted vertex buffer of the wheel meshes: each slot holds an
// array of one block per source mesh.
// GLOBAL: CMR2 0x00831198
BYTE **g_unk0x00831198[2];
// Per-slot byte copied from each source mesh (its field 0x30).
// GLOBAL: CMR2 0x0082d1dc
BYTE *g_unk0x0082d1dc[2];

int *RallyData_GetDriverPrimaryPairRecord(int index);
int *RallyData_GetDriverSettingPair(int index);
int *RallyData_GetDriverSecondaryPairRecord(int index);
BYTE OptionMenu_GetScaledWorkingValue(int index, int type);
extern char g_strWheelVariantL[4];
extern char g_strWheelVariantN[4];
extern double g_unk0x00511300;

// Fills the per-slot texture set indices (g_unk0x0082ca04, counted by
// g_unk0x0082ca18) from the rally tables of the active slot; entries without
// a table value are marked 100.
// FUNCTION: CMR2 0x00505e70
void OptionPreview_FillTextureSetIndices(void)
{
    int *pStages;
    BYTE *pPairs;
    BYTE *pDest;
    int slot;
    int count;
    int i;
    int *pEntry;

    pStages = RallyData_GetDriverPrimaryPairRecord(0);
    pPairs = (BYTE *)RallyData_GetDriverSettingPair(0);
    // The original reads [index * 0x50 + 0x82c6c0]: byte 0x48 of the previous
    // 0x50-byte entry of the table at 0x82c6c8 (index is 1-based).
    count = ((BYTE *)g_unk0x0082c6c8)[g_unk0x0082c694 * 0x50 - 8];
    slot = g_unk0x0082c710 & 0xff;
    g_unk0x0082ca18 = count - slot + 2;
    pEntry = RallyData_GetDriverSecondaryPairRecord(slot);
    if (*pEntry != 0)
        g_unk0x0082ca04[0] = 100;
    else
        g_unk0x0082ca04[0] = pPairs[slot * 8];
    if (slot <= count) {
        i = slot;
        pDest = &g_unk0x0082ca04[1];
        do {
            pEntry = RallyData_GetDriverSecondaryPairRecord(slot);
            if (*pEntry != 0)
                *pDest = 100;
            else
                *pDest = (BYTE)pStages[i];
            i++;
            pDest++;
        } while (i <= count);
    }
}

// Assigns one of the three wheel-mesh variants to the scene nodes of the slot
// according to the option tables, and when the textures do not come from the
// CD swaps the "L"/"N" variants of every wheel mesh triangle of the slot.
// Byte-offset views of the 0x54-byte entries of the table at 0x82cb78 (same
// fields as Unk0x0082cb78, indexed with the source's byte offset).
#define CB78_BYTE(o) (*(BYTE *)((BYTE *)&g_unk0x0082cb78 + (o)))
#define CB78_NODE(o) (*(SceneNode **)((BYTE *)&g_unk0x0082cb78 + (o)))
#define CB78_MESH(o) (*(Mesh **)((BYTE *)&g_unk0x0082cb78 + (o)))

// FUNCTION: CMR2 0x00506080
void OptionPreview_UpdateWheelMeshVariants(int param1)
{
    int off;
    int i;
    int k;
    int w;
    Mesh *pMesh;
    Texture *pTex;

    off = param1 * 0x54;
    if (CGameInfo::GetPreviewMode() == 0) {
        if (g_unk0x0082d15c[param1] != (int)(char)OptionMenu_GetScaledWorkingValue(param1, 0)) {
            int *pVal = &g_unk0x0082d15c[param1];
            *pVal = (int)(char)OptionMenu_GetScaledWorkingValue(param1, 0);
            if (OptionPreview_UsesWheelVariantSix(CB78_BYTE(off), *pVal) != 0) {
                for (i = 0; i < 4; i++) {
                    if (CB78_MESH(off + 0x44 + i * 4) != 0)
                        *(Mesh **)((BYTE *)CB78_NODE(off + 0x14 + i * 4) + 0xc) = CB78_MESH(off + 0x44 + i * 4);
                }
                return;
            }
            if (OptionPreview_UsesLowWheelVariant(CB78_BYTE(off), g_unk0x0082d15c[param1]) != 0) {
                for (i = 0; i < 4; i++) {
                    if (CB78_MESH(off + 0x34 + i * 4) != 0)
                        *(Mesh **)((BYTE *)CB78_NODE(off + 0x14 + i * 4) + 0xc) = CB78_MESH(off + 0x34 + i * 4);
                }
                return;
            }
            for (i = 0; i < 4; i++) {
                if (CB78_MESH(off + 0x24 + i * 4) != 0)
                    *(Mesh **)((BYTE *)CB78_NODE(off + 0x14 + i * 4) + 0xc) = CB78_MESH(off + 0x24 + i * 4);
            }
            return;
        }
    } else {
        if (g_unk0x0082d15c[param1] != (int)(char)OptionMenu_GetScaledWorkingValue(param1, 0)) {
            g_unk0x0082d15c[param1] = (int)(char)OptionMenu_GetScaledWorkingValue(param1, 0);
            for (w = 0; w < 4; w++) {
                pMesh = *(Mesh **)((BYTE *)CB78_NODE(off + 0x14) + 0xc);
                for (k = 0; k < pMesh->triangleCount; k++) {
                    pTex = CGraphics::m_pTextureManager->textureBuffer[((int *)&pMesh->pTriangles[k])[1]];
                    if (OptionPreview_UsesLowWheelVariant(CB78_BYTE(off), g_unk0x0082d15c[param1]) != 0) {
                        if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1) == 0) {
                            strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1);
                            Graphics_ReloadTexture(pTex);
                        }
                    } else if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1) == 0) {
                        strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1);
                        Graphics_ReloadTexture(pTex);
                    }
                }
            }
        }
    }
}

// Animates every slot's wheel rotation towards its target angles: slots whose
// animation time is over copy the target straight into the current angles, the
// rest write the animated angles scaled by the squared proportion of the
// elapsed time and hand them to the slot's scene node.
// FUNCTION: CMR2 0x00506720
void OptionPreview_AnimateWheelRotations(void)
{
    unsigned int time;
    int i;

    time = CMain::GetFrameDelta();
    i = 0;
    if (CGameInfo::GetConfiguredPlayerCount() > 0) {
        do {
            unsigned int duration;
            short *pAngles;

            duration = g_unk0x00831080;
            if (g_unk0x0083131c[i] != 0 && time - g_unk0x00831148[i] > g_unk0x00831080)
                g_unk0x0083131c[i] = 0;
            pAngles = (short *)&g_unk0x0082d0b8[i];
            if (g_unk0x0083131c[i] != 0) {
                unsigned int t = (time * 100 - g_unk0x00831148[i] * 100) / duration;
                int k = (int)(t * t) / 100;

                pAngles[0] = (short)(g_unk0x0082d0d8[i].field_0x0 * k / 100 + g_unk0x0082d0f8[i].field_0x0);
                pAngles[1] = (short)(g_unk0x0082d0d8[i].field_0x2 * k / 100 + g_unk0x0082d0f8[i].field_0x2);
                pAngles[2] = (short)(g_unk0x0082d0d8[i].field_0x4 * k / 100 + g_unk0x0082d0f8[i].field_0x4);
                OptionMenu_InterpolateLayoutPairs(i, 0, k);
            } else {
                pAngles[0] = g_unk0x0082fce0[i].field_0x0;
                pAngles[1] = g_unk0x0082fce0[i].field_0x2;
                pAngles[2] = g_unk0x0082fce0[i].field_0x4;
                OptionMenu_InterpolateLayoutPairs(i, 1, 0);
            }
            SceneNode_SetRotation(g_unk0x0082cb78[i].pNode, (FixAngles *)pAngles);
            OptionPreview_AnimatePreviewNodeTransforms(i);
            OptionPreview_BuildCornerHeightTransform(i);
            i++;
        } while (i < (CGameInfo::GetConfiguredPlayerCount() & 0xff));
    }
}

// Saves the rotation matrix of the slot's scene node, rotates the node to the
// given angles, rotates param2 by the new matrix into param3 and puts the old
// rotation matrix back.
// FUNCTION: CMR2 0x005068b0
void OptionPreview_RotateSlotVector(int param1, FixVector *param2, FixVector *param3, FixAngles *param4)
{
    SceneNode *pNode;
    FixMatrix saved;

    pNode = g_unk0x0082cb78[param1].pNode;
    saved = *(FixMatrix *)((BYTE *)pNode + 0x98);
    SceneNode_SetRotation(pNode, param4);
    FixMatrix_RotateVector(param3, param2, (FixMatrix *)((BYTE *)g_unk0x0082cb78[param1].pNode + 0x98));
    *(FixMatrix *)((BYTE *)g_unk0x0082cb78[param1].pNode + 0x98) = saved;
}

// Stores the target angles of a slot (three degrees as 16-bit values) in the
// current tables, wraps the delta to the target into [-180, 180) and writes
// the three wrapped deltas as 12-bit angles in the animated table.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00506930
void OptionPreview_SetTargetWheelAngles(int param1, short *param2, int param3)
{
    unsigned int time;
    short tX, tY, tZ;
    int d[3];
    int a[3];
    int abs;

    time = CMain::GetFrameDelta();
    g_unk0x00831080 = 0x14;
    tY = g_unk0x0082d0b8[param1].field_0x2;
    g_unk0x00831148[param1] = time;
    tX = g_unk0x0082d0b8[param1].field_0x0;
    g_unk0x0083131c[param1] = 1;
    tZ = g_unk0x0082d0b8[param1].field_0x4;
    g_unk0x0082d0f8[param1].field_0x0 = tX;
    g_unk0x0082d0f8[param1].field_0x2 = tY;
    g_unk0x0082d0f8[param1].field_0x4 = tZ;
    g_unk0x0082fce0[param1].field_0x0 = param2[0];
    g_unk0x0082fce0[param1].field_0x2 = param2[1];
    g_unk0x0082fce0[param1].field_0x4 = param2[2];
    if (param3 != 0) {
        g_unk0x0082d0b8[param1] = g_unk0x0082fce0[param1];
        g_unk0x0082d0f8[param1] = g_unk0x0082fce0[param1];
    }
    d[0] = g_unk0x0082fce0[param1].field_0x0 - g_unk0x0082d0f8[param1].field_0x0;
    d[1] = g_unk0x0082fce0[param1].field_0x2 - g_unk0x0082d0f8[param1].field_0x2;
    d[2] = g_unk0x0082fce0[param1].field_0x4 - g_unk0x0082d0f8[param1].field_0x4;
    a[0] = d[0] * 0x1680;
    a[1] = d[1] * 0x1680;
    a[2] = d[2] * 0x1680;
    abs = a[0];
    if (a[0] < 0)
        abs = -a[0];
    if (abs > 0xb40000) {
        if (a[0] >= 1)
            a[0] = 0x1680000 - a[0];
        else
            a[0] += 0x1680000;
    }
    abs = a[1];
    if (a[1] < 0)
        abs = -a[1];
    if (abs > 0xb40000) {
        if (a[1] > 0)
            a[1] = 0x1680000 - a[1];
        else
            a[1] += 0x1680000;
    }
    abs = a[2];
    if (a[2] < 0)
        abs = -a[2];
    if (abs > 0xb40000) {
        if (a[2] > 0)
            a[2] = 0x1680000 - a[2];
        else
            a[2] += 0x1680000;
    }
    g_unk0x0082d0d8[param1].field_0x0 = (short)(__int64)((double)a[0] * g_unk0x00511300);
    g_unk0x0082d0d8[param1].field_0x2 = (short)(__int64)((double)a[1] * g_unk0x00511300);
    g_unk0x0082d0d8[param1].field_0x4 = (short)(__int64)((double)a[2] * g_unk0x00511300);
}

void SceneNode_SetViewMaskTree(SceneNode *pNode, BYTE mask);

// Shows or hides the wheel scene nodes of a slot: the root node and its
// bounding-box child receive the visibility mask, and showing them also
// re-applies the option state and the wheel mesh variants.
// FUNCTION: CMR2 0x00506b20
void OptionPreview_SetWheelNodesVisible(int index, char visible)
{
    if (visible != '\0') {
        SceneNode_SetViewMaskTree(g_unk0x0082cb78[index].pNode, 0xff);
        SceneNode_SetViewMaskTree(g_unk0x0082cb78[index].pNode5, 0xff);
        OptionPreview_SetMeshOpacity(index);
        OptionPreview_UpdateWheelMeshVariants(index);
        return;
    }
    SceneNode_SetViewMaskTree(g_unk0x0082cb78[index].pNode, 0);
    SceneNode_SetViewMaskTree(g_unk0x0082cb78[index].pNode5, 0);
}

// Allocates and fills the per-slot wheel vertex buffers of param3: the six
// floats of every source entry become 16.16 values and its normal is stored
// twice as three signed bytes biased by 0x80, the second copy scaled to unit
// length.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00506bb0
void OptionPreview_AllocateConvertedWheelVertices(int param1, int param2, int param3)
{
    BYTE ***ppBlock;
    BYTE **ppFlags;
    unsigned short *pCounts;
    FixVector normal;
    int totalSize;
    int i;
    int k;
    int srcOff;
    int destOff;
    int length;
    int v;
    int *pDest;
    BYTE *pNormal;
    unsigned int n;
    BYTE nb[3];
    FixVector pos;

    ppBlock = &g_unk0x00831198[param1];
    *ppBlock = (BYTE **)CFileBuffer::AllocateLockedBuffer((unsigned int)*(BYTE *)(param3 + 0x26a) << 2);
    totalSize = (unsigned int)*(BYTE *)(param3 + 0x26a) << 2;
    ppFlags = &g_unk0x0082d1dc[param1];
    *ppFlags = (BYTE *)CFileBuffer::AllocateLockedBuffer((unsigned int)*(BYTE *)(param3 + 0x26a));
    pCounts = (unsigned short *)(param3 + 0x24c);
    for (i = 0; i < (int)(*(BYTE *)(param3 + 0x26a) & 0xff); i++) {
        {
            (*ppBlock)[i] = (BYTE *)CFileBuffer::AllocateLockedBuffer((unsigned int)*pCounts << 5);
            totalSize += (unsigned int)*pCounts * 0x20;
            (*ppFlags)[i] = *(BYTE *)(*(int *)(param3 + i * 4 + 0x3c) + 0x30);
            destOff = 0;
            srcOff = 0;
            for (k = 0; k < (int)(unsigned int)*pCounts; k++) {
                pDest = (int *)((*ppBlock)[i] + destOff);
                pDest[0] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff) * CGraphics::m_65536);
                pDest[1] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 4) * CGraphics::m_65536);
                pDest[2] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 8) * CGraphics::m_65536);
                pDest[3] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 0xc) * CGraphics::m_65536);
                pDest[4] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 0x10) * CGraphics::m_65536);
                pDest[5] = (int)(__int64)((double)*(float *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 0x14) * CGraphics::m_65536);
                pos = *(FixVector *)pDest;
                if (pos.x >= 0)
                    *(BYTE *)((BYTE *)pDest + 0x1b) = 0x81;
                else
                    *(BYTE *)((BYTE *)pDest + 0x1b) = 0x7f;
                if (pos.y >= 0)
                    *(BYTE *)((BYTE *)pDest + 0x1c) = 0x81;
                else
                    *(BYTE *)((BYTE *)pDest + 0x1c) = 0x7f;
                if (pos.z >= 0)
                    *(BYTE *)((BYTE *)pDest + 0x1d) = 0x81;
                else
                    *(BYTE *)((BYTE *)pDest + 0x1d) = 0x7f;
                pNormal = (BYTE *)(*(int *)(*(int *)(param3 + i * 4) + 0xc) + srcOff + 0x18);
                n = *(unsigned int *)pNormal;
                nb[0] = (BYTE)(n >> 16);
                nb[2] = *pNormal;
                nb[1] = (BYTE)(n >> 8);
                v = (nb[0] & 0xff) - 0x80;
                if (v < -0x7f)
                    v = -0x7f;
                else if (v > 0x7f)
                    v = 0x7f;
                *(BYTE *)((BYTE *)pDest + 0x1b) = (BYTE)v;
                v = (nb[1] & 0xff) - 0x80;
                if (v < -0x7f)
                    v = -0x7f;
                else if (v > 0x7f)
                    v = 0x7f;
                *(BYTE *)((BYTE *)pDest + 0x1c) = (BYTE)v;
                v = (nb[2] & 0xff) - 0x80;
                if (v < -0x7f)
                    v = -0x7f;
                else if (v > 0x7f)
                    v = 0x7f;
                *(BYTE *)((BYTE *)pDest + 0x1d) = (BYTE)v;
                normal.x = *(char *)((BYTE *)pDest + 0x1b) * -0x200;
                normal.y = *(char *)((BYTE *)pDest + 0x1c) * -0x200;
                normal.z = *(char *)((BYTE *)pDest + 0x1d) * -0x200;
                length = FixVecLength(&normal);
                if (length == 0) {
                    *(BYTE *)((BYTE *)pDest + 0x18) = 0;
                    *(BYTE *)((BYTE *)pDest + 0x19) = 0;
                    *(BYTE *)((BYTE *)pDest + 0x1a) = 0;
                } else {
                    FixVecScaleRecip(&normal, &normal, length);
                    v = normal.x >> 9;
                    if (v >= 0x80)
                        v = 0x7f;
                    else if (v < -0x7f)
                        v = -0x7f;
                    *(BYTE *)((BYTE *)pDest + 0x18) = (BYTE)v;
                    v = normal.y >> 9;
                    if (v > 0x7f)
                        v = 0x7f;
                    else if (v < -0x7f)
                        v = -0x7f;
                    *(BYTE *)((BYTE *)pDest + 0x19) = (BYTE)v;
                    v = normal.z >> 9;
                    if (v > 0x7f)
                        v = 0x7f;
                    else if (v < -0x7f)
                        v = -0x7f;
                    *(BYTE *)((BYTE *)pDest + 0x1a) = (BYTE)v;
                }
                srcOff += 0x30;
                destOff += 0x20;
            }
        }
        pCounts++;
    }
}

/* ===== integrated from casc/s4 ===== */

// Provisional copy of the globals the stage deform code shares with the other
// GameInfo lots (they are declared next to OptionPreview_LoadSkyColourRecord as well).
extern float g_oneOverRandMax;

// Per-entry stage data loaded from the entry's .c3d file (0x54 bytes): the four
// anchor matrices and the handles of the entry's meshes.


// Deform geometry of one stage entry (0x138 bytes): the box OptionPreview_BuildDeformGeometry builds
// around it, the twelve vertices OptionPreview_RebuildDeformedGeometry deforms and the position of the
// entry's four anchor matrices.

// Planar influence of the camera on the stage (all 16.16): inner radius, width
// of the falloff band and the scale of the falloff, set by OptionPreview_UpdateStageSkyColours.
extern int g_unk0x0082d150;  // defined in StageTiming.cpp
extern int g_unk0x0082d154;  // defined in StageTiming.cpp
extern int g_unk0x0082d158;  // defined in StageTiming.cpp

void StageDeform_ClampVertex(int *pPosition, int meshIndex, int vertexIndex, int *pRecord);

void CarDamage_ApplyCameraSpaceMeshDent(int *pRecord);
void OptionPreview_RebuildDeformedGeometry(Unk0x0082d220 *pObject, Unk0x0082fd00 *pGeom);

// Recomputes the three sky colours of the stage from the colour loaded by
// OptionPreview_LoadSkyColourRecord: the raw weights when the sky type is 0, a single clamped value
// for type 1 and two interpolated values for the rest. It only touches stages
// whose mesh record is filled in.
// FUNCTION: CMR2 0x005078e0
void OptionPreview_UpdateStageSkyColours(int index)
{
    int scale;

    if (g_unk0x0082d220[index].meshCount == 0)
        return;
    if (g_unk0x0082d220[index].field_0x2a8 == 0)
        return;
    switch (g_unk0x0082d14c) {
    case 0:
        g_unk0x0082d150 = FixMul(g_unk0x0082d148, 0x5999);
        g_unk0x0082d154 = FixMul(g_unk0x0082d148, 0x9999);
        g_unk0x0082d158 = FixMul(g_unk0x0082d148, 0xb333);
        OptionPreview_RebuildDeformedGeometry(&g_unk0x0082d220[index], &g_unk0x0082fd00[index]);
        return;
    case 1:
        scale = FixMul(g_unk0x0082d148, 0x8000);
        if (scale > 0x4000)
            scale = 0x4000;
        g_unk0x0082d150 = scale;
        g_unk0x0082d154 = scale;
        g_unk0x0082d158 = scale;
        CarDamage_ApplyCameraSpaceMeshDent((int *)&g_unk0x0082d220[index]);
        return;
    default:
        g_unk0x0082d144 = 0x4000;
        scale = FixMul(g_unk0x0082d148, 0x8000);
        if (scale > g_unk0x0082d144)
            scale = g_unk0x0082d144;
        g_unk0x0082d150 = scale;
        g_unk0x0082d154 = scale;
        g_unk0x0082d158 = scale;
        CarDamage_ApplyCameraSpaceMeshDent((int *)&g_unk0x0082d220[index]);
        return;
    }
}

// Builds the deform geometry of stage entry <index>: the eight corners of the
// box around it, the twelve vertices OptionPreview_RebuildDeformedGeometry deforms and the position of
// the four anchor matrices. The box extents depend on the rally the entry
// belongs to.
// FUNCTION: CMR2 0x00507a10
void OptionPreview_BuildDeformGeometry(Unk0x0082d220 *pObject, int index)
{
    Unk0x0082fd00 *pGeom;
    FixVector sizes;
    FixVector half;
    int i;
    int value;
    int offX0;
    int offX1;
    int offY0;
    int offY1;
    int offZ0;

    pGeom = &g_unk0x0082fd00[index];
    pGeom->pEntry = &g_unk0x0082cb78[index];
    pGeom->field_0x124 = 0;
    for (i = 0; i < 4; i++) {
        FixMatrix_GetPosition(&pGeom->anchor[i],
                              (FixMatrix *)(pGeom->pEntry->matrix[i] + 0x58));
        value = rand();
        pGeom->field_0x128[i] =
            FixMul((int)(__int64)((float)value * g_oneOverRandMax * CGraphics::m_65536), 0x1680000);
    }
    switch ((int)CFrontend::GetArchivePrimaryIDEntry(RallyData_GetDriverRecordSelectionValue((BYTE)index))) {
    case 3:
        sizes.x = 0x44560;
        sizes.y = 0x15eb8;
        sizes.z = 0x1cfdf;
        offX0 = 0x1cccc;
        offX1 = 0x14ccc;
        offZ0 = 0x4ccc;
        offY0 = 0xb0a3;
        offY1 = 0xfa9f;
        break;
    case 0:
        sizes.x = 0x426e9;
        sizes.y = 0x16b85;
        sizes.z = 0x1c51e;
        offX0 = 0x1cccc;
        offX1 = 0x8000;
        offZ0 = 0x3d70;
        offY0 = 0xcf5c;
        offY1 = 0x10ccc;
        break;
    case 6:
        sizes.x = 0x3e3d7;
        sizes.y = 0x15eb8;
        sizes.z = 0x1c28f;
        offX0 = 0x1ae14;
        offX1 = 0x9c28;
        offZ0 = 0x4ccc;
        offY0 = 0xcf5c;
        offY1 = 0xfae1;
        break;
    case 2:
        sizes.x = 0x40f5c;
        sizes.y = 0x163d7;
        sizes.z = 0x1c51e;
        offX0 = 0x1cccc;
        offX1 = 0x451e;
        offZ0 = 0x4ccc;
        offY0 = 0xcf5c;
        offY1 = 0x1147a;
        break;
    case 7:
        sizes.x = 0x475c2;
        sizes.y = 0x1570a;
        sizes.z = 0x1c28f;
        offX0 = 0x1e147;
        offX1 = 0x1028f;
        offZ0 = 0x4ccc;
        offY0 = 0xcf5c;
        offY1 = 0xfae1;
        break;
    case 1:
        sizes.x = 0x4451e;
        sizes.y = 0x15999;
        sizes.z = 0x1d70a;
        offX0 = 0x1cccc;
        offX1 = 0xf851;
        offZ0 = 0x570a;
        offY0 = 0xcf5c;
        offY1 = 0x1147a;
        break;
    case 8:
        sizes.x = 0x30083;
        sizes.y = 0x14041;
        sizes.z = 0x18000;
        offX0 = 0x13333;
        offX1 = 0x4ccc;
        offZ0 = 0x2666;
        offY0 = 0xb5c2;
        offY1 = 0xfa9f;
        break;
    case 5:
        sizes.x = 0x41c28;
        sizes.y = 0x154bc;
        sizes.z = 0x1d47a;
        offX0 = 0x1c000;
        offX1 = 0x10000;
        offZ0 = 0x4ccc;
        offY0 = 0xe3d7;
        offY1 = 0x12dd2;
        break;
    case 4:
        sizes.x = 0x40312;
        sizes.y = 0x14ccc;
        sizes.z = 0x1c51e;
        offX0 = 0x1c000;
        offX1 = 0xcccc;
        offZ0 = 0x4ccc;
        offY0 = 0xe3d7;
        offY1 = 0x12dd2;
        break;
    case 9:
        sizes.x = 0x3b958;
        sizes.y = 0x15db2;
        sizes.z = 0x1e041;
        offX0 = 0x1a666;
        offX1 = 0x9999;
        offZ0 = 0x4ccc;
        offY0 = 0xe3d7;
        offY1 = 0xe106;
        break;
    case 11:
        sizes.x = 0x3d333;
        sizes.y = 0x15999;
        sizes.z = 0x1c312;
        offX0 = 0x1a666;
        offX1 = 0x9999;
        offZ0 = 0x4ccc;
        offY0 = 0xe3d7;
        offY1 = 0xe106;
        break;
    case 10:
        sizes.x = 0x3b333;
        sizes.y = 0x106a7;
        sizes.z = 0x1cf5c;
        offX0 = 0x1a666;
        offX1 = 0x13333;
        offZ0 = 0x4ccc;
        offY0 = 0xca3d;
        offY1 = 0xe106;
        break;
    case 12:
        sizes.x = 0x3ec49;
        sizes.y = 0x146a7;
        sizes.z = 0x1c28f;
        offX0 = 0x1a666;
        offX1 = 0x13333;
        offZ0 = 0x4ccc;
        offY0 = 0xca3d;
        offY1 = 0xe106;
        break;
    case 13:
        sizes.x = 0x41687;
        sizes.y = 0x16147;
        sizes.z = 0x1bb22;
        offX0 = 0x1ae14;
        offX1 = 0xfd70;
        offZ0 = 0x428f;
        offY0 = 0xd70a;
        offY1 = 0xf581;
        break;
    }
    FixVecScale(&half, &sizes, 0x8000);
    pGeom->corner[1].x = half.x;
    pGeom->corner[1].y = -half.y;
    pGeom->corner[1].z = -half.z;
    pGeom->corner[0].x = half.x;
    pGeom->corner[0].y = -half.y;
    pGeom->corner[0].z = half.z;
    pGeom->corner[2].x = -half.x;
    pGeom->corner[2].y = -half.y;
    pGeom->corner[2].z = half.z;
    pGeom->corner[3].x = -half.x;
    pGeom->corner[3].y = -half.y;
    pGeom->corner[3].z = -half.z;
    pGeom->corner[5].x = half.x;
    pGeom->corner[5].y = half.y;
    pGeom->corner[5].z = -half.z;
    pGeom->corner[4].x = half.x;
    pGeom->corner[4].y = half.y;
    pGeom->corner[4].z = half.z;
    pGeom->corner[6].x = -half.x;
    pGeom->corner[6].y = half.y;
    pGeom->corner[6].z = half.z;
    pGeom->corner[7].x = -half.x;
    pGeom->corner[7].y = half.y;
    pGeom->corner[7].z = -half.z;
    pGeom->vertex[0].x = pObject->field_0x21c;
    pGeom->vertex[0].y = -half.y;
    pGeom->vertex[0].z = pObject->field_0x224;
    pGeom->vertex[1].x = pObject->field_0x21c;
    pGeom->vertex[1].y = -half.y;
    pGeom->vertex[1].z = pObject->field_0x228;
    pGeom->vertex[2].x = pObject->field_0x220;
    pGeom->vertex[2].y = -half.y;
    pGeom->vertex[2].z = pObject->field_0x224;
    pGeom->vertex[3].x = pObject->field_0x220;
    pGeom->vertex[3].y = -half.y;
    pGeom->vertex[3].z = pObject->field_0x228;
    pGeom->vertex[4].x = pObject->field_0x21c;
    pGeom->vertex[4].y = offY0 - half.y;
    pGeom->vertex[4].z = pObject->field_0x224;
    pGeom->vertex[5].x = pObject->field_0x21c;
    pGeom->vertex[5].y = offY0 - half.y;
    pGeom->vertex[5].z = pObject->field_0x228;
    pGeom->vertex[6].x = pObject->field_0x220;
    pGeom->vertex[6].y = offY1 - half.y;
    pGeom->vertex[6].z = pObject->field_0x224;
    pGeom->vertex[7].x = pObject->field_0x220;
    pGeom->vertex[7].y = offY1 - half.y;
    pGeom->vertex[7].z = pObject->field_0x228;
    pGeom->vertex[8].x = half.x - offX0;
    pGeom->vertex[8].y = half.y;
    pGeom->vertex[8].z = offZ0 - half.z;
    pGeom->vertex[9].x = half.x - offX0;
    pGeom->vertex[9].y = half.y;
    pGeom->vertex[9].z = half.z - offZ0;
    pGeom->vertex[10].x = offX1 - half.x;
    pGeom->vertex[10].y = half.y;
    pGeom->vertex[10].z = half.z - offZ0;
    pGeom->vertex[11].x = offX1 - half.x;
    pGeom->vertex[11].y = half.y;
    pGeom->vertex[11].z = offZ0 - half.z;
}

// Rebuilds the deformed geometry of the stage entry <pObject> refers to: the
// twelve vertices OptionPreview_BuildDeformGeometry generated are used to find the vertex closest to
// the camera on either side of the camera plane, the camera is pushed onto that
// plane and then every vertex of the entry's meshes is moved along its stored
// limit normal (clamped by StageDeform_ClampVertex) with three times the
// displacement the clamp applied.
// FUNCTION: CMR2 0x00507fe0
void OptionPreview_RebuildDeformedGeometry(Unk0x0082d220 *pObject, Unk0x0082fd00 *pGeom)
{
    FixVector d;
    FixVector dv;
    FixVector pos;
    FixVector dest;
    FixVector saved;
    int positive;
    int minValue;
    int minPositive;
    int radius2;
    int invRadius;
    int band2;
    int invBand;
    int falloff;
    int value;
    int band;
    int angle;
    int dirty;
    int i;
    int j;

    FixVecScale(&d, &g_unk0x0082d120, -0x10000);
    positive = FixVecDot(&g_unk0x0082d12c, &d) >= 0;
    minPositive = 0;
    minValue = 0;
    for (i = 0; i < 12; i++) {
        d.x = pGeom->vertex[i].x - g_unk0x0082d120.x;
        d.y = pGeom->vertex[i].y - g_unk0x0082d120.y;
        d.z = pGeom->vertex[i].z - g_unk0x0082d120.z;
        value = FixVecDot(&g_unk0x0082d12c, &d);
        if (!positive)
            value = -value;
        if (value < minValue)
            minValue = value;
        if (value > 0 && (minPositive == 0 || value < minPositive))
            minPositive = value;
    }
    if (!positive) {
        minValue = -minValue;
        minPositive = -minPositive;
    }
    if (minValue != 0) {
        FixVecScale(&d, &g_unk0x0082d12c, minValue);
        g_unk0x0082d120.x += d.x;
        g_unk0x0082d120.y += d.y;
        g_unk0x0082d120.z += d.z;
    } else if (minPositive != 0) {
        FixVecScale(&d, &g_unk0x0082d12c, minPositive);
        g_unk0x0082d120.x += d.x;
        g_unk0x0082d120.y += d.y;
        g_unk0x0082d120.z += d.z;
    }
    positive = FixVecDot(&g_unk0x0082d12c, &g_unk0x0082d120) >= 0;
    radius2 = FixMul(g_unk0x0082d150, g_unk0x0082d150);
    invRadius = FixDiv(0x10000, g_unk0x0082d150);
    band = g_unk0x0082d150 + g_unk0x0082d154;
    band2 = FixMul(band, band);
    invBand = FixDiv(0x10000, g_unk0x0082d154);
    falloff = FixMul(g_unk0x0082d158, 0x3333);
    for (j = 0; j < pObject->meshCount; j++) {
        dirty = 0;
        for (i = 0; i < pObject->vertexCount[j]; i++) {
            pos.x = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].x *
                                   CGraphics::m_65536);
            pos.y = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].y *
                                   CGraphics::m_65536);
            pos.z = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].z *
                                   CGraphics::m_65536);
            dv.x = g_unk0x0082d120.x - pos.x;
            dv.y = g_unk0x0082d120.y - pos.y;
            dv.z = g_unk0x0082d120.z - pos.z;
            value = FixVecDot(&dv, &g_unk0x0082d12c);
            value = FixMul(value, value);
            if (value > band2)
                continue;
            saved = pos;
            if (value <= radius2) {
                value = g_unk0x0082d150 - FixMul(invRadius, value);
                FixVecScale(&dv, &g_unk0x0082d12c, value);
                if (positive) {
                    pos.x -= dv.x;
                    pos.y -= dv.y;
                    pos.z -= dv.z;
                } else {
                    pos.x += dv.x;
                    pos.y += dv.y;
                    pos.z += dv.z;
                }
            } else {
                value = FixMul(FixSqrt(value) - g_unk0x0082d150, invBand);
                value = FixMul(value, falloff);
                angle = dv.x + dv.z;
                if (angle < 0)
                    angle = -angle;
                angle &= 0xffffff80;
                angle %= 1024;
                angle <<= 6;
                if (angle < 0x8000)
                    angle -= 0x10000;
                value = FixMul(value, angle);
                dest.x = (int)(signed char)pObject->pVertexData[j][i].field_0x18[0] << 9;
                dest.y = (int)(signed char)pObject->pVertexData[j][i].field_0x18[1] << 9;
                dest.z = (int)(signed char)pObject->pVertexData[j][i].field_0x18[2] << 9;
                FixVecScale(&dv, &dest, value);
                pos.x += dv.x;
                pos.y += dv.y;
                pos.z += dv.z;
            }
            StageDeform_ClampVertex(&pos.x, j, i, (int *)pObject);
            dest.x = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].nx *
                                    CGraphics::m_65536);
            dest.y = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].ny *
                                    CGraphics::m_65536);
            dest.z = (int)(__int64)(((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].nz *
                                    CGraphics::m_65536);
            d.x = pos.x - saved.x;
            d.y = pos.y - saved.y;
            d.z = pos.z - saved.z;
            FixVecScale(&d, &d, 0x30000);
            dest.x += d.x;
            dest.y += d.y;
            dest.z += d.z;
            ((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].nx =
                (float)(dest.x * CGraphics::m_oneOver65536);
            ((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].ny =
                (float)(dest.y * CGraphics::m_oneOver65536);
            ((Unk0x0082d220VertexF *)pObject->pMeshes[j]->pVertexData)[i].nz =
                (float)(dest.z * CGraphics::m_oneOver65536);
            dirty = 1;
        }
        if (dirty && pObject->pNodes[j]->pObject != 0)
            Mesh_Rebuild((Mesh *)pObject->pNodes[j]->pObject);
    }
}
// Copies the converted vertex buffers of the slot into its per-mesh pointer
// array: for every source mesh it finds the first buffer whose stored flag
// matches the mesh's flag and points the mesh entry at it.
// FUNCTION: CMR2 0x00506fc0
void OptionPreview_AssignConvertedVertexBuffers(int param1, int param2, int param3)
{
    int i;
    int j;
    int match;
    int target;

    OptionPreview_AllocateConvertedWheelVertices(param1, param2, param3);
    for (i = 0; i < *(BYTE *)(param3 + 0x26a); i++) {
        j = 0;
        match = -1;
        target = *(int *)(*(int *)(param3 + 0x3c + i * 4) + 0x30) & 0xff;
        for (; j <= *(BYTE *)(param3 + 0x26a); j++) {
            if (g_unk0x0082d1dc[param1][j] == target) {
                match = j;
                j = *(BYTE *)(param3 + 0x26a);
            }
        }
        if (match >= 0)
            *(BYTE **)(param3 + 0x78 + i * 4) = g_unk0x00831198[param1][match];
    }
    *(int *)(param3 + 0x2a8) = 1;
}

// Returns one entry of the pointer table used by the mode selection screens.
// FUNCTION: CMR2 0x004d06f0
char *NetworkChat_GetLine(int index)
{
    return g_unk0x00817c84[index];
}

// Steps the shared mode value one position; when it reaches the top it stores
// the frame time instead and flags the fade as finished.
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00500130
void OptionMenu_AdvanceSharedModeValue(void)
{
    Unk0049c2c0 *p = Game_GetSecondaryCallbackMachine();
    int value = OptionMenu_GetPlayerSlotOffset() - 1;

    if (value != 0) {
        CGame::PromoteCallbackEntryByRule(p, 0, 1, 2);
    } else {
        value = 1;
        CGame::PromoteCallbackEntryByRule(p, 0, 0, 2);
        g_unk0x0082a938 = 0;
        g_unk0x0082b0a4 = CMain::GetFrameDelta();
    }
    OptionMenu_SetPlayerSlotOffset(value);
}

// Starts the animation of the shared value towards one (or towards zero when it
// is already at one).
// FUNCTION: CMR2 0x00503960
void OptionMenu_ToggleValueAnimation(int param_1, int param_2)
{
    FixInterp *p;

    if (g_unk0x0082ca1c == 0xff)
        return;
    p = (FixInterp *)&g_unk0x0082c6c8[(signed char)g_unk0x0082ca1c];
    if (param_1 != 0) {
        if (p->end != 0x10000)
            FixInterp_StartToOne(p);
        return;
    }
    if (param_2 != 0) {
        if (p->end != 0) {
            FixInterp_StartToZero(p);
            return;
        }
    } else {
        if (p->end == 0) {
            FixInterp_StartToOne(p);
            return;
        }
        if (p->end == 0x10000)
            FixInterp_StartToZero(p);
    }
}

// Stores one option value of a slot into the working table and, the first time
// the slot is touched, adds the option's weight to its interpolation distance.
// FUNCTION: CMR2 0x00502670
void OptionMenu_SetWorkingValue(int param_1, int param_2, char param_3)
{
    int distance;

    switch (param_2) {
    case 0:
        g_unk0x0082bee8[param_1][0] = param_3;
        break;
    case 1:
        g_unk0x0082bee8[param_1][1] = param_3;
        break;
    case 6:
        g_unk0x0082bee8[param_1][6] = param_3 * 10;
        break;
    case 2:
        g_unk0x0082bee8[param_1][2] = param_3 * 10;
        break;
    case 3:
        g_unk0x0082bee8[param_1][3] = param_3 * 10;
        break;
    case 4:
        g_unk0x0082bee8[param_1][4] = param_3 * 10;
        break;
    case 5:
        g_unk0x0082bee8[param_1][5] = param_3 * 10;
        break;
    }
    if (g_unk0x0082bf20[param_1][param_2] == 0) {
        distance = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, distance - g_unk0x00527098[param_2]);
        g_unk0x0082bf20[param_1][param_2] = 1;
    }
}

// Restores one option value of a slot from the default table and subtracts the
// option's weight from its interpolation distance.
// FUNCTION: CMR2 0x00502790
void OptionMenu_RestoreDefaultValue(int param_1, int param_2)
{
    int distance;

    switch (param_2) {
    case 0:
        g_unk0x0082bee8[param_1][0] = g_unk0x0082bf04[param_1 * 7];
        break;
    case 1:
        g_unk0x0082bee8[param_1][1] = g_unk0x0082bf04[param_1 * 7 + 1];
        break;
    case 6:
        g_unk0x0082bee8[param_1][6] = g_unk0x0082bf04[param_1 * 7 + 6];
        break;
    case 2:
        g_unk0x0082bee8[param_1][2] = g_unk0x0082bf04[param_1 * 7 + 2];
        break;
    case 3:
        g_unk0x0082bee8[param_1][3] = g_unk0x0082bf04[param_1 * 7 + 3];
        break;
    case 4:
        g_unk0x0082bee8[param_1][4] = g_unk0x0082bf04[param_1 * 7 + 4];
        break;
    case 5:
        g_unk0x0082bee8[param_1][5] = g_unk0x0082bf04[param_1 * 7 + 5];
        break;
    }
    OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x00527098[param_2]);
    g_unk0x0082bf20[param_1][param_2] = 0;
}

// Tells whether a slot option still holds the given value (options 2..6 store
// the value multiplied by ten).
// FUNCTION: CMR2 0x00502a00
BYTE OptionMenu_DoesValueMatch(int param_1, int param_2, int param_3)
{
    switch (param_2) {
    case 0:
        return g_unk0x0082bf04[param_1 * 7] == param_3;
    case 1:
        return g_unk0x0082bf04[param_1 * 7 + 1] == param_3;
    case 6:
        return g_unk0x0082bf04[param_1 * 7 + 6] == param_3 * 10;
    case 2:
        return g_unk0x0082bf04[param_1 * 7 + 2] == param_3 * 10;
    case 3:
        return g_unk0x0082bf04[param_1 * 7 + 3] == param_3 * 10;
    case 4:
        return g_unk0x0082bf04[param_1 * 7 + 4] == param_3 * 10;
    case 5:
        return g_unk0x0082bf04[param_1 * 7 + 5] == param_3 * 10;
    }
    return 0;
}

// Runs the callback of every enabled slot: the table is 0x14 bytes per slot and
// each slot owns 8 of them.
void OptionPreview_LoadStageGeometryRecord(unsigned int);

// FUNCTION: CMR2 0x0050a3c0
void OptionPreview_UpdateAllSlots(void)
{
    int i = 0;

    if ((unsigned int)CGameInfo::GetConfiguredPlayerCount() > 0) {
        do {
            OptionPreview_LoadStageGeometryRecord(i);
            i++;
        } while (i < CGameInfo::GetConfiguredPlayerCount());
    }
}

// Moves the left or the right edge of the slot's layout rectangle towards the
// centre while its interpolation is running.
// FUNCTION: CMR2 0x00501de0
void OptionMenu_AnimateLayoutRectangleEdge(int param_1, short *param_2)
{
    Unk0x0082b2c0 *p;
    int lo;
    int hi;
    int a;
    int delta;

    p = &g_unk0x0082b2c0[param_1];
    if (p->field_0xc == 1) {
        lo = param_2[2] / 2 + param_2[0];
        hi = lo + 1;
        lo--;
        if (p->field_0x0 < 0x8000) {
            param_2[0] = lo;
            param_2[2] = hi - lo;
            a = FixMul(p->field_0x0, 0x20000);
            a = FixMul(a, a);
            delta = param_2[3] - (FixMul(param_2[3] << 16, a) >> 16);
            param_2[1] += delta / 2;
            param_2[3] -= delta;
            return;
        }
        a = FixMul(p->field_0x0 - 0x8000, 0x20000);
        a = FixMul(a, a);
        delta = param_2[2] - (FixMul(param_2[2] << 16, a) >> 16);
        param_2[0] += delta / 2;
        param_2[2] -= delta;
        return;
    } else if (p->field_0xc == 0) {
        param_2[3] = 0;
        param_2[2] = 0;
    }
}

// Records the viewport of the active menu background (top-left origin and two
// thirds of the current resolution) in the shared rectangle.
// FUNCTION: CMR2 0x00501710
void OptionMenu_SetBackgroundViewport(void)
{
    short view[4];

    view[0] = 0;
    view[1] = 0;
    view[2] = (short)((int)g_pGraphics->resX * 2 / 3);
    view[3] = (short)((int)g_pGraphics->resY * 2 / 3);
    Game_DrawSceneViewport(g_unk0x0082b1b4, g_unk0x0082b1b0, view, 0, 0);
}

// Restarts the shared value animation from zero (unused callback parameters).
// FUNCTION: CMR2 0x00500100
void OptionMenu_ResetValueAnimation(int param1, int param2)
{
    OptionMenu_ToggleValueAnimation(0, 0);
}

// Colour pairs of the results screen: the second pointer (0x5297b4) starts two
// entries into the first table.
// GLOBAL: CMR2 0x005297ac
int g_unk0x005297ac[6] = { -11579569, 1078939471, -1, -11579569, 538976288, 0 };
// GLOBAL: CMR2 0x005297c4
char g_str0x005297c4[] = "%s: %.2d.00";

// Draws the four result rows of the screen: the X coordinate and the viewport
// are fixed, the rows advance 14 units each.
// FUNCTION: CMR2 0x0050fd30
void OptionMenu_DrawResultRows(int param_1)
{
    int x = (int)g_pGraphics->resX * 0x1c / 0x280;
    int y = 0x19a;
    int index = param_1 * 4 + 0xd9;
    int count = 4;

    do {
        OptionMenu_DrawTransitionTextShortCoords(5, 1, 1, CFrontend::GetTextString(index), x,
                     (int)g_pGraphics->resY * y / 0x1e0, g_unk0x005297ac,
                     g_unk0x005297ac + 2, 0x11);
        y += 0xe;
        index++;
    } while (--count);
}

// Startup (.CRT$XCU) initializer: caches the render resolution in the globals
// the 2D drawing code reads.
// FUNCTION: CMR2 0x0050fdd0
void __cdecl OptionMovie_InitResolution(void)
{
    g_unk0x00831c60 = g_pGraphics->resX;
    g_unk0x00831c64 = g_pGraphics->resY;
}

// Startup (.CRT$XCU) initializer registered in the startup table. The linker
// folds it into a jump to the identical initializer above (both are reached).
// FUNCTION: CMR2 0x0050fdc0
void __cdecl OptionMovie_InitResolutionAlias(void)
{
    OptionMovie_InitResolution();
}

// PORT: was a function pointer in MSVC's .CRT$XCU startup table; standard
// C++ dynamic initialisation runs it at startup.
static int s_resolutionInit = (OptionMovie_InitResolutionAlias(), 0);

// Draws the title of the screen: the text table string is formatted into the
// shared buffer, right aligned at 0x140/0x280 of the resolution.
// FUNCTION: CMR2 0x0050e1c0
void OptionMenu_DrawScreenTitle(void)
{
    int flag = 0;
    int width;

    if (CGameInfo::GetActiveOptionSlot() == 1) {
        if (CGameInfo::IsConfiguredMultiplayer() == 0)
            flag = 1;
    }
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x131));
    Font_Unused((int)CFrontend::m_stringDest, flag);
    width = Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
    OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::m_stringDest,
                 (int)g_pGraphics->resX * 0x140 / 0x280 - width / 2,
                 (int)g_pGraphics->resY * 0xf0 / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
}

// Moves the shared value one position up; clamps it to the option count.
// FUNCTION: CMR2 0x00500210
void OptionMenu_IncrementSharedValue(Menu *pMenu, char param)
{
    Unk0049c2c0 *p;
    int value;

    if (param == 0)
        return;
    p = Game_GetSecondaryCallbackMachine();
    value = OptionMenu_GetPlayerSlotOffset() + 1;
    if (value > CGameInfo::GetConfiguredPlayerCount())
        value = CGameInfo::GetConfiguredPlayerCount();
    else
        CGame::PromoteCallbackEntryByRule(p, 0, 1, 2);
    OptionMenu_SetPlayerSlotOffset(value);
}

// Applies the option selected in the first item of the mode menu: restores the
// old value when it still matches, otherwise stores the new one.
// FUNCTION: CMR2 0x005003d0
void OptionMenu_ApplyFirstModeItem(Menu *pMenu, int param)
{
    int index = Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1);
    int value = OptionMenu_GetColumnUnitId(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max);
    int option = pMenu->items[0].max;

    if (OptionMenu_GetColumnDirtyFlag(CGameInfo::GetActiveOptionSlot(), value) &&
        OptionMenu_DoesValueMatch(CGameInfo::GetActiveOptionSlot(), value, option)) {
        OptionMenu_RestoreDefaultValue(CGameInfo::GetActiveOptionSlot(), value);
        Menu_SetNextAction((int)pMenu->pParent);
        return;
    }
    if (!OptionMenu_DoesValueMatch(CGameInfo::GetActiveOptionSlot(), value, option)) {
        if (OptionMenu_GetSlotSelector(CGameInfo::GetActiveOptionSlot()) - OptionMenu_GetColumnBaseWeight(value) < 0 &&
            !OptionMenu_GetColumnDirtyFlag(CGameInfo::GetActiveOptionSlot(), value))
            return;
        OptionMenu_SetWorkingValue(CGameInfo::GetActiveOptionSlot(), value, option);
    }
    Menu_SetNextAction((int)pMenu->pParent);
}

// Colour pair used by the highlighted option rows and the value at 0x52737c.
// GLOBAL: CMR2 0x00527378
int g_unk0x00527378 = -11250490;
// GLOBAL: CMR2 0x0052737c
int g_unk0x0052737c = -1;
// X multiplier of the option menu layout (450/640 of the resolution).
// GLOBAL: CMR2 0x005293a0
int g_unk0x005293a0 = 0x1c2;

// Draws the four option rows of the menu's right column: the first one uses the
// normal font, the other three the highlighted one.
// FUNCTION: CMR2 0x0050a680
void OptionMenu_DrawRightColumn(void)
{
    OptionMenu_DrawTransitionTextShortCoords(3, 0, 0,
                 CFrontend::GetTextString((int)CFrontend::GetArchivePrimaryIDEntry(
                     RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot())) * 4 + 0x4c),
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0xb4 / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 0,
                 CFrontend::GetTextString((int)CFrontend::GetArchivePrimaryIDEntry(
                     RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot())) * 4 + 0x4d),
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0xc2 / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 0,
                 CFrontend::GetTextString((int)CFrontend::GetArchivePrimaryIDEntry(
                     RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot())) * 4 + 0x4e),
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0xd0 / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 0,
                 CFrontend::GetTextString((int)CFrontend::GetArchivePrimaryIDEntry(
                     RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot())) * 4 + 0x4f),
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0xde / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
}

// GLOBAL: CMR2 0x0051a904
char g_str0x0051a904[] = "%.2d:%.2d";
// GLOBAL: CMR2 0x005295f0
char g_str0x005295f0[] = " (mins)";

// Draws the stage time of the results screen: the elapsed time clamped to one
// minute around the target, the time of the target and the "(mins)" suffix.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0050a3f0
void OptionMenu_DrawClampedStageTime(void)
{
    int value = OptionMenu_GetSlotSelector(CGameInfo::GetActiveOptionSlot());
    int target = OptionMenu_GetSharedValue();
    int shown;
    int width;
    int x;

    if (target > value) {
        shown = target - 6000;
        if (shown < value)
            shown = value;
    } else if (target < value) {
        shown = target + 6000;
        if (shown > value)
            shown = value;
    } else {
        shown = value;
    }
    OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::GetTextString(0x90),
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0x5d / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x91),
            OptionMenu_GetSlotBaseSelector(CGameInfo::GetActiveOptionSlot()));
    OptionMenu_DrawTransitionTextShortCoords(4, 1, 0, CFrontend::m_stringDest,
                 g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                 (int)g_pGraphics->resY * 0x6d / 0x1e0, g_unk0x00527380, g_unk0x0052738c, 0x11);
    OptionMenu_SetSharedValue(shown);
    sprintf(CFrontend::m_stringDest, g_str0x0051a904, (shown / 100) / 0x3c, (shown / 100) % 0x3c);
    if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
        OptionMenu_DrawTransitionTextShortCoords(4, 3, 3, CFrontend::m_stringDest,
                     g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                     (int)g_pGraphics->resY * 0x96 / 0x1e0,
                     &g_unk0x00527378, &g_unk0x00527378, 0x11);
    else
        OptionMenu_DrawTransitionTextShortCoords(4, 3, 3, CFrontend::m_stringDest,
                     g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280,
                     (int)g_pGraphics->resY * 0x96 / 0x1e0,
                     &g_unk0x0052737c, &g_unk0x0052738c[1], 0x11);
    width = Font_GetTextWidth(3, (BYTE *)CFrontend::m_stringDest);
    x = g_unk0x005293a0 * (int)g_pGraphics->resX;
    sprintf(CFrontend::m_stringDest, g_str0x005295f0);
    if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
        OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::m_stringDest, x / 0x280 + width,
                     (int)g_pGraphics->resY * 0x90 / 0x1e0,
                     &g_unk0x00527378, &g_unk0x00527378, 0x11);
    else
        OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::m_stringDest, x / 0x280 + width,
                     (int)g_pGraphics->resY * 0x90 / 0x1e0,
                     &g_unk0x0052737c, &g_unk0x0052738c[1], 0x11);
}

// 13-byte entry of the option record's 0x104-byte block at +0x0.
struct Unk0x0082c070Item {
    BYTE field_0x0[0xd];
};

// GLOBAL: CMR2 0x005270b8
int g_unk0x005270b8[11] = { 0x17700, 0xbb80, 0xbb80, 0x11940, 0xea60, 0xea60,
                            0x14820, 0x8ca0, 0x5dc0, 0xbb80, 0xbb80 };

// Copies one group of fields of a rally data record into the working option
// record of the given index and adds the group's weight to its value; clears the
// group's "already applied" flag.
// match 39%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x005034f0
void OptionMenu_ApplyRallyRecordGroup(int param_1, int param_2)
{
    BYTE *pDest = g_unk0x0082c070 + param_1 * 0x148;
    BYTE *pSrc = (BYTE *)RallyData_GetDriverGroupRecord(param_1);
    int value;
    int i;

    switch (param_2) {
    case 1:
        pDest[0x116] = pSrc[0x116];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[0]);
        g_unk0x0082c040[param_1][1] = 0;
        return;
    case 2:
        pDest[0x117] = pSrc[0x117];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[1]);
        g_unk0x0082c040[param_1][2] = 0;
        return;
    case 3:
        pDest[0x10e] = pSrc[0x10e];
        pDest[0x10f] = pSrc[0x10f];
        pDest[0x110] = pSrc[0x110];
        pDest[0x111] = pSrc[0x111];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[2]);
        OptionPreview_UpdateSkyFadeShape(param_1);
        g_unk0x0082c040[param_1][3] = 0;
        return;
    case 4:
        pDest[0x118] = pSrc[0x118];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[3]);
        g_unk0x0082c040[param_1][4] = 0;
        return;
    case 5:
        pDest[0x112] = pSrc[0x112];
        pDest[0x113] = pSrc[0x113];
        pDest[0x114] = pSrc[0x114];
        pDest[0x115] = pSrc[0x115];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[4]);
        g_unk0x0082c040[param_1][5] = 0;
        return;
    case 6:
        pDest[0x119] = pSrc[0x119];
        pDest[0x11a] = pSrc[0x11a];
        pDest[0x11b] = pSrc[0x11b];
        pDest[0x11c] = pSrc[0x11c];
        pDest[0x11d] = pSrc[0x11d];
        pDest[0x11e] = pSrc[0x11e];
        pDest[0x122] = pSrc[0x122];
        pDest[0x123] = pSrc[0x123];
        pDest[0x124] = pSrc[0x124];
        pDest[0x125] = pSrc[0x125];
        pDest[0x126] = pSrc[0x126];
        pDest[0x127] = pSrc[0x127];
        pDest[0x128] = pSrc[0x128];
        pDest[0x129] = pSrc[0x129];
        for (i = 0; i < 3; i++)
            *(int *)(pDest + 0x13c + i * 4) = *(int *)(pSrc + 0x13c + i * 4);
        for (i = 0; i < 4; i++)
            *(int *)(pDest + 0x12c + i * 4) = *(int *)(pSrc + 0x12c + i * 4);
        pDest[0x104] = pSrc[0x104];
        pDest[0x105] = pSrc[0x105];
        for (i = 0; i < 20; i++) {
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[0] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[0];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[1] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[1];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[2] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[2];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[3] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[3];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[4] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[4];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[5] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[5];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[6] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[6];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[7] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[7];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[8] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[8];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[9] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[9];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[10] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[10];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[11] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[11];
            ((Unk0x0082c070Item *)pDest)[i].field_0x0[12] = ((Unk0x0082c070Item *)pSrc)[i].field_0x0[12];
        }
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[5]);
        g_unk0x0082c040[param_1][6] = 0;
        OptionPreview_ApplyStageDeformations(param_1);
        return;
    case 7:
        pDest[0x10c] = pSrc[0x10c];
        pDest[0x10d] = pSrc[0x10d];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[6]);
        g_unk0x0082c040[param_1][7] = 0;
        return;
    case 8:
        pDest[0x11f] = pSrc[0x11f];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[7]);
        g_unk0x0082c040[param_1][8] = 0;
        return;
    case 9:
        pDest[0x120] = pSrc[0x120];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[8]);
        g_unk0x0082c040[param_1][9] = 0;
        return;
    case 10:
        pDest[0x121] = pSrc[0x121];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[9]);
        g_unk0x0082c040[param_1][10] = 0;
        return;
    case 11:
        pDest[0x108] = pSrc[0x108];
        pDest[0x109] = pSrc[0x109];
        pDest[0x10a] = pSrc[0x10a];
        pDest[0x10b] = pSrc[0x10b];
        OptionMenu_SetSlotSelector(param_1, OptionMenu_GetSlotSelector(param_1) + g_unk0x005270b8[10]);
        OptionPreview_UnpackColourFractions(param_1);
        g_unk0x0082c040[param_1][11] = 0;
    }
}

// Applies the second item of the mode menu when its value still differs from the
// stored option; the parent menu is always left as the next action.
// FUNCTION: CMR2 0x00500360
void OptionMenu_ApplySecondModeItem(Menu *pMenu, int param)
{
    int index = Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2);

    if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(),
                     OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max))) {
        OptionMenu_ApplyRallyRecordGroup(CGameInfo::GetActiveOptionSlot(),
                     OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max));
    }
    Menu_SetNextAction((int)pMenu->pParent);
}

// Callback of the option menu: applies the highlighted item when the confirm
// button was pressed this frame.
// FUNCTION: CMR2 0x004ff630
void OptionMenu_ConfirmHighlightedItem(Menu *pMenu)
{
    int slot = 0;
    int index;
    int value;
    int option;

    if (CGameInfo::GetActiveOptionSlot() == 1) {
        if (CGameInfo::IsConfiguredMultiplayer() == 0)
            slot = 1;
    }
    if ((CInput::GetAvailableDeviceRecord(slot)->field_0x8 & 0x20) == 0)
        return;
    if (pMenu->cursor == Menu_FindItem(pMenu, 0)) {
        OptionMenu_ToggleValueAnimation(0, 1);
        return;
    }
    if (pMenu->cursor == Menu_FindItem(pMenu, 2)) {
        index = Menu_FindItem(pMenu, 2);
        option = pMenu->items[index].max;
        if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(option)))
            OptionMenu_ApplyRallyRecordGroup(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(option));
    } else if (pMenu->cursor == Menu_FindItem(pMenu, 1)) {
        index = Menu_FindItem(pMenu, 1);
        value = pMenu->items[index].max;
        value = OptionMenu_GetColumnUnitId(value);
        if (OptionMenu_GetColumnDirtyFlag(CGameInfo::GetActiveOptionSlot(), value))
            OptionMenu_RestoreDefaultValue(CGameInfo::GetActiveOptionSlot(), value);
    }
}

#include "Sprite.h"

extern int g_unk0x0082ca20[9];
extern int g_unk0x0082c6bc;
extern int g_unk0x0082c698[6];
extern BYTE g_unk0x005270e4[8];
extern int g_unk0x005270fc;
extern int g_unk0x00527100;
extern int g_unk0x005270b8[11];
void OptionMenu_DrawTransitionTextShortCoords(int index, int font1, int font2, char *text, short x, short y, int *pColour1,
                  int *pColour2, unsigned int flags);

// Vertical offsets subtracted from the flag row when the screen is either too
// narrow for 1024x768 or the row is scrolled (per country).
// GLOBAL: CMR2 0x00527254
BYTE g_unk0x00527254[9] = { 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d };
// GLOBAL: CMR2 0x00527260
BYTE g_unk0x00527260[9] = { 0x3a, 0x3a, 0x3b, 0x39, 0x39, 0x39, 0x3a, 0x3b, 0x3c };

// Draws the country flag row of the championship screen: a row of buttons
// whose width depends on the language and the selected country, holding the
// country name and clipped to the screen.
// FUNCTION: CMR2 0x005057e0
void OptionMenu_DrawCountryFlags(void)
{
    SpriteRect dest;
    Texture *pTexture;
    int i;
    int width;
    int y;

    if (CGameInfo::GetGameLanguage() == 4)
        width = (BYTE)RallyDataCountryIndex() != 3 ? 0x139 : 0x148;
    else if (CGameInfo::GetGameLanguage() == 3)
        width = 0x140;
    else if (CGameInfo::GetGameLanguage() == 1 && (BYTE)RallyDataCountryIndex() == 3)
        width = 0x150;
    else
        width = 0x136;
    width = width * (int)g_pGraphics->resX / 0x280;
    pTexture = (Texture *)g_unk0x0082ca20[0];
    dest.x = (short)width - pTexture->width / 2;
    dest.w = pTexture->width;
    dest.h = pTexture->height;
    for (i = 0; i < g_unk0x0082c6bc; i++) {
        int num = 0xea0000;
        int scale = FixDiv(num, g_unk0x0082c6bc * 0x10000 - 0x10000);
        int rowY = (FixMul(i << 16, scale) >> 16) + 0x93;
        OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::GetTextString(g_unk0x0082c698[i] + 0x9d), width,
                     (int)g_pGraphics->resY * rowY / 0x1e0, &g_unk0x005270fc,
                     &g_unk0x00527100, 0x22);
        dest.y = (short)((int)g_pGraphics->resY * (rowY - 0x12) / 0x1e0);
        if (CGameInfo::GetScreenWidth() < 0x400 || !CFrontend::IsTextureWidthSupported(0x400) ||
            !CFrontend::IsTextureHeightSupported(0x400))
            dest.y = dest.y - g_unk0x00527254[g_unk0x0082c698[i]];
        else
            dest.y = dest.y - g_unk0x00527260[g_unk0x0082c698[i]];
        pTexture = (Texture *)g_unk0x0082ca20[g_unk0x0082c698[i]];
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, &dest, pTexture, 3, 0, NULL, NULL,
                     g_unk0x005270e4, 8);
    }
}

// Draws the rows of an in-race menu: every visible row shows its label (and,
// for the value rows, each of its values) with the selected row brighter,
// plus the sprite behind the block.
// match 75%: implementada; misma lógica, MSVC elige &item+0xa como base del
// item (la original usa &item->id) y reordena el prólogo del rect.
// FUNCTION: CMR2 0x00402000
void InRaceMenu_DrawLabelAndValueRows(Menu *pMenu)
{
    MenuItem *pItem;
    int i;
    int j;
    int x;
    int y;
    char *str1;
    char *str2;
    char *sub[3];
    char *text;
    int *pA;
    int *pB;
    char single;
    bool twoLines;
    int *pColour;
    int texture;

    g_unk0x0052ad58[0] = 0;
    g_unk0x0052ad58[1] = 0;
    g_unk0x0052ad58[2] = (short)g_pGraphics->resX;
    y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0;
    g_unk0x0052ad58[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x0052ad58, (BYTE *)&g_unk0x0051608c, 2);
    InRaceMenu_DrawHeaderBar();
    g_unk0x0052ad58[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    g_unk0x0052ad58[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    g_unk0x0052ad58[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    g_unk0x0052ad58[1] = (short)((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                 - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        if (pItem->visible) {
            str1 = CFrontend::GetTextString(pItem->id);
            str2 = CFrontend::GetTextString(pItem->id + 1);
            if (pMenu->cursor == i) {
                pColour = &g_unk0x00516074;
                texture = g_unk0x0052aa60;
            } else {
                pColour = &g_unk0x00516078;
                texture = g_unk0x0052aa68;
            }
            Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)g_unk0x0052ad58,
                         (Texture *)texture, 2, 0, NULL, NULL, (BYTE *)pColour, 8);
            twoLines = true;
            switch (pItem->value) {
            case 1:
                sub[0] = CFrontend::GetTextString(pItem->id);
                sub[1] = CFrontend::GetTextString(pItem->id + 1);
                single = 0;
                break;
            case 2:
                for (j = 0; j < (int)(BYTE)pItem->min; j++)
                    sub[j] = CFrontend::GetTextString(j + 0xa0);
                single = 0;
                break;
            case 4:
                for (j = 0; j < (int)(BYTE)pItem->min; j++)
                    sub[j] = CFrontend::GetTextString(j + 0x9e);
                /* the original falls through into the case below */
            case 0:
                single = 0;
                break;
            default:
                sub[0] = CFrontend::GetTextString(pItem->id);
                twoLines = false;
                break;
            }
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            x = (int)(g_pGraphics->resX * 0x86) / 0x280;
            Font_DrawText(1, str1, x, y, pColour, 0x11);
            if (twoLines)
                Font_DrawText(0, str2, x, (int)(g_pGraphics->resY * 0xf) / 0x1e0 + y, pColour, 0x11);
            pA = (int *)sub;
            pB = (int *)sub + 1;
            for (j = 0; j < (int)(BYTE)pItem->min; j++) {
                text = (char *)*pB;
                if (single == 0)
                    text = (char *)*pA;
                x = (int)(g_pGraphics->resX * 10) / 0x280 + x
                    + Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, text, x, y,
                              pItem->max != j ? &g_unk0x00516078 : &g_unk0x00516074, 0x11);
                strcpy(CFrontend::m_stringDest, text);
                pA++;
                pB--;
            }
            g_unk0x0052ad58[1] = (short)(g_unk0x0052ad58[1] + (int)(g_pGraphics->resY * 0x36) / 0x1e0);
            y = y + (int)(g_pGraphics->resY * 0x36) / 0x1e0;
        }
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x0051607c
int g_unk0x0051607c = 0x80dbaca7;

// Draws the rows of a value menu: each row shows its label and, for the 0..3
// value rows, a slider bar (outline plus filled part) whose length is the
// value; the selected row is drawn brighter.
// match 46%: implementada; misma logica, MSVC ordena de otro modo el prologo,
// el maximo de anchos y las divisiones del slider (los dos rects en registros).
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00403890
void InRaceMenu_DrawSliderRows(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    short bar[4];
    int i;
    int k;
    int x;
    int y;
    int maxWidth;
    int width;
    int *pColour;
    char *text;
    BYTE fade[4];

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    *(DWORD *)fade = g_unk0x0051608c;
    fade[3] = 0x73;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, fade, 2);
    InRaceMenu_DrawHeaderBar();
    maxWidth = 0;
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    rect[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    for (k = 0; k < pMenu->itemCount; k++) {
        if (pMenu->items[k].id == 0x62)
            break;
        text = CFrontend::GetTextString(pMenu->items[k].id);
        width = Font_GetTextWidth(1, (BYTE *)text);
        if (width > maxWidth) {
            text = CFrontend::GetTextString(pMenu->items[k].id);
            maxWidth = Font_GetTextWidth(1, (BYTE *)text);
        }
    }
    i = 0;
    if (pMenu->itemCount > 0) {
        pItem = pMenu->items;
        do {
            pColour = pMenu->cursor == i ? &g_unk0x00516074 : &g_unk0x00516078;
            switch (pItem->value) {
            case 4:
                y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                    + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i;
                Font_DrawText(1, CFrontend::GetTextString(0x62),
                              (int)(g_pGraphics->resX * 0x86) / 0x280, y,
                              pItem->max != 0 ? &g_unk0x0051607c : pColour, 0x11);
                break;
            case 0:
            case 1:
            case 2:
            case 3:
                y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                    + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i;
                Font_DrawText(1, CFrontend::GetTextString(pItem->id),
                              (int)(g_pGraphics->resX * 0x86) / 0x280, y, pColour, 0x11);
                bar[0] = (short)((int)(g_pGraphics->resX * 5) / 0x280 + maxWidth
                                 + (int)(g_pGraphics->resX * 0x86) / 0x280);
                bar[1] = (short)((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                 + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i
                                 - (int)(g_pGraphics->resY * 0xe) / 0x1e0);
                bar[2] = (short)((int)(g_pGraphics->resX * 0xe) / 0x280
                                 + (int)(pItem->min * g_pGraphics->resX * 10) / 0x280);
                bar[3] = (short)((int)(g_pGraphics->resY * 0x10) / 0x1e0);
                DrawRectOutline(bar, (BYTE *)pColour);
                bar[0] = (short)((int)(g_pGraphics->resX * 2) / 0x280
                                 + (int)(g_pGraphics->resX * 5) / 0x280 + maxWidth
                                 + (int)(g_pGraphics->resX * 0x86) / 0x280
                                 + (int)(pItem->max * g_pGraphics->resX * 10) / 0x280);
                bar[1] = (short)((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                                 + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i
                                 - (int)(g_pGraphics->resY * 0xc) / 0x1e0);
                bar[2] = (short)((int)(g_pGraphics->resX * 0x17) / 0x280);
                bar[3] = (short)((int)(g_pGraphics->resY * 0xe) / 0x1e0);
                Sprite_FillRect((int)g_pGraphics + 0x150, bar, (BYTE *)pColour, 2);
                break;
            default:
                y = (int)(g_pGraphics->resY * 0xaa) / 0x1e0
                    + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i;
                Font_DrawText(1, CFrontend::GetTextString(pItem->id),
                              (int)(g_pGraphics->resX * 0x86) / 0x280, y, pColour, 0x11);
                break;
            }
            rect[1] = (short)((int)(g_pGraphics->resY * 0xaa) / 0x1e0
                              + ((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * i
                              - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            if (pMenu->cursor == i)
                Sprite_Queue((SpriteRect *)(g_unk0x0052aa60 + 0x11c), (SpriteRect *)rect,
                             (Texture *)g_unk0x0052aa60, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516074, 8);
            else
                Sprite_Queue((SpriteRect *)(g_unk0x0052aa68 + 0x11c), (SpriteRect *)rect,
                             (Texture *)g_unk0x0052aa68, 2, 0, NULL, NULL, (BYTE *)&g_unk0x00516078, 8);
            i++;
            pItem++;
        } while (i < pMenu->itemCount);
    }
    Font_SetBlendMode(2);
}


// Draws the rows of the car-setup menu (InRaceMenu_BuildCarSetup): each row paints the
// item's own label and then, for every value placed in the item's min/max
// bytes, the matching sub-string; the entry equal to max is highlighted and the
// selected row is drawn brighter with the highlighted row sprite.
// The original recomputes the row's y at every use (MENU_ITEM_Y).
#define MENU_ITEM_Y(i) (((int)(g_pGraphics->resY * 0x2a) / 0x1e0) * (i) + (int)(g_pGraphics->resY * 0xaa) / 0x1e0)

// FUNCTION: CMR2 0x004041e0
void InRaceMenu_DrawCarSetupChoices(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    int i;
    int j;
    int k;
    int x;
    int width;
    int index;
    int *pColour;
    int *pSubColour;
    int texture;

    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)&g_unk0x0051608c, 4);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = *(short *)(g_unk0x0052aa60 + 0x120);
    rect[3] = *(short *)(g_unk0x0052aa60 + 0x122);
    i = 0;
    if (pMenu->itemCount > 0) {
        pItem = pMenu->items;
        do {
            pColour = pMenu->cursor == i ? &g_unk0x00516074 : &g_unk0x00516078;
            switch (pItem->value) {
            case 0:
            case 1:
            case 3:
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
                x = (int)(g_pGraphics->resX * 0x86) / 0x280;
                Font_DrawText(1, CFrontend::m_stringDest, x, MENU_ITEM_Y(i), pColour, 0x11);
                j = 0;
                if (pItem->min != 0) {
                    do {
                        width = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                        pSubColour = pItem->max == j ? &g_unk0x00516074 : &g_unk0x00516078;
                        x = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
                        Font_DrawText(1, CFrontend::GetTextString(j + 0x9c), x, MENU_ITEM_Y(i),
                                      pSubColour, 0x11);
                        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x9c));
                        j++;
                    } while (j < pItem->min);
                }
                break;
            case 2:
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
                x = (int)(g_pGraphics->resX * 0x86) / 0x280;
                Font_DrawText(1, CFrontend::m_stringDest, x, MENU_ITEM_Y(i), pColour, 0x11);
                j = 0;
                if (pItem->min == 2) {
                    do {
                        width = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                        pSubColour = pItem->max == j ? &g_unk0x00516074 : &g_unk0x00516078;
                        x = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
                        Font_DrawText(1, CFrontend::GetTextString(j + 0x9c), x, MENU_ITEM_Y(i),
                                      pSubColour, 0x11);
                        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x9c));
                        j++;
                    } while (j < pItem->min);
                } else if (pItem->min != 0) {
                    do {
                        width = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                        x = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
                        pSubColour = pItem->max == j ? &g_unk0x00516074 : &g_unk0x00516078;
                        index = j == 0 ? 0x9c : j + 0xef;
                        Font_DrawText(1, CFrontend::GetTextString(index), x, MENU_ITEM_Y(i),
                                      pSubColour, 0x11);
                        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(index));
                        j++;
                    } while (j < pItem->min);
                }
                break;
            case 4:
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
                x = (int)(g_pGraphics->resX * 0x86) / 0x280;
                Font_DrawText(1, CFrontend::m_stringDest, x, MENU_ITEM_Y(i), pColour, 0x11);
                j = 0;
                if (pItem->min != 0) {
                    k = 0xa4;
                    do {
                        width = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                        pSubColour = pItem->max == j ? &g_unk0x00516074 : &g_unk0x00516078;
                        x = (int)(g_pGraphics->resX * 10) / 0x280 + x + width;
                        if (RallyData_IsHeadToHeadRaceMode() != 0)
                            strcpy(CFrontend::m_stringDest,
                                   CFrontend::GetTextString(j + 0x9c));
                        else
                            strcpy(CFrontend::m_stringDest,
                                   CFrontend::GetTextString(k));
                        Font_DrawText(1, CFrontend::m_stringDest, x, MENU_ITEM_Y(i), pSubColour, 0x11);
                        j++;
                        k--;
                    } while (j < pItem->min);
                }
                break;
            case 5:
                pSubColour = pItem->max != 0 ? &g_unk0x0051607c : pColour;
                Font_DrawText(1, CFrontend::GetTextString(0x62),
                              (int)(g_pGraphics->resX * 0x86) / 0x280, MENU_ITEM_Y(i), pSubColour, 0x11);
                break;
            default:
                Font_DrawText(1, CFrontend::GetTextString(pItem->id),
                              (int)(g_pGraphics->resX * 0x86) / 0x280, MENU_ITEM_Y(i), pColour, 0x11);
                break;
            }
            rect[1] = (short)(MENU_ITEM_Y(i) - (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            if (pMenu->cursor == i) {
                pColour = &g_unk0x00516074;
                texture = g_unk0x0052aa60;
            } else {
                pColour = &g_unk0x00516078;
                texture = g_unk0x0052aa68;
            }
            Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect,
                         (Texture *)texture, 2, 0, NULL, NULL, (BYTE *)pColour, 8);
            i++;
            pItem++;
        } while (i < pMenu->itemCount);
    }
    Font_SetBlendMode(2);
}


// ---------------------------------------------------------------------------
// Network options and the option screen separators.

void RallyData_LayoutPlayerSplitBars(void);
void Dash_Update(int player);
// Screen separator scratch rect of the option screens (x, y, width, height),
// also used by the controls pages; defined in FrontendMenus.cpp.
extern short g_controlsLine[4];

// White colour the option screen separators and the panel marker sprites use.
// GLOBAL: CMR2 0x00526ffc
BYTE g_unk0x00526ffc[4] = { 0xff, 0xff, 0xff, 0xff };

// Applies the two switches of the network options menu back to the game info,
// rebuilds the split bar and refreshes the dash of the selected player.
// FUNCTION: CMR2 0x00402f20
void InRaceMenu_ApplyNetworkOptions(Menu *pMenu, int param)
{
    int index;

    index = Menu_FindItem(pMenu, 2);
    CGameInfo::SetSplitBarEnabled(pMenu->items[index].max);
    RallyData_LayoutPlayerSplitBars();
    index = Menu_FindItem(pMenu, 4);
    CGameInfo::SetDashOptionEnabled(pMenu->items[index].max);
    Dash_Update(*(BYTE *)&g_unk0x0052af58[1]);
}

// Draws the separator lines of the option screen: one vertical line per slot of
// the layout record param1 (always 1 pixel wide, from the slot's row down to
// the bottom of the option area and alternating between the two sprite layers),
// then the horizontal line that closes the option area and, under its centre,
// the lower separator of the option screen.
// FUNCTION: CMR2 0x00500550
void OptionMenu_DrawSeparators(int param1)
{
    short rect[4];
    int minX;
    int maxX;
    int centre;
    int i;

    g_controlsLine[2] = 1;
    g_unk0x0082ace8.pad[3] = 1;
    if (g_unk0x00831674 != 0) {
        rect[2] = *(short *)(g_unk0x00831674 + 0x120);
        rect[3] = *(short *)(g_unk0x00831674 + 0x122);
    }
    for (i = 0; i < 4; i++) {
        // The layout records hold 16.16 pairs: the integer part is the slot.
        g_controlsLine[0] = (short)(g_unk0x0082ac68[param1 * 4 + i][0] >> 16);
        g_controlsLine[1] = (short)(g_unk0x0082ac68[param1 * 4 + i][1] >> 16);
        g_controlsLine[3] = (short)((int)(g_pGraphics->resY * 0xf5) / 0x1e0) - g_controlsLine[1];
        // Drawn first: the bounds below read the line after Sprite_FillRect.
        if (i % 2 == 0) {
            Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine,
                            (BYTE *)&g_unk0x00526ffc, 4);
        } else {
            Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine,
                            (BYTE *)&g_unk0x00526ffc, 1);
        }
        if (i == 0) {
            minX = g_controlsLine[0];
            maxX = g_controlsLine[0];
        } else if (g_controlsLine[0] < minX) {
            minX = g_controlsLine[0];
        } else if (g_controlsLine[0] > maxX) {
            maxX = g_controlsLine[0];
        }
        if (g_unk0x00831674 != 0) {
            if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::IsTextureWidthSupported(0x400) &&
                CFrontend::IsTextureHeightSupported(0x400)) {
                rect[0] = g_controlsLine[0] - 6;
                rect[1] = g_controlsLine[1] - 6;
            } else {
                rect[0] = g_controlsLine[0] - 3;
                rect[1] = g_controlsLine[1] - 3;
            }
            if (i % 2 == 0) {
                Sprite_Queue((SpriteRect *)(g_unk0x00831674 + 0x11c), (SpriteRect *)rect,
                             (Texture *)g_unk0x00831674, 4, 0, NULL, NULL,
                             (BYTE *)&g_unk0x00526ffc, 8);
            } else {
                Sprite_Queue((SpriteRect *)(g_unk0x00831674 + 0x11c), (SpriteRect *)rect,
                             (Texture *)g_unk0x00831674, 1, 0, NULL, NULL,
                             (BYTE *)&g_unk0x00526ffc, 8);
            }
        }
    }
    if (maxX != minX) {
        g_unk0x0082ace8.pad[0] = (short)minX;
        g_unk0x0082ace8.pad[1] = (short)((int)(g_pGraphics->resY * 0xf5) / 0x1e0);
        g_unk0x0082ace8.pad[2] = (short)(maxX - minX + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x0082ace8.pad, (BYTE *)&g_unk0x00526ffc, 1);
    }
    centre = minX + (maxX - minX) / 2;
    g_controlsLine[0] = (short)centre;
    g_controlsLine[1] = (short)((int)(g_pGraphics->resY * 0xf5) / 0x1e0);
    if (centre <= (int)(g_pGraphics->resX * 0xe4) / 0x280) {
        g_controlsLine[3] = (short)((int)(g_pGraphics->resY * 0xff) / 0x1e0) -
                            (short)((int)(g_pGraphics->resY * 0xf5) / 0x1e0);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, (BYTE *)&g_unk0x00526ffc, 1);
        return;
    }
    g_controlsLine[3] = (short)((int)(g_pGraphics->resY * 0x114) / 0x1e0) -
                        (short)((int)(g_pGraphics->resY * 0xf5) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_controlsLine, (BYTE *)&g_unk0x00526ffc, 1);
    g_unk0x0082ace8.pad[0] = (short)((int)(g_pGraphics->resX * 0xe5) / 0x280);
    g_unk0x0082ace8.pad[1] = (short)((int)(g_pGraphics->resY * 0x114) / 0x1e0);
    g_unk0x0082ace8.pad[2] = (short)(centre - g_unk0x0082ace8.pad[0] + 1);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x0082ace8.pad, (BYTE *)&g_unk0x00526ffc, 1);
}

// 0x50-byte entry of 0x82c6c8 as seen by the option panel animation: the source
// and destination rects, the slide ramp, the two fade phases and their start
// times.
struct Unk0x0082c6c8Anim {
    void *texture;          // 0x0
    short srcX1;            // 0x4
    short srcY1;            // 0x6
    short srcX2;            // 0x8
    short srcY2;            // 0xa
    short dstX1;            // 0xc
    short dstY1;            // 0xe
    short dstX2;            // 0x10
    short dstY2;            // 0x12
    int start;              // 0x14
    int end;                // 0x18
    int current;            // 0x1c
    int distance;           // 0x20 duration of the slide ramp
    int phase1;             // 0x24
    int phase2;             // 0x28
    BYTE field_0x2c[0x10];  // 0x2c
    int startTime;          // 0x3c
    int startTime2;         // 0x40
    int startTime3;         // 0x44
    int field_0x48;         // 0x48
    int active;             // 0x4c
};

// Advances the animation of the option panel selected by 0x82ca1c: eases its
// slide (the ramp is 16.16 and follows the square root of the elapsed
// fraction), copies the source rect while it has not started or the destination
// rect when it is over and interpolates the four edges in between, runs the two
// fade phases of the finished panel and finally rolls the highlight pulse.
// match 61%: implementada; el original resuelve la raiz cuadrada por g_sqrtTable con el
// mismo contador de saltos (bl) que nosotros, pero guarda los 16.16 en EBP-4/EBP-8 y reutiliza
// EAX/EDX en otro orden, asi que las cuatro interpolaciones de borde salen desplazadas.
// FUNCTION: CMR2 0x00505b40
void OptionMenu_AnimateSelectedPanel(void)
{
    Unk0x0082c6c8Anim *pEntry;
    unsigned int now;
    int ratio;
    int index;

    if (g_unk0x0082ca1c == 0xff)
        return;
    index = (signed char)g_unk0x0082ca1c;
    pEntry = (Unk0x0082c6c8Anim *)g_unk0x0082c6c8 + index;
    if (pEntry->active != 0) {
        now = CMain::GetFrameDelta();
        ratio = FixDiv((int)(now - pEntry->startTime) << 16, pEntry->distance);
        ratio = FixSqrt(ratio);
        if (ratio >= 0x10000) {
            pEntry->active = 0;
            pEntry->current = pEntry->end;
            pEntry->startTime2 = CMain::GetFrameDelta();
        } else {
            pEntry->current = FixMul(pEntry->end - pEntry->start, ratio) + pEntry->start;
        }
    }
    if (pEntry->current == 0) {
        *(int *)&pEntry->dstX1 = *(int *)&pEntry->srcX1;
        *(int *)&pEntry->dstX2 = *(int *)&pEntry->srcX2;
        pEntry->phase1 = 0;
        pEntry->phase2 = 0;
    } else if (pEntry->current == 0x10000) {
        pEntry->dstX1 = g_unk0x0082c9ec[0];
        pEntry->dstY1 = g_unk0x0082c9ec[1];
        pEntry->dstX2 = g_unk0x0082c9ec[2];
        pEntry->dstY2 = g_unk0x0082c9ec[3];
        if (pEntry->active == 0) {
            if (pEntry->phase1 != 0x10000) {
                pEntry->phase1 =
                    FixDiv((int)(CMain::GetFrameDelta() - pEntry->startTime2) << 16, 0x320000);
                if (pEntry->phase1 >= 0x10000) {
                    pEntry->phase1 = 0x10000;
                    pEntry->startTime3 = CMain::GetFrameDelta();
                }
            }
            if (pEntry->phase1 == 0x10000 && pEntry->phase2 != 0x10000) {
                pEntry->phase2 =
                    FixDiv((int)(CMain::GetFrameDelta() - pEntry->startTime3) << 16, 0x640000);
                if (pEntry->phase2 >= 0x10000)
                    pEntry->phase2 = 0x10000;
            }
        } else {
            pEntry->phase1 = 0;
            pEntry->phase2 = 0;
        }
    } else {
        pEntry->dstX1 = (short)((FixMul((g_unk0x0082c9ec[0] - pEntry->srcX1) << 16, pEntry->current) >> 16) + pEntry->srcX1);
        pEntry->dstY1 = (short)((FixMul((g_unk0x0082c9ec[1] - pEntry->srcY1) << 16, pEntry->current) >> 16) + pEntry->srcY1);
        pEntry->dstX2 = (short)(FixMulShift32((g_unk0x0082c9ec[2] - pEntry->srcX2) << 16,
                                              pEntry->current) + pEntry->srcX2);
        pEntry->dstY2 = (short)(FixMulShift32((g_unk0x0082c9ec[3] - pEntry->srcY2) << 16,
                                              pEntry->current) + pEntry->srcY2);
        pEntry->phase1 = 0;
        pEntry->phase2 = 0;
    }
    // Sawtooth highlight pulse, restarted when the panel changed (75 frames).
    {
        int t = FixDiv((int)(CMain::GetFrameDelta() - g_unk0x0082c6c0) << 16, 0x4b0000);
        g_unk0x0082cb44 = t - (t & 0xffff0000);
    }
}

// Selects the display mode of a slot: if the entry of the active mode has no
// pending switch, the new slot index (reduced modulo the slot count) is
// activated and the flags of the menu panes are set accordingly; otherwise the
// flags are cleared. Always ends by refreshing the mode animation.
// FUNCTION: CMR2 0x005059d0
void OptionMenu_SelectDisplaySlot(int param_1)
{
    if (g_unk0x0082c6c8[(signed char)g_unk0x0082ca1c].field_0x4c == 0) {
        if ((signed char)g_unk0x0082ca1c != param_1) {
            param_1 = param_1 % g_unk0x0082c694;
            CGameInfo::SwitchOptionPreviewMode(param_1);
        }
        Menu_SetFlags((Menu *)OptionMenu_GetControlSetupMenu(), 1, 1, 1, 1);
        if ((signed char)g_unk0x0082ca1c != param_1) {
            CGameInfo::SelectIdleOptionPreviewEntry((BYTE)(param_1 % g_unk0x0082c694));
            OptionMenu_AnimateSelectedPanel();
            return;
        }
    } else {
        Menu_SetFlags((Menu *)OptionMenu_GetControlSetupMenu(), 0, 0, 0, 0);
    }
    OptionMenu_AnimateSelectedPanel();
}

// ---------------------------------------------------------------------------
// W171 batch: game-info state/menu functions (0x50a880-0x50ee10)
// ---------------------------------------------------------------------------
void RallyData_EnterSelectedOptionPanel(void);
void OptionMenu_DrawAnimatedScreenRows(short param1, short param2, short param3, short param4, short param5, int param6);
void OptionMenu_DrawStageSelectionPanel(int param1);
void OptionMenu_DrawResultsOptions(int param1);
void OptionMenu_DrawGameInfoLayout(int param1, int param2);
void OptionMenu_DrawResultsRallyInfo(int param1);

// Switches the active display slot of the current menu to param_1 and then
// refreshes the slot animation.
// FUNCTION: CMR2 0x0050a880
void OptionMenu_SelectAndAnimateSlot(int param_1, int param_2)
{
    OptionMenu_SelectDisplaySlot(param_1);
    RallyData_EnterSelectedOptionPanel();
}

// Draws the recalled rally entry of the mode panel: option row 7 with the
// "used car" colour 0xf2.
// FUNCTION: CMR2 0x0050a8a0
void OptionMenu_DrawRecalledRallyEntry(int param_1, int param_2)
{
    int value = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14];
    OptionMenu_DrawAnimatedScreenRows(7, value, 2, g_unk0x005293a0, 0xf2, param_2);
}

// Draws the stage list of the mode panel: option row 0xb with colour 0xaa.
// FUNCTION: CMR2 0x0050a8e0
void OptionMenu_DrawModeStageList(int param_1)
{
    int value = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14];
    OptionMenu_DrawAnimatedScreenRows(0xb, value, 1, g_unk0x005293a0, 0xaa, 0);
}

// Draws the game-info title string and the current option of the mode panel.
// FUNCTION: CMR2 0x0050e6a0
void OptionMenu_DrawModeTitle(void)
{
    OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::GetTextString(0x133),
                 (int)g_pGraphics->resX * 0xf0 / 0x280, (int)g_pGraphics->resY * 200 / 0x1e0,
                 g_unk0x00527380, g_unk0x0052738c, 0x11);
    int value = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 5) * 0x14];
    OptionMenu_DrawAnimatedScreenRows(2, value, 3, 0xf0, 0xd7, 0);
}

// Clears the live slots and then re-renders the mode panel with the raised
// entry selected (menu action callback of the rally-info pane).
// FUNCTION: CMR2 0x0050edf0
void OptionMenu_DrawRaisedRallyInfo(unsigned int param_1)
{
    OptionMenu_DrawResultsOptions(0);
    OptionMenu_DrawStageSelectionPanel(param_1);
}

// Renders the whole rally-info screen for param_1: the option panes, the
// recalled entry, the game-info layout and the rally preview text.
// FUNCTION: CMR2 0x0050ee10
void OptionMenu_DrawRallyInfoScreen(unsigned int param_1)
{
    OptionMenu_DrawResultsOptions(1);
    OptionMenu_DrawClampedStageTime();
    OptionMenu_DrawRightColumn();
    OptionMenu_DrawRecalledRallyEntry(param_1, 1);
    OptionMenu_DrawGameInfoLayout(param_1, 1);
    OptionMenu_DrawResultsRallyInfo(0);
    OptionMenu_DrawSeparators(CGameInfo::GetActiveOptionSlot());
}

// The PE entry point of CMR2.exe is the CRT's WinMainCRTStartup (crt0): it
// initialises the heap, the stdio tables, argv/envp and the C initialisers and
// then calls WinMain. It is not game code and our build links the same CRT
// startup from the library, so it is declared as a library symbol instead of
// being decompiled.
// LIBRARY: CMR2 0x00405626
// _WinMainCRTStartup

// Screen-space rectangle (x, y, w, h) the game-info option screens draw into.
// GLOBAL: CMR2 0x00831660
short g_unk0x00831660[4];
// Background colour of the game-info option screens.
// GLOBAL: CMR2 0x005273b8
int g_unk0x005273b8 = -2832441;

// Draws the game-info option screen of the menu's active item: the shared
// background panel, the moving highlight bar of the selected row and, per
// option type, the title and the value strings of the option list; the rows of
// the list screen (case 0) are laid out in pairs around their centred bar.
// FUNCTION: CMR2 0x0050cc10
void OptionMenu_DrawGameInfoLayout(int param_1, int param_2)
{
    int *pColour;
    int index;
    int count;
    int x;
    int y;
    int centre;
    int i;
    int width;

    index = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14];
    pColour = &g_unk0x0052737c;
    g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x1c / 0x280);
    g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0);
    g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x147 / 0x280);
    g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0);
    if (!(char)param_2)
        pColour = &g_unk0x00527380[2];
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x005273b8, 3);
    if ((char)param_2 != '\0' && (CMain::GetFrameDelta() / 0x14 & 1) == 0) {
        g_unk0x00831660[0] -= 3;
        g_unk0x00831660[2] += 6;
        g_unk0x00831660[1] -= 3;
        if (index == 0)
            g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x18e / 0x1e0) -
                                 g_unk0x00831660[1];
        else
            g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x181 / 0x1e0) -
                                 g_unk0x00831660[1];
        OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x0052737c);
    }
    count = OptionMenu_GetRawWorkingValue(CGameInfo::GetActiveOptionSlot(), index);
    switch (index) {
    case 0:
        if (count == 6)
            g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x151 / 0x280);
        else
            g_unk0x00831660[0] = (short)((count * 0x1824 / 0x78 + 0x1c) *
                                         (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        if (OptionMenu_GetValueSliderMenu()[0x1f] == 6)
            g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x151 / 0x280);
        else
            g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x1824 / 0x78 + 0x1c) *
                                         (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        g_unk0x00831660[2] = 1;
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x1a / 0x1e0);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x16f / 0x1e0);
        for (i = 0; i < 7; i++) {
            if (i == 0) {
                x = (int)g_pGraphics->resX * 0x1c / 0x280;
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                        CFrontend::GetTextString(0x41));
                y = (int)g_pGraphics->resY * 0x16c / 0x1e0;
                Font_DrawText(1, CFrontend::m_stringDest, x, y, g_unk0x00527380, 9);
            } else if (i == 6) {
                x = (int)g_pGraphics->resX * 0x163 / 0x280;
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                        CFrontend::GetTextString(0x40));
                y = (int)g_pGraphics->resY * 0x16c / 0x1e0;
                Font_DrawText(1, CFrontend::m_stringDest, x, y, g_unk0x00527380, 0xc);
            } else {
                x = ((i * 0x1824) / 0x78 + 0x25) * (int)g_pGraphics->resX / 0x280;
                sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                        CFrontend::GetTextString(i % 2 + 0x41));
                y = (int)g_pGraphics->resY * 0x16c / 0x1e0;
                Font_DrawText(1, CFrontend::m_stringDest, x, y, g_unk0x00527380, 10);
            }
            if (i % 2 == 1) {
                if (i > 0)
                    centre = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest) / 2 + x;
                x += (int)g_pGraphics->resX * -0x1a / 0x280;
                Font_DrawText(1, CFrontend::GetTextString(i / 2 + 0x3d), x,
                              (int)g_pGraphics->resY * 0x179 / 0x1e0,
                              g_unk0x00527380, 10);
            }
            if (i % 2 == 0 && i > 0) {
                if (i == 6)
                    x -= Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                else
                    x -= Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest) / 2;
                g_unk0x00831660[0] = (short)((centre + x) / 2);
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660,
                                (BYTE *)&g_unk0x00527380[2], 3);
            }
        }
        return;
    case 1:
        if (count == 4)
            g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x151 / 0x280);
        else
            g_unk0x00831660[0] = (short)((count * 0x1e2d / 100 + 0x1c) *
                                         (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        if (OptionMenu_GetValueSliderMenu()[0x1f] == 4)
            g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x151 / 0x280);
        else
            g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x1e2d / 100 + 0x1c) *
                                         (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xc9),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xca),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        return;
    case 2:
        g_unk0x00831660[0] = (short)((count * 0x135 / 100 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x135 / 10 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xcb),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xcc),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        return;
    case 3:
        g_unk0x00831660[0] = (short)((count * 0x135 / 100 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x135 / 10 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xce),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xcd),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        return;
    case 4:
        g_unk0x00831660[0] = (short)((count * 0x135 / 100 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x135 / 10 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xce),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xcd),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        return;
    case 6:
        g_unk0x00831660[0] = (short)((count * 0x135 / 100 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x135 / 10 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xd2),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xd1),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        return;
    case 5:
        g_unk0x00831660[0] = (short)((count * 0x135 / 100 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 != '\0')
            OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        g_unk0x00831660[0] = (short)((OptionMenu_GetValueSliderMenu()[0x1f] * 0x135 / 10 + 0x1c) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((char)param_2 == '\0' || (CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)pColour, 3);
        Font_DrawText(1, CFrontend::GetTextString(0xcf),
                      (int)g_pGraphics->resX * 0x1c / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 9);
        Font_DrawText(1, CFrontend::GetTextString(0xd0),
                      (int)g_pGraphics->resX * 0x163 / 0x280,
                      (int)g_pGraphics->resY * 0x16c / 0x1e0, g_unk0x00527380, 0xc);
        break;
    }
}

// ---------------------------------------------------------------------------
// Boot/state subsystem and option value list (addresses 0x0050e280-0x0050eba0)
// ---------------------------------------------------------------------------

// Keyboard labels of the option value rows ("1".."10","SS").
extern char g_keypad2[];
extern char g_keypad3[];
extern char g_keypad4[];
extern char g_keypad6[];
extern char g_keypad7[];
extern char g_keypad8[];
extern char g_keypad9[];
extern char g_strNum1[4];
extern char g_strNum5[4];
extern char g_strNum10[4];
extern char g_strFlagSS[4];

// Undocumented batch functions of the option/boot subsystem.
void OptionMenu_DrawRecalledRallyEntry(int, int);
void OptionMenu_DrawModeStageList(int);
void OptionMenu_DrawResultsRallyInfo(int);
void OptionMenu_DrawSlotStripPanel(int);
void OptionMenu_DrawResultsOptions(int);
void OptionMenu_DrawGameInfoLayout(int, int);
void OptionMenu_DrawModeTitle(void);
void OptionMenu_DrawAnimatedScreenRows(short, short, short, short, short, int);
void OptionMenu_DrawRecordValues(unsigned int);

// GLOBAL: CMR2 0x00529430
int g_unk0x00529430 = -1;

// Draws the option record values: composes the text of every record row from
// its value, its unit label and its sub-label, highlights the row currently
// being edited and finally lays out the value bar of the selected item.
// FUNCTION: CMR2 0x0050e280
void OptionMenu_DrawRecordValues(unsigned int param_1)
{
    char *pLabels[11] = { g_strNum1,  g_keypad2, g_keypad3, g_keypad4, g_strNum5, g_keypad6,
                          g_keypad7, g_keypad8, g_keypad9, g_strNum10, g_strFlagSS };
    int *pSel;
    int *pUnsel;
    int i;

    if (g_unk0x0082ab44 != 0) {
        pSel = &g_unk0x0052737c;
        pUnsel = &g_unk0x0052738c[1];
    } else {
        pSel = &g_unk0x0052738c[1];
        pUnsel = g_unk0x00527380;
    }
    if (g_unk0x0082ac48 > 0) {
        for (i = 0; i < g_unk0x0082ac48; i++) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x145),
                    SavedGames_GetDifficulty(g_unk0x0082aa3c + i),
                    CFrontend::GetTextString(SavedGames_GetDateLowBits(g_unk0x0082aa3c + i) + 0x93),
                    pLabels[SavedGames_GetDateMiddleBits(g_unk0x0082aa3c + i)]);
            if (g_unk0x0082a924 == g_unk0x0082aa3c + i) {
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, (char *)SavedGames_GetFileName(g_unk0x0082aa3c + i),
                             (int)g_pGraphics->resX * 0x32 / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pSel, g_unk0x0052738c, 0x11);
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, CFrontend::m_stringDest,
                             (int)g_pGraphics->resX * 0x9b / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pSel, g_unk0x0052738c, 0x11);
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, (char *)SavedGames_GetRecordData(g_unk0x0082aa3c + i),
                             (int)g_pGraphics->resX * 0xff / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pSel, g_unk0x0052738c, 0x11);
            } else {
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, (char *)SavedGames_GetFileName(g_unk0x0082aa3c + i),
                             (int)g_pGraphics->resX * 0x32 / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pUnsel, g_unk0x0052738c, 0x11);
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, CFrontend::m_stringDest,
                             (int)g_pGraphics->resX * 0x9b / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pUnsel, g_unk0x0052738c, 0x11);
                OptionMenu_DrawTransitionTextShortCoords(4, 1, 1, (char *)SavedGames_GetRecordData(g_unk0x0082aa3c + i),
                             (int)g_pGraphics->resX * 0xff / 0x280,
                             (int)g_pGraphics->resY * 100 / 0x1e0 +
                                 ((int)g_pGraphics->resY * 0x14) / 0x1e0 * i,
                             pUnsel, g_unk0x0052738c, 0x11);
            }
        }
    } else {
        OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::GetTextString(0x140), (int)g_pGraphics->resX / 2,
                     (int)g_pGraphics->resY / 2, pUnsel, g_unk0x0052738c, 0x12);
    }
    OptionMenu_DrawAnimatedScreenRows(2, *(unsigned char *)(param_1 + 0x1f +
                                       Menu_FindItem((Menu *)param_1, 4) * 0x14),
                 4, 0x32, 0x17c, 1);
    if (CGameInfo::GetActiveOptionSlot() == 1)
        CGameInfo::IsConfiguredMultiplayer();
}

// FUNCTION: CMR2 0x0050e780
void OptionMenu_DrawValueSelectionRows(unsigned int param_1)
{
    short destRect[4];
    int i;
    int yBase;
    int colour;
    int x0;
    int sel;

    x0 = (int)g_pGraphics->resX * 0xf0 / 0x280;
    yBase = (int)g_pGraphics->resY * 0xd7 / 0x1e0;
    sel = (int)*(signed char *)(param_1 + 7);
    colour = g_unk0x0052737c;
    destRect[0] = (short)(x0 * (int)g_pGraphics->resX / 0x280);
    destRect[1] = 0;
    destRect[2] = 0;
    destRect[3] = 0;
    OptionMenu_DrawTransitionTextShortCoords(4, 0, 0, CFrontend::GetTextString(0x13f),
                 (int)g_pGraphics->resX * 0xf0 / 0x280, (int)g_pGraphics->resY * 200 / 0x1e0,
                 g_unk0x00527380, g_unk0x0052738c, 0x11);
    g_unk0x00831660[0] = (short)x0;
    g_unk0x00831660[1] = (short)yBase;
    g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0xa2 / 0x280);
    g_unk0x00831660[3] = 1;
    OptionMenu_EaseRecordBar(0, g_unk0x00831660, 1, 1);
    if (sel == 0)
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&colour, 1);
    else
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527380[2], 1);
    for (i = 0; i < (int)*(signed char *)(param_1 + 6); i++) {
        if (g_unk0x0083166c != 0) {
            if (CGameInfo::GetScreenWidth() < 0x400 || !CFrontend::IsTextureWidthSupported(0x400) ||
                !CFrontend::IsTextureHeightSupported(0x400))
                destRect[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2) +
                              g_unk0x00831660[1] - 6;
            else
                destRect[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2) +
                              g_unk0x00831660[1] - 0xa;
        }
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                CFrontend::GetTextString(i + 0x100));
        if (i == sel) {
            ((void (__stdcall *)(int, int, int, char *, int, short, int *, int *, unsigned int))OptionMenu_DrawTransitionText)(7, 0, 0, CFrontend::m_stringDest,
                         (int)g_pGraphics->resX * 0x14 / 0x280 + x0,
                         (short)((int)g_pGraphics->resY * 0x12 / 0x1e0) + g_unk0x00831660[1],
                         &colour, g_unk0x00527380, 0x11);
            if (g_unk0x0083166c != 0)
                Sprite_Queue((SpriteRect *)(g_unk0x0083166c + 0x11c), (SpriteRect *)destRect,
                             (Texture *)g_unk0x0083166c, 1, 0, 0, 0, (BYTE *)&colour, 8);
        } else {
            OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest,
                         (int)g_pGraphics->resX * 0x14 / 0x280 + x0,
                         (short)((int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1]),
                         g_unk0x00527380, g_unk0x0052738c, 0x11);
            if (g_unk0x00831670 != 0)
                Sprite_Queue((SpriteRect *)(g_unk0x00831670 + 0x11c), (SpriteRect *)destRect,
                             (Texture *)g_unk0x00831670, 1, 0, 0, 0, (BYTE *)g_unk0x00527380, 8);
        }
        g_unk0x00831660[1] =
            (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 * (i + 1) + yBase);
        if (i == sel || i + 1 == sel) {
            if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527378, 1);
            else
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&colour, 1);
        } else {
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527380[2], 1);
        }
    }
}

// Boot/state subsystem entry point: dispatches on the frontend state and
// updates the record tables of the option record being edited.
// FUNCTION: CMR2 0x0050eba0
void OptionMenu_DispatchRecordEditState(unsigned int param_1)
{
    OptionMenu_DrawResultsOptions(0);
    switch (OptionMenu_FindFirstDetailedSlot()) {
    case 1:
        OptionMenu_DrawClampedStageTime();
        OptionMenu_DrawRightColumn();
        OptionMenu_DrawRecalledRallyEntry(param_1, 0);
        OptionMenu_DrawGameInfoLayout((int)OptionMenu_GetValueSliderMenu(), 0);
        OptionMenu_DrawResultsRallyInfo(0);
        OptionMenu_DrawSeparators(CGameInfo::GetActiveOptionSlot());
        if (g_unk0x00529430 !=
            OptionMenu_GetColumnLabelId(OptionMenu_GetControlSetupMenu()[0x1f +
                                        Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14])) {
            OptionPreview_SetTargetWheelAngles(CGameInfo::GetActiveOptionSlot(),
                         (short *)&g_unk0x005273c0[OptionMenu_GetColumnLabelId(
                             OptionMenu_GetControlSetupMenu()[0x1f +
                                            Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14])],
                         0);
            OptionMenu_SetTargetLayoutPairs(CGameInfo::GetActiveOptionSlot(),
                         &g_unk0x008313c8[OptionMenu_GetColumnLabelId(
                             OptionMenu_GetControlSetupMenu()[0x1f +
                                            Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14]) *
                                          8],
                         0);
            g_unk0x00529430 = OptionMenu_GetColumnLabelId(OptionMenu_GetControlSetupMenu()[0x1f +
                                                           Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(),
                                                                         1) *
                                                               0x14]);
            return;
        }
        break;
    case 2:
        OptionMenu_DrawClampedStageTime();
        OptionMenu_DrawModeStageList(param_1);
        OptionMenu_DrawResultsRallyInfo(1);
        OptionMenu_DrawSeparators(CGameInfo::GetActiveOptionSlot());
        if (g_unk0x00529430 !=
            OptionMenu_GetColumnWeight(OptionMenu_GetControlSetupMenu()[0x1f +
                                        Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14])) {
            OptionPreview_SetTargetWheelAngles(CGameInfo::GetActiveOptionSlot(),
                         (short *)&g_unk0x005273c0[OptionMenu_GetColumnWeight(
                             OptionMenu_GetControlSetupMenu()[0x1f +
                                            Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14])],
                         0);
            OptionMenu_SetTargetLayoutPairs(CGameInfo::GetActiveOptionSlot(),
                         &g_unk0x008313c8[OptionMenu_GetColumnWeight(
                             OptionMenu_GetControlSetupMenu()[0x1f +
                                            Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14]) *
                                          8],
                         0);
            g_unk0x00529430 = OptionMenu_GetColumnWeight(OptionMenu_GetControlSetupMenu()[0x1f +
                                                           Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(),
                                                                         2) *
                                                               0x14]);
            return;
        }
        break;
    case 0:
        OptionMenu_DrawSlotStripPanel(param_1);
        return;
    case 3:
        OptionMenu_DrawScreenTitle();
        return;
    case 4:
        OptionMenu_DrawRecordValues(param_1);
        return;
    case 5:
        OptionMenu_DrawModeTitle();
    }
}


// Panel colours used by the rally information screen (amber, blue, green and
// grey, always opaque).
// GLOBAL: CMR2 0x005273a8
BYTE g_colour0x005273a8[8] = { 0xff, 0xff, 0xff, 0xff, 0xfc, 0xf0, 0xcc, 0xff };
// GLOBAL: CMR2 0x005273b0
BYTE g_colour0x005273b0[8] = { 0xe7, 0xeb, 0xc6, 0xff, 0xc9, 0xd6, 0xd8, 0xff };
// Working rectangle shared by the rally information screen (0x831660..0x831667).
// Stage duration in hundredths of a second, indexed by the stage type.
// GLOBAL: CMR2 0x005293c0
int g_unk0x005293c0[7] = { 2, 5, 8, 7, 4, 4, 5 };
// GLOBAL: CMR2 0x0052960c
char g_str0x0052960c[] = "Weather Forecast: %s";
// GLOBAL: CMR2 0x00529624
char g_str0x00529624[] = "Stage";
// GLOBAL: CMR2 0x00524e20
char g_str0x00524e20[] = "100";

extern char g_stageNumberFormat[];
extern char g_str0x00519fb0[];


// stdcall callees that are not implemented yet (ret 8 / ret 0x18).
void OptionMenu_DrawAnimatedScreenRows(short, short, short, short, short, int);
void OptionMenu_SelectAndAnimateSlot(int, int);

// Draws the rally information panel of the results screen: the two shaded
// bars, the stage name, the stage thumbnail and (second mode only) the time
// bar, the target time and the stage row labels.
// match 88%: register allocation / block placement only (676 vs 675 instrs, all relocations and magic-division blocks match); MSVC picks different registers across the menu byte loads and the option-row loop, and if-converts the two ternaries where the original branches.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0050a920
void OptionMenu_DrawResultsRallyInfo(int param_1)
{
    BYTE col[12] = { 0xae, 0xc9, 0x65, 0xff, 0x57, 0x94, 0xc4, 0xff,
                     0xc1, 0x61, 0xa0, 0xff };
    int t;
    int base;
    int w;
    int x;
    int y;
    int i;
    int *pCol;
    Texture *tex;

    g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x1c / 0x280);
    g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0xff / 0x1e0);
    g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0xc9 / 0x280);
    g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x5d / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, g_colour0x005273b0, 3);
    g_unk0x00831660[0] = g_unk0x00831660[0] + g_unk0x00831660[2];
    g_unk0x00831660[3] = g_unk0x00831660[3] + g_unk0x00831660[1];
    g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x12a / 0x1e0);
    g_unk0x00831660[3] = g_unk0x00831660[3] - g_unk0x00831660[1];
    g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x7e / 0x280);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, g_colour0x005273b0 + 4, 3);
    x = g_unk0x00831660[2] / 2 + g_unk0x00831660[0];
    y = (int)g_pGraphics->resY * 0x124 / 0x1e0;
    if (param_1 != 0)
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(0x134));
    else
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(0x135));
    x = x - Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest) / 2;
    Font_DrawText(1, CFrontend::m_stringDest, x, y, g_unk0x00527380, 0x11);
    if (param_1 != 0) {
        tex = (Texture *)g_unk0x0083137c[OptionMenu_GetColumnWeight(
            OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14])];
    } else {
        tex = (Texture *)g_unk0x0083137c[OptionMenu_GetColumnLabelId(
            OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14])];
    }
    if (tex != NULL) {
        g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x2c / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x12d / 0x1e0);
        g_unk0x00831660[2] = *(short *)((char *)tex + 0x120);
        g_unk0x00831660[3] = *(short *)((char *)tex + 0x122);
        g_unk0x00831660[1] = g_unk0x00831660[1] - *(short *)((char *)tex + 0x122) / 2;
        Sprite_Queue((SpriteRect *)((char *)tex + 0x11c), (SpriteRect *)g_unk0x00831660,
                     tex, 1, 0, NULL, NULL, g_colour0x005273a8, 8);
    }
    if (param_1 != 0) {
        g_unk0x00831660[0] = (short)((int)g_pGraphics->resX * 0x1c / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x147 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0);
        OptionMenu_DrawThreeColourGradientStrips(g_unk0x00831660, 1, 0x15, col);
        sprintf(CFrontend::m_stringDest, g_str0x00519fb0);
        Font_DrawText(0, CFrontend::m_stringDest, g_unk0x00831660[0],
                      (int)g_pGraphics->resY * 0x17c / 0x1e0, g_unk0x00527380, 0x11);
        sprintf(CFrontend::m_stringDest, g_str0x00524e20);
        Font_DrawText(0, CFrontend::m_stringDest,
                      *(int *)&g_unk0x00831660[0] + *(int *)&g_unk0x00831660[2],
                      (int)g_pGraphics->resY * 0x17c / 0x1e0, g_unk0x00527380, 0x14);
        Font_DrawText(1, CFrontend::GetTextString(0xf9),
                      g_unk0x00831660[2] / 2 + *(int *)&g_unk0x00831660[0],
                      (int)g_pGraphics->resY * 0x17c / 0x1e0, g_unk0x00527380, 0x12);
        int idx = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2) * 0x14];
        t = FixDiv(OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(idx), 1) << 16,
                   0x640000);
        if (t <= -1)
            t = 0;
        else if (t > 0x10000)
            t = 0x10000;
        g_unk0x00831660[0] = (short)(((FixMul(t, 0x1350000) + 0x1c0000) >> 16) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        OptionMenu_DrawBoxEdges(g_unk0x00831660, (BYTE *)&g_unk0x00527380[2]);
        t = FixDiv(OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(idx), 0) << 16,
                   0x640000);
        if (t < 0)
            t = 0;
        else if (t > 0x10000)
            t = 0x10000;
        g_unk0x00831660[0] = (short)(((FixMul(t, 0x1350000) + 0x1c0000) >> 16) *
                                     (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x160 / 0x1e0 + 1);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0x12 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0xd / 0x1e0 - 2);
        if ((CMain::GetFrameDelta() / 0x14 & 1) == 0)
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660,
                            (BYTE *)&g_unk0x0052737c, 3);
        w = OptionMenu_GetColumnWeight(idx);
        t = OptionMenu_GetRecordGroupWeight(w);
        base = 0xc;
    } else {
        int idx = OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1) * 0x14];
        base = 0xad;
        t = g_unk0x005293c0[idx] * 6000;
        w = idx;
    }
    sprintf(CFrontend::m_stringDest, g_str0x0051a904, (t / 100) / 60, (t / 100) % 60);
    if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
        Font_DrawText(3, CFrontend::m_stringDest, (int)g_pGraphics->resX * 0x124 / 0x280,
                          (int)g_pGraphics->resY * 0x150 / 0x1e0, &g_unk0x00527378, 0x12);
    else
        Font_DrawText(3, CFrontend::m_stringDest, (int)g_pGraphics->resX * 0x124 / 0x280,
                          (int)g_pGraphics->resY * 0x150 / 0x1e0, &g_unk0x0052737c, 0x12);
    x = (int)g_pGraphics->resX * 0x1c / 0x280;
    for (i = 0; i < 4; i++) {
        OptionMenu_DrawTransitionTextShortCoords(5, 1, 1, CFrontend::GetTextString(base + w * 4 + i), x,
                     (int)g_pGraphics->resY * (0x19a + i * 0xe) / 0x1e0,
                     g_unk0x00527380, g_unk0x0052738c, 0x11);
    }
}

// Draws the stage selection panel: the country name, its stage list (the
// current stage highlighted) and the weather forecast, then selects the mode
// of the active slot.
// FUNCTION: CMR2 0x0050c130
void OptionMenu_DrawStageSelectionPanel(int param_1)
{
    int i;
    int *pRec;

    Font_DrawText(0, CFrontend::GetTextString(0x9c),
                  (int)g_pGraphics->resX * 0x25f / 0x280,
                  (int)g_pGraphics->resY * 100 / 0x1e0, &g_unk0x0052737c, 0xc);
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
            CFrontend::GetTextString((RallyDataCountryIndex() & 0xff) + 0x93));
    Font_DrawText(0, CFrontend::m_stringDest,
                  (int)g_pGraphics->resX * 0x25f / 0x280,
                  (int)g_pGraphics->resY * 0x78 / 0x1e0, &g_unk0x0052737c, 0xc);
    sprintf(CFrontend::m_stringDest, g_str0x00529624);
    Font_DrawText(0, CFrontend::m_stringDest,
                  (int)g_pGraphics->resX * 0x25f / 0x280,
                  (int)g_pGraphics->resY * 0xa0 / 0x1e0, &g_unk0x0052737c, 0xc);
    for (i = 0; i < CGameInfo::GetPreviewLayoutMode(); i++) {
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat,
                (RallyDataStageIndex() & 0xff) + 1 + i);
        if (i == OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0) * 0x14]) {
            Font_DrawText(0, CFrontend::m_stringDest,
                          (int)g_pGraphics->resX * 0x25f / 0x280,
                          (int)g_pGraphics->resY * 0xb4 / 0x1e0 +
                              (int)g_pGraphics->resY * 0x14 / 0x1e0 * i,
                          &g_unk0x0052737c, 0xc);
        } else {
            Font_DrawText(0, CFrontend::m_stringDest,
                          (int)g_pGraphics->resX * 0x25f / 0x280,
                          (int)g_pGraphics->resY * 0xb4 / 0x1e0 +
                              (int)g_pGraphics->resY * 0x14 / 0x1e0 * i,
                          g_unk0x00527380, 0xc);
        }
    }
    pRec = RallyData_GetDriverSettingPair(OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0) * 0x14]);
    sprintf(CFrontend::m_stringDest, g_str0x0052960c,
            CFrontend::GetTextString(*pRec + 0x9d));
    Font_DrawText(0, CFrontend::m_stringDest,
                  (int)g_pGraphics->resX * 100 / 0x280,
                  (int)g_pGraphics->resY * 400 / 0x1e0, g_unk0x00527380, 9);
    OptionMenu_SelectAndAnimateSlot(OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0) * 0x14], 1);
}

// Draws the bottom panel of the options screen: the animated strip of all the
// slots and, when the panel texture is loaded, its background sprite with the
// current slot texture; selects the mode of the active slot afterwards.
// FUNCTION: CMR2 0x0050bfd0
void OptionMenu_DrawSlotStripPanel(int param_1)
{
    SpriteRect local;

    OptionMenu_DrawAnimatedScreenRows((short)CGameInfo::GetPreviewLayoutMode(),
                 OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0) * 0x14],
                 0, g_unk0x005293a0, 0x124, 0);
    if (g_unk0x00831668 != 0) {
        g_unk0x00831660[0] = (short)(g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280);
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x6f / 0x1e0);
        g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0xa1 / 0x280);
        g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x37 / 0x1e0);
        local = *(SpriteRect *)g_unk0x00831660;
        local.y = 0;
        local.x = 0;
        OptionMenu_AnimateLayoutRectangleEdge(2, g_unk0x00831660);
        Sprite_Queue(&local, (SpriteRect *)g_unk0x00831660, (Texture *)g_unk0x00831668,
                     1, 0, NULL, NULL, g_colour0x005273a8, 8);
    }
    OptionMenu_SelectAndAnimateSlot(OptionMenu_GetControlSetupMenu()[0x1f + Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0) * 0x14],
                 0);
}


// ===== Agent W171.GiD: results screens (0x50c420, 0x50f650) =====

extern char g_classRowHeaderFormat[];  // 0x00516098 "%s %d" (GameMenus.cpp)
extern char g_keypadFormat[];          // 0x00516940 "%s %s" (Input.cpp)
extern char g_strLabelText[];          // 0x00524d38 "%s: %s" (FrontendScreens.cpp)

BYTE FrontendRecords_GetStageEntryPayload(BYTE **out, int row, int column);
const char *FrontendRecords_GetStageName(int row, int column);
BYTE *FrontendRecords_GetStageEntry(int row, int column);
BYTE RallyData_GetDriverGridRow(BYTE param1, char param2);
int RallyData_GetIndexedStageScoreScale(int row, int column, int variant);

// Space width of the menu font (0x00516870, inside the rally text table below);
// the original has no separate symbol for it, so it is a view into the table.
extern BYTE g_unk0x0051682c[132];

// Run of spaces that separates the two columns of the class table.
// GLOBAL: CMR2 0x005297bc
char g_str0x005297bc[] = "    ";

// Formats of the stage information panel.
// GLOBAL: CMR2 0x00524d0c
char g_str0x00524d0c[] = "%d%% %s";
// GLOBAL: CMR2 0x00524d14
char g_str0x00524d14[] = "%s: ";
// GLOBAL: CMR2 0x00524d1c
char g_str0x00524d1c[] = "km";
// GLOBAL: CMR2 0x00524d20
char g_str0x00524d20[] = "%s: %d.%d";
// GLOBAL: CMR2 0x00524d2c
char g_str0x00524d2c[] = "%s: %d.0";

// Scratch rectangle (x, y, w, h) of the screen rows.

// Draws the results screen: one row per menu option with the option text, its
// value and the separator line between the rows, then the screen title, the
// rally record and finally the stage time. The selected row is drawn with the
// pulsed menu colour, blended against the shared grey.
// match 72%: the instruction sequence is the original's (same calls, same 16.16
// blend through FixMul/FixDiv), but MSVC 6 packs this function's frame
// differently: it gives the `colour` local the dead parameter slot [ebp+8] and
// the fixed-point/FILD temporaries -0x14..-0x4, while the original keeps colour
// at [ebp-0x14] and spills the temporaries/factor to [ebp+8]/[ebp-0xc]/[ebp-8]/
// [ebp-4]. Every mismatch in the two blend blocks is that slot swap; the block
// ordering and the switch lowering are identical.
// FUNCTION: CMR2 0x0050c420
void OptionMenu_DrawResultsOptions(int param_1)
{
    int colour;
    int frames;
    int mode;
    int i;
    int a;
    int b;
    int x;
    int f;
    unsigned int remaining;

    x = (int)g_pGraphics->resX * 0x1c / 0x280;
    colour = *(int *)OptionMenu_GetStateByteAddress();
    if (param_1 != 0)
        colour = g_unk0x0052737c;
    mode = OptionMenu_GetSlotType(OptionMenu_FindFirstDetailedSlot());
    g_unk0x00831660[2] = 1;
    g_unk0x00831660[3] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0);
    g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * 0x1c / 0x1e0);
    frames = OptionMenu_GetScrollDistance();
    i = 0;
    switch (mode) {
    case 0:
        f = 0x10000 - FixDiv(0x10000 - (int)(__int64)((double)(unsigned)frames * CGraphics::m_65536), 0x10000);
        FixMul((int)(__int64)((double)(colour & 0xff) * CGraphics::m_65536), f);
        FixMul((int)(__int64)((double)(g_unk0x00527380[0] & 0xff) * CGraphics::m_65536), 0x10000 - f);
        FixMul((int)(__int64)((double)(colour >> 8 & 0xff) * CGraphics::m_65536), f);
        FixMul((int)(__int64)((double)(g_unk0x00527380[0] >> 8 & 0xff) * CGraphics::m_65536), 0x10000 - f);
        FixMul((int)(__int64)((double)(colour >> 16 & 0xff) * CGraphics::m_65536), f);
        FixMul(0x10000 - f, (int)(__int64)((double)(g_unk0x00527380[0] >> 16 & 0xff) * CGraphics::m_65536));
        break;
    case 1:
        break;
    case 2:
        f = 0x10000 - FixDiv(0x10000 - (int)(__int64)((double)(unsigned)frames * CGraphics::m_65536), 0x10000);
        FixMul(f, (int)(__int64)((double)(g_unk0x00527380[0] & 0xff) * CGraphics::m_65536));
        FixMul((int)(__int64)((double)(colour & 0xff) * CGraphics::m_65536), 0x10000 - f);
        FixMul((int)(__int64)((double)(g_unk0x00527380[0] >> 8 & 0xff) * CGraphics::m_65536), f);
        FixMul((int)(__int64)((double)(colour >> 8 & 0xff) * CGraphics::m_65536), 0x10000 - f);
        FixMul((int)(__int64)((double)(g_unk0x00527380[0] >> 16 & 0xff) * CGraphics::m_65536), f);
        FixMul(0x10000 - f, (int)(__int64)((double)(colour >> 16 & 0xff) * CGraphics::m_65536));
        break;
    }
    while (i < (signed char)OptionMenu_GetControlSetupMenu()[6]) {
        a = OptionMenu_FindFirstDetailedSlot();
        b = OptionMenu_FindFirstDetailedSlot();
        if (CGameInfo::GetConfiguredGameMode() != 0 && i == 4)
            a--;
        if (CGameInfo::GetConfiguredGameMode() != 0 && i == 3)
            b--;
        if (a == i)
            OptionMenu_DrawTransitionText(6, 0, 0,
                         CFrontend::GetTextString(*(short *)(OptionMenu_GetControlSetupMenu() + i * 0x14 + 0x18)),
                         x, (int)g_pGraphics->resY * 0x2c / 0x1e0, &colour, g_unk0x00527380, 0x11);
        else
            Font_DrawText(0,
                          CFrontend::GetTextString(*(short *)(OptionMenu_GetControlSetupMenu() + i * 0x14 + 0x18)),
                          x, (int)g_pGraphics->resY * 0x2c / 0x1e0, g_unk0x00527380, 0x11);
        x += Font_GetTextWidth(0, (BYTE *)CFrontend::GetTextString(
                                      *(short *)(OptionMenu_GetControlSetupMenu() + i * 0x14 + 0x18)));
        x += Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
        g_unk0x00831660[0] = (short)x;
        if (i != (signed char)OptionMenu_GetControlSetupMenu()[6] - 1) {
            if (a == i || b == i + 1)
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&colour, 3);
            else
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660,
                                (BYTE *)(g_unk0x00527380 + 2), 3);
        }
        x += 2 + Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
        i++;
    }
    x = g_unk0x005293a0 * (int)g_pGraphics->resX / 0x280;
    sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat, CFrontend::GetTextString(0x4a),
            CGameInfo::GetActiveOptionSlot() + 1);
    Font_DrawText(0, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0x2c / 0x1e0,
                  g_unk0x00527380, 0x11);
    x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
    x += Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
    g_unk0x00831660[0] = (short)x;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)(g_unk0x00527380 + 2), 3);
    x += 2 + Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
    Font_DrawText(0, (char *)RallyData_GetRecord((BYTE)CGameInfo::GetActiveOptionSlot()), x,
                  (int)g_pGraphics->resY * 0x2c / 0x1e0, g_unk0x00527380, 0x11);
    if (CGameInfo::GetGameModeOptionBit19() == 0)
        return;
    x = (int)g_pGraphics->resX * 0x1e / 0x280 + x +
        Font_GetTextWidth(0, (BYTE *)RallyData_GetRecord((BYTE)CGameInfo::GetActiveOptionSlot()));
    if (CGameInfo::GetConfiguredGameMode() == '\n') {
        unsigned int limit;
        unsigned int delta;

        if (GameInfo_GetSessionField397C() == 0)
            return;
        delta = CMain::GetFrameDelta() - NetPlayers_GetInputFrameDelta();
        limit = (unsigned int)(GameInfo_GetSessionField397C() * 6000);
        if (delta >= limit)
            remaining = 0;
        else
            remaining = limit - delta;
    } else {
        remaining = (unsigned int)CGameInfo::GetOptionMenuRemainingTime();
    }
    sprintf(CFrontend::m_stringDest, g_str0x0051a904, (remaining / 100) / 0x3c,
            (remaining / 100) % 0x3c);
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0x2c / 0x1e0,
                  g_unk0x00527380, 0x11);
}

// Draws the stage summary panel: the event header (special stage or round), the
// stage name, the record time, the surface and severity values, the distance,
// the class rows in two columns and the panel title, plus the animated divider
// bar on the left.
// match 88%: same instructions; the residual is the frame layout. The original
// allocates 0x14 bytes (bar at [esp+8..0xf], v0/v1 at [esp+0x10], table at
// [esp+0x14] plus one unused 4-byte slot at [esp+0xc]); this build allocates
// 0x10 bytes (table at [esp+0x10]), so every frame-relative operand and the
// scheduling of `bar[2] = 1` differ. Source-level reordering (declaration order,
// separate pointer/second-column variables) does not move MSVC 6's slots.
// FUNCTION: CMR2 0x0050f650
void OptionMenu_DrawStageSummaryPanel(int param_1, int param_2)
{
    short bar[4];
    BYTE *table;
    int x;
    int i;
    int count;
    int limit;
    int y;
    int w;
    int w2;
    int column;
    BYTE v0;
    BYTE v1;

    bar[2] = 1;
    x = (int)g_pGraphics->resX * 0x1c2 / 0x280;
    bar[3] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0);
    bar[1] = (short)((int)g_pGraphics->resY * 0xac / 0x1e0);
    OptionMenu_EaseRecordBar(0, bar, 0, 1);
    if (CGameInfo::GetConfiguredGameMode() != 0 && CGameInfo::GetConfiguredGameMode() != 1) {
        if (param_2 == 10)
            sprintf(CFrontend::m_stringDest, g_keypadFormat, CFrontend::GetTextString(0x9b),
                    CFrontend::GetTextString(0x13d));
        else
            sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat, CFrontend::GetTextString(0x9b),
                    param_2 + 1);
        OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xbe / 0x1e0,
                     g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    } else {
        int value;
        int right;

        if (param_2 == 10) {
            switch (CGameInfo::GetConfiguredDifficulty()) {
            case 0:
                value = 2;
                break;
            case 2:
                value = 4;
                break;
            default:
                value = 3;
                break;
            }
        } else {
            value = param_2 / 4 + 1;
        }
        sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat, CFrontend::GetTextString(0xfa),
                value);
        OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xbe / 0x1e0,
                     g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
        right = Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest) + x;
        right += Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
        bar[0] = (short)right;
        Sprite_FillRect((int)g_pGraphics + 0x150, bar, (BYTE *)(g_unk0x005297ac + 3), 3);
        right += 2 + Font_GetTextWidth(0, (BYTE *)((char *)&g_unk0x0051682c[0x44]));
        if (param_2 == 10)
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x13c),
                    RallyData_GetDriverGridRow(RallyDataCountryIndex(), CGameInfo::GetConfiguredDifficulty()) & 0xff,
                    RallyData_GetDriverGridRow(RallyDataCountryIndex(), CGameInfo::GetConfiguredDifficulty()) & 0xff);
        else
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x13c), param_2 + 1,
                    RallyData_GetDriverGridRow(RallyDataCountryIndex(), CGameInfo::GetConfiguredDifficulty()) & 0xff);
        OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest, right, (int)g_pGraphics->resY * 0xbe / 0x1e0,
                     g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    }
    sprintf(CFrontend::m_stringDest, g_strLabelText, CFrontend::GetTextString(0xff),
            FrontendRecords_GetStageName(param_1, param_2));
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xd2 / 0x1e0,
                 g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    sprintf(CFrontend::m_stringDest, g_str0x005297c4, CFrontend::GetTextString(0xfb),
            RallyData_GetIndexedStageScoreScale(param_1, param_2, 0) / 100);
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xe0 / 0x1e0,
                 g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    v0 = FrontendRecords_GetStageEntry(param_1, param_2)[0];
    v1 = FrontendRecords_GetStageEntry(param_1, param_2)[1];
    if (v1 == 0)
        sprintf(CFrontend::m_stringDest, g_str0x00524d2c, CFrontend::GetTextString(0xfc), v0);
    else
        sprintf(CFrontend::m_stringDest, g_str0x00524d20, CFrontend::GetTextString(0xfc), v0, v1);
    strcat(CFrontend::m_stringDest, g_str0x00524d1c);
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xee / 0x1e0,
                 g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    sprintf(CFrontend::m_stringDest, g_str0x00524d14, CFrontend::GetTextString(0xfd));
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0xfc / 0x1e0,
                 g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
    count = FrontendRecords_GetStageEntryPayload(&table, param_1, param_2) & 0xff;
    limit = count;
    if (limit > 1)
        limit = 2;
    w = 0;
    y = 0x10a;
    for (i = 0; i < limit; i++) {
        sprintf(CFrontend::m_stringDest, g_str0x00524d0c, table[i * 8 + 4],
                CFrontend::GetTextString(*(int *)(table + i * 8) + 0xd4));
        OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * y / 0x1e0,
                     g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
        y += 0xe;
        w2 = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
        if (w2 > w)
            w = w2;
    }
    column = Font_GetTextWidth(1, (BYTE *)g_str0x005297bc) + w + x;
    y = 0x10a;
    for (i = limit; i < count; i++) {
        sprintf(CFrontend::m_stringDest, g_str0x00524d0c, table[i * 8 + 4],
                CFrontend::GetTextString(*(int *)(table + i * 8) + 0xd4));
        OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, column, (int)g_pGraphics->resY * y / 0x1e0,
                     g_unk0x005297ac, g_unk0x005297ac + 2, 0x11);
        y += 0xe;
    }
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x13e));
    OptionMenu_DrawTransitionTextShortCoords(3, 1, 1, CFrontend::m_stringDest, x, (int)g_pGraphics->resY * 0x17d / 0x1e0,
                 g_unk0x005297ac, g_unk0x005297ac + 2, 0x21);
}

/* ===== integrated from w171/GiB ===== */
BYTE RallyData_GetDriverGridRow(BYTE param1, char param2);
extern char g_keypadFormat[];
extern char g_strLabelText[];

// Colour of a highlighted row (light grey).
// GLOBAL: CMR2 0x00527398
int g_unk0x00527398 = -2142286001;
// Colour of a highlighted row (transparent white).
// x/y/w/h of the background bar of the row being drawn.
// Name of the current rally/weather of the rally screen rows.
// GLOBAL: CMR2 0x0082aa44
char g_unk0x0082aa44[0x100];
// Row of the highlighted weather entry: "%s: %s_".
// GLOBAL: CMR2 0x005295f8
char g_str0x005295f8[] = "%s: %s_";
// Row of the rally menu: "%s %d (%s)".
// GLOBAL: CMR2 0x00529600
char g_str0x00529600[] = "%s %d (%s)";
// Format drawn by the rows with no text.

/* ===== W194: saved-game image + option menu callbacks (0x4f5190-0x5004xx) ===== */

extern void RallyData_CopyDriverStageTimesToRecords(void);
extern BYTE RallyData_GetDriverOrCategoryFlag(BYTE param1);
extern BYTE *RallyData_GetDriverEntryRecord(int index);
extern int RallyTiming_GetStageTieBreak(int index);
extern BYTE *RallyData_GetDriverCategoryProfile(int index);
extern BYTE *RallyData_GetDriverSkillRecord(int index);
extern int *RallyData_GetDriverSettingPair(int index);
extern int *RallyData_GetDriverPrimaryPairRecord(int index);
extern BYTE *RallyData_GetDriverSelectionFlagRecord(int index);
extern int *RallyData_GetDriverPairRecord(int index);
extern int *RallyData_GetDriverSecondaryPairRecord(int index);
extern unsigned char RallyDataCountryIndex(void);
extern unsigned char RallyDataStageIndex(void);
extern BYTE RallyData_GetSelectionStateByte(void);
extern void OptionMenu_ConfirmOrRevertHighlightedValue(Menu *, MenuItem *);
extern void OptionMenu_RevertRallyRecordGroup(int param_1, int param_2);
extern void OptionMenu_AdvanceSharedModeValue(void);
extern void OptionMenu_Start(char param1);
extern void OptionMenu_ClearRecordTransition(int index);
extern void OptionMenu_ClearTimeout(void);
extern void OptionMenu_UpdateTimeout(void);
extern void OptionPreview_SetWheelNodesVisible(int index, char visible);
extern void OptionMenu_FillValueSlider(void);
extern void OptionMenu_ConfirmHighlightedItem(Menu *pMenu);
extern int OptionMenu_IsSlotEnabled(int mode, int index);
extern int OptionMenu_GetColumnLabelId(int index);
extern int OptionMenu_GetSlotSelector(int index);
extern void OptionMenu_SetSlotSelector(int index, int value);
extern unsigned int OptionMenu_GetRecordPercentage(int index, int type, int dynamic);
extern void OptionPreview_RebuildStageMeshes(int index);
extern int g_unk0x005270b8[11];

// Session image rebuilt before a game is saved. The bytes also form the 0x7f4
// byte record written to <install>\gamesave\*.rcs.
// GLOBAL: CMR2 0x0081a858
BYTE g_unk0x0081a858[0x34];
// GLOBAL: CMR2 0x0081a868
char g_unk0x0081a868[0x20];
// GLOBAL: CMR2 0x0081a888
int g_unk0x0081a888;
// GLOBAL: CMR2 0x0081a88c
int g_unk0x0081a88c[0x20];
// GLOBAL: CMR2 0x0081a90c
int g_unk0x0081a90c[4];
// GLOBAL: CMR2 0x0081a91c
BYTE g_unk0x0081a91c[0x30];
// GLOBAL: CMR2 0x0081a94c
BYTE g_unk0x0081a94c[0x520];
// GLOBAL: CMR2 0x0081ae6c
BYTE g_unk0x0081ae6c[0x1c];
// GLOBAL: CMR2 0x0081ae88
BYTE g_unk0x0081ae88[0xa0];
// GLOBAL: CMR2 0x0081af28
BYTE g_unk0x0081af28[0x50];
// GLOBAL: CMR2 0x0081af78
BYTE g_unk0x0081af78[0x14];
// GLOBAL: CMR2 0x0081af8c
BYTE g_unk0x0081af8c[0x50];
// GLOBAL: CMR2 0x0081afdc
BYTE g_unk0x0081afdc[0x50];
// GLOBAL: CMR2 0x0081b02c
BYTE g_unk0x0081b02c;
// GLOBAL: CMR2 0x0081b030
int g_unk0x0081b030;
// GLOBAL: CMR2 0x0081b034
BYTE g_unk0x0081b034[16];
// GLOBAL: CMR2 0x00525c20
char g_str0x00525c20[] = "%s\\gamesave\\%s";

// Builds the session image that is written as a saved game. When a format
// string is given it is used for the file name; otherwise the name of the
// saved game selected by index is read back from the loaded records.
// FUNCTION: CMR2 0x004f5190
BYTE SavedGames_WriteSessionImage(int index, char *fmt)
{
    int i;
    BYTE *pCar;
    BYTE *p1;
    BYTE *p2;
    BYTE *p3;
    int *pRec;

    RallyData_CopyDriverStageTimesToRecords();

    g_unk0x0081a888 = (g_unk0x0081a888 & 0xffffff80) | (CGameInfo::GetConfiguredGameMode() & 0x7f);
    g_unk0x0081a888 = (g_unk0x0081a888 & 0xfffffc7f) | ((CGameInfo::GetConfiguredDifficulty() & 7) << 7);
    g_unk0x0081a888 = (g_unk0x0081a888 & 0xffffc3ff) | ((CGameInfo::GetConfiguredPlayerCount() & 0xf) << 0xa);
    g_unk0x0081a888 = (g_unk0x0081a888 & 0xffffbfff) | ((CGameInfo::IsConfiguredMultiplayer() & 1) << 0xe);

    for (i = 0; i < (BYTE)CGameInfo::GetConfiguredPlayerCount(); i++)
        g_unk0x0081a88c[i * 2] = (g_unk0x0081a88c[i * 2] & 0xffffffbf) |
                                 ((RallyData_GetDriverOrCategoryFlag((BYTE)i) & 1) << 6);

    for (i = 0; i < 16; i++) {
        pCar = RallyData_GetDriverEntryRecord(i);
        g_unk0x0081a88c[i * 2] = (g_unk0x0081a88c[i * 2] & 0xffffffc0) | (*(int *)pCar & 0x3f);
        pCar = RallyData_GetDriverEntryRecord(i);
        g_unk0x0081a88c[i * 2] = (g_unk0x0081a88c[i * 2] & 0xffffc07f) |
                                 ((*(int *)pCar & 0x1fc0) << 1);
        pCar = RallyData_GetDriverEntryRecord(i);
        g_unk0x0081a88c[i * 2 + 1] = *(int *)(pCar + 4);
    }

    for (i = 0; i < 16; i++)
        g_unk0x0081b034[i] = (BYTE)RallyTiming_GetStageTieBreak(i);

    i = 0;
    if ((BYTE)CGameInfo::GetConfiguredPlayerCount() > 0) {
        p3 = g_unk0x0081ae6c;
        p2 = g_unk0x0081a94c;
        p1 = g_unk0x0081a91c;
        pRec = g_unk0x0081a90c;
        do {
            BYTE *p = RallyData_GetDriverCategoryProfile(i);
            *pRec = (*pRec & 0xfffffcff) | (*(int *)(p + 0x5c) & 0x300);
            p = RallyData_GetDriverCategoryProfile(i);
            *pRec = (*pRec & 0xffffffc7) | (*(int *)(p + 0x5c) & 0x38);
            p = RallyData_GetDriverCategoryProfile(i);
            *pRec = (*pRec & 0xffffe3ff) | (*(int *)(p + 0x5c) & 0x1c00);
            p = RallyData_GetDriverCategoryProfile(i);
            *pRec = (*pRec & 0xffffff3f) | (*(int *)(p + 0x5c) & 0xc0);
            p = RallyData_GetDriverCategoryProfile(i);
            *pRec = (*pRec & 0xfffffff8) | (*(int *)(p + 0x5c) & 7);
            p = RallyData_GetDriverCategoryProfile(i);
            *(FixVector *)p1 = *(FixVector *)(p + 0x10);
            memcpy(p2, RallyData_GetDriverGroupRecord(i), 0x148);
            memcpy(p3, RallyData_GetDriverSkillRecord(i), 7);
            i++;
            pRec++;
            p1 += 0xc;
            p2 += 0x148;
            p3 += 7;
        } while (i < (BYTE)CGameInfo::GetConfiguredPlayerCount());
    }

    memcpy(g_unk0x0081ae88, RallyData_GetDriverSettingPair(0), 0xa0);
    memcpy(g_unk0x0081af28, RallyData_GetDriverPrimaryPairRecord(0), 0x50);
    memcpy(g_unk0x0081af78, RallyData_GetDriverSelectionFlagRecord(0), 0x14);
    memcpy(g_unk0x0081af8c, RallyData_GetDriverPairRecord(0), 0x50);
    memcpy(g_unk0x0081afdc, RallyData_GetDriverSecondaryPairRecord(0), 0x50);

    g_unk0x0081b030 = (g_unk0x0081b030 & 0xffffffe0) | (RallyDataCountryIndex() & 0x1f);
    g_unk0x0081b030 = (g_unk0x0081b030 & 0xfffffc1f) | ((RallyDataStageIndex() & 0x1f) << 5);
    g_unk0x0081b02c = RallyData_GetSelectionStateByte();

    if (fmt != NULL) {
        sprintf(g_unk0x0081a868, fmt);
        SavedGames_MakeUnusedFileName(CFrontend::m_stringDest);
    } else if (index > -1) {
        strcpy(g_unk0x0081a868, (char *)((BYTE *)g_unk0x0081b14c + index * 0x7f4 + 0x10));
        sprintf(CFrontend::m_stringDest, g_str0x00525c20, CInstallInfo::GetGameHDPath(),
                (char *)g_unk0x0081b150[index]);
    } else {
        return 0;
    }
    CInstallInfo::WriteFileToDisk(CFrontend::m_stringDest, 0, g_unk0x0081a858, 0x7f4);
    return 1;
}

// Loads the highlighted saved game into the frontend and moves to the game
// screen; if the slot is empty it only reports the failure.
// FUNCTION: CMR2 0x00500020
void OptionMenu_LoadSelectedSavedGame(unsigned int param1, unsigned int param2)
{
    if (SavedGames_WriteSessionImage(g_unk0x0082a924, NULL)) {
        strcpy(g_unk0x0082a93c, CFrontend::GetTextString(0x143));
        Menu_SetNextAction((int)OptionMenu_GetStatusMenu());
        SavedGames_LoadRecords();
    } else {
        strcpy(g_unk0x0082a93c, CFrontend::GetTextString(0x144));
        Menu_SetNextAction((int)OptionMenu_GetStatusMenu());
    }
}

// "Load game" entry of the option menu: arms the load request and, once the
// records were re-read, starts the frontend on the selected saved game.
// FUNCTION: CMR2 0x004ffed0
void OptionMenu_LoadGameItem(Menu *pMenu, MenuItem *pItem)
{
    if (g_unk0x0082ab44 != 0) {
        Menu_SetNextAction((int)OptionMenu_GetAdvancedOptionsMenu());
        return;
    }
    if (pMenu->items[4].max == 0 && SavedGames_GetCount() > 0) {
        g_unk0x0082ab44 = 1;
        Menu_SetFlags(pMenu, 0, 0, 0, 0);
        return;
    }
    if (pMenu->items[4].max == 1) {
        if (SavedGames_WriteSessionImage(-1, g_unk0x0082aa44) != 0) {
            strcpy(g_unk0x0082aa44, CMain::m_logFileBlankLine);
            strcpy(g_unk0x0082a93c, CFrontend::GetTextString(0x143));
            Menu_SetNextAction((int)OptionMenu_GetStatusMenu());
            SavedGames_LoadRecords();
            g_unk0x0082a924 = SavedGames_GetCount() - 1;
        } else {
            strcpy(g_unk0x0082a93c, CFrontend::GetTextString(0x144));
            Menu_SetNextAction((int)OptionMenu_GetStatusMenu());
        }
    }
}

// Applies the value of the highlighted entry of the option menu when it was
// confirmed: only stores it when it is a valid change for this slot.
// FUNCTION: CMR2 0x004fffe0
void OptionMenu_ApplyHighlightedValue(Menu *pMenu, MenuItem *pItem)
{
    if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(pItem->max)) != 0)
        OptionMenu_ApplySecondModeItem(pMenu, (int)pItem);
    else
        OptionMenu_ConfirmOrRevertHighlightedValue(pMenu, pItem);
}

// Confirms the value of the highlighted entry of the option menu: applies it
// when the slot still had a different one, or reverts the highlighted row.
// FUNCTION: CMR2 0x00500270
void OptionMenu_ConfirmOrRevertHighlightedValue(Menu *pMenu, MenuItem *pItem)
{
    int index;

    index = Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2);
    if (OptionMenu_GetSlotSelector(CGameInfo::GetActiveOptionSlot()) - OptionMenu_GetRecordGroupWeight(OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max)) < 0) {
        if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max)) == 0) {
            if ((int)OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max), 0) > 0) {
                CGameInfo::StartOptionMenuTimeout();
                return;
            }
            return;
        }
    }
    if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max)) == 0)
        OptionMenu_RevertRallyRecordGroup(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(((Menu *)OptionMenu_GetControlSetupMenu())->items[index].max));
    Menu_SetNextAction((int)pMenu->pParent);
}

// Reverts one option group of the current slot: clears the group's "changed"
// flags and subtracts the group's weight from the slot value.
// (Not listed in functions.tsv; it is the counter part of OptionMenu_ApplyRallyRecordGroup.)
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00503010
void OptionMenu_RevertRallyRecordGroup(int param_1, int param_2)
{
    BYTE *pDest = g_unk0x0082c070 + param_1 * 0x148;
    int value;
    int i;

    switch (param_2) {
    case 1:
        if (pDest[0x116] == 0)
            return;
        pDest[0x116] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[0]);
        g_unk0x0082c040[param_1][1] = 1;
        return;
    case 2:
        if (pDest[0x117] == 0)
            return;
        pDest[0x117] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[1]);
        g_unk0x0082c040[param_1][2] = 1;
        return;
    case 3:
        if (pDest[0x10e] == 0 && pDest[0x10f] == 0 && pDest[0x110] == 0 && pDest[0x111] == 0)
            return;
        pDest[0x10e] = 0;
        pDest[0x10f] = 0;
        pDest[0x110] = 0;
        pDest[0x111] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[2]);
        OptionPreview_ClearSkyFadeShape(param_1);
        g_unk0x0082c040[param_1][3] = 1;
        return;
    case 4:
        if (pDest[0x118] == 0)
            return;
        pDest[0x118] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[3]);
        g_unk0x0082c040[param_1][4] = 1;
        return;
    case 5:
        if (pDest[0x112] == 0 && pDest[0x113] == 0 && pDest[0x114] == 0 && pDest[0x115] == 0)
            return;
        pDest[0x112] = 0;
        pDest[0x113] = 0;
        pDest[0x114] = 0;
        pDest[0x115] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[4]);
        g_unk0x0082c040[param_1][5] = 1;
        return;
    case 6:
        {
            int any = 0;
            int changed = 0;
            int has104 = 0;

            if (pDest[0x119] != 0 || pDest[0x11a] != 0 || pDest[0x11b] != 0 ||
                pDest[0x11c] != 0 || pDest[0x11d] != 0 || pDest[0x11e] != 0 ||
                pDest[0x122] != 0 || pDest[0x123] != 0 || pDest[0x124] != 0 ||
                pDest[0x125] != 0 || pDest[0x126] != 0 || pDest[0x127] != 0 ||
                pDest[0x128] != 0 || pDest[0x129] != 0)
                any = 1;
            for (i = 0; i < 3; i++)
                if (*(int *)(pDest + 0x13c + i * 4) != 0)
                    changed = 1;
            for (i = 0; i < 4; i++)
                if (*(int *)(pDest + 0x12c + i * 4) != 0)
                    changed = 1;
            if (pDest[0x104] != 0)
                has104 = 1;
            if (any != 0 || changed != 0 || has104 != 0) {
                pDest[0x119] = 0;
                pDest[0x11a] = 0;
                pDest[0x11b] = 0;
                pDest[0x11c] = 0;
                pDest[0x11d] = 0;
                pDest[0x11e] = 0;
                pDest[0x122] = 0;
                pDest[0x123] = 0;
                pDest[0x124] = 0;
                pDest[0x125] = 0;
                pDest[0x126] = 0;
                pDest[0x127] = 0;
                pDest[0x128] = 0;
                pDest[0x129] = 0;
                *(int *)(pDest + 0x13c) = 0;
                *(int *)(pDest + 0x140) = 0;
                *(int *)(pDest + 0x144) = 0;
                *(int *)(pDest + 0x12c) = 0;
                *(int *)(pDest + 0x130) = 0;
                *(int *)(pDest + 0x134) = 0;
                *(int *)(pDest + 0x138) = 0;
                pDest[0x104] = 0;
                pDest[0x105] = 0;
                for (i = 0; i < 20; i++) {
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[0] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[1] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[2] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[3] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[4] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[5] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[6] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[7] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[8] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[9] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[10] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[11] = 0;
                    ((Unk0x0082c070Item *)pDest)[i].field_0x0[12] = 0xff;
                }
                value = OptionMenu_GetSlotSelector(param_1);
                OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[5]);
                g_unk0x0082c040[param_1][6] = 1;
            }
            OptionPreview_RebuildStageMeshes(param_1);
            return;
        }
    case 7:
        if (pDest[0x10c] == 0 && pDest[0x10d] == 0)
            return;
        pDest[0x10c] = 0;
        pDest[0x10d] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[6]);
        g_unk0x0082c040[param_1][7] = 1;
        return;
    case 8:
        if (pDest[0x11f] == 0)
            return;
        pDest[0x11f] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[7]);
        g_unk0x0082c040[param_1][8] = 1;
        return;
    case 9:
        if (pDest[0x120] == 0)
            return;
        pDest[0x120] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[8]);
        g_unk0x0082c040[param_1][9] = 1;
        return;
    case 10:
        if (pDest[0x121] == 0)
            return;
        pDest[0x121] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[9]);
        g_unk0x0082c040[param_1][10] = 1;
        return;
    case 11:
        if (pDest[0x108] == 0 && pDest[0x109] == 0 && pDest[0x10a] == 0 && pDest[0x10b] == 0)
            return;
        pDest[0x108] = 0;
        pDest[0x109] = 0;
        pDest[0x10a] = 0;
        pDest[0x10b] = 0;
        value = OptionMenu_GetSlotSelector(param_1);
        OptionMenu_SetSlotSelector(param_1, value - g_unk0x005270b8[10]);
        OptionPreview_ClearColourFractions(param_1);
        g_unk0x0082c040[param_1][11] = 1;
        return;
    }
}

extern void OptionMenu_UpdateLoadGameAndNameEntry(Menu *pMenu);
extern void OptionMenu_UpdateMessagesAndRecordStates(unsigned int param1);
extern bool Input_PopQueuedCharacter(int *pOut);
extern void Input_ClearCharacterQueue(void);

// Name of a saved game being edited (0x526fb8 is the accepted character set).
// GLOBAL: CMR2 0x00526fb8
char g_str0x00526fb8[] = "abcdefghijklmnopqrstuvwxyz. ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
// Buffer whose contents the name editor is editing.
// GLOBAL: CMR2 0x0082a934
char *g_unk0x0082a934;
// Set when the name editor was opened this frame.
// GLOBAL: CMR2 0x00526f40
BYTE g_unk0x00526f40 = 1;

// Callback of the load-game screen of the option menu: scrolls the saved-games
// list while it is armed, and feeds the name editor (append/backspace, width and
// character-set checks) while the highlighted row is being edited.
// FUNCTION: CMR2 0x004ff720
void OptionMenu_UpdateLoadGameAndNameEntry(Menu *pMenu)
{
    int value;
    DeviceInfo *pDevice;
    int index;
    int length;

    g_unk0x0082aa40 = SavedGames_GetCount();
    if (g_unk0x0082ab44 != 0) {
        Menu_SetFlags(pMenu, 0, 0, 1, 1);
        pDevice = CInput::GetAvailableDeviceRecord(0);
        if (g_unk0x0082aa40 > 0) {
            if (g_unk0x0082a924 == -1) {
                g_unk0x0082a924 = 0;
            } else if ((pDevice->field_0x8 & 0x20) != 0) {
                g_unk0x0082ab44 = 0;
                g_unk0x0082ac60 = 1;
            } else if ((pDevice->field_0x8 & 4) != 0 && g_unk0x0082a924 > 0) {
                g_unk0x0082a924--;
            } else if ((pDevice->field_0x8 & 8) != 0 && g_unk0x0082a924 < g_unk0x0082aa40 - 1) {
                g_unk0x0082a924++;
            }
        } else {
            g_unk0x0082a924 = -1;
            g_unk0x0082ab44 = 0;
            g_unk0x0082aa3c = 0;
        }
        if (g_unk0x0082a924 < g_unk0x0082aa3c) {
            g_unk0x0082aa3c--;
            if (g_unk0x0082aa3c < 0)
                g_unk0x0082aa3c = 0;
        }
        if (g_unk0x0082a924 >= g_unk0x0082aa3c + 13)
            g_unk0x0082aa3c++;
        if (g_unk0x0082aa40 > 13)
            g_unk0x0082ac48 = 13;
        else
            g_unk0x0082ac48 = g_unk0x0082aa40;
        return;
    }

    if (g_unk0x0082a924 < g_unk0x0082aa3c) {
        g_unk0x0082aa3c--;
        if (g_unk0x0082aa3c < 0)
            g_unk0x0082aa3c = 0;
    }
    if (g_unk0x0082a924 >= g_unk0x0082aa3c + 13)
        g_unk0x0082aa3c++;
    if (g_unk0x0082aa40 > 13)
        g_unk0x0082ac48 = 13;
    else
        g_unk0x0082ac48 = g_unk0x0082aa40;

    index = Menu_FindItem(pMenu, 4);
    if (pMenu->items[index].max == 1) {
        Menu_SetFlags(pMenu, 0, 0, 1, 0);
        pDevice = CInput::GetAvailableDeviceRecord(0);
        if (pDevice->field_0x8 == 0) {
            g_unk0x00526f40 = 1;
        } else if (g_unk0x00526f40 == 0) {
            goto editor;
        }
        if ((pDevice->field_0x8 & 4) != 0) {
            index = Menu_FindItem(pMenu, 4);
            pMenu->items[index].max = 0;
            strcpy(g_unk0x0082aa44, CMain::m_logFileBlankLine);
        } else if ((pDevice->field_0x8 & 1) != 0) {
            pMenu->cursor--;
            strcpy(g_unk0x0082aa44, CMain::m_logFileBlankLine);
        } else if ((pDevice->field_0x8 & 2) != 0) {
            pMenu->cursor++;
            strcpy(g_unk0x0082aa44, CMain::m_logFileBlankLine);
        }
    editor:
        g_unk0x0082a934 = g_unk0x0082aa44;
        strcpy(CFrontend::m_stringDest, g_unk0x0082aa44);
        if (Input_PopQueuedCharacter(&value)) {
            if (value != 8) {
                length = strlen(CFrontend::m_stringDest);
                if (length < 0x1e && strchr(g_str0x00526fb8, (char)value) != NULL &&
                    Font_GetTextWidth(0, (BYTE *)g_unk0x0082a934) <
                        (int)g_pGraphics->resX * 0xdc / 0x280) {
                    CFrontend::m_stringDest[length] = (char)value;
                    CFrontend::m_stringDest[length + 1] = 0;
                    Menu_PlaySoundId(1);
                }
            } else {
                if (CFrontend::m_stringDest[0] != 0) {
                    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 1] = 0;
                    Menu_PlaySoundId(2);
                }
            }
            strcpy(g_unk0x0082a934, CFrontend::m_stringDest);
        }
    } else {
        Input_ClearCharacterQueue();
        strcpy(g_unk0x0082aa44, CMain::m_logFileBlankLine);
        g_unk0x00526f40 = 0;
        Menu_SetFlags(pMenu, 1, 1, 1, 1);
    }
}

// Per-slot number of converted mesh blocks in g_unk0x00831198 (stride 0x2ac).
// This is meshCount at +0x26a in the same records the converter fills.
// Keeping a separate array here makes session cleanup skip the vertex buffers.
#define g_unk0x0082d48a ((BYTE *)g_unk0x0082d220 + 0x26a)
// GLOBAL: CMR2 0x0082d19c
BYTE *g_unk0x0082d19c[16];
// GLOBAL: CMR2 0x008311d8
BYTE *g_unk0x008311d8[16];
// GLOBAL: CMR2 0x00831158
BYTE *g_unk0x00831158[16];

// 0xc-byte per-slot entry of the table at 0x831088.
struct Unk0x00831088 {
    void *pUnk0x0;              // 0x0
    void *pUnk0x4;              // 0x4
    void *pUnk0x8;              // 0x8
};
// GLOBAL: CMR2 0x00831088
Unk0x00831088 g_unk0x00831088[16];
// GLOBAL: CMR2 0x00831318
BYTE g_unk0x00831318;

// Releases every resource of the rally session: the scene nodes and converted
// mesh buffers of every slot, its textures and scene nodes, and clears the
// session flag.
// FUNCTION: CMR2 0x00505f10
BYTE OptionPreview_ReleaseSessionResources(void)
{
    int i;
    int j;

    if (CGameInfo::GetPreviewMode() == 0) {
        for (i = 0; i < 16; i++) {
            for (j = 0; j < 4; j++) {
                if (g_unk0x0082cb78[i].field_0x24[j] != 0)
                    *(int *)((char *)g_unk0x0082cb78[i].pWheels[j] + 0xc) =
                        g_unk0x0082cb78[i].field_0x24[j];
            }
        }
    }
    for (i = 0; i < 16; i++) {
        if (g_unk0x00831198[i] != NULL) {
            for (j = 0; j < g_unk0x0082d48a[i * 0x2ac]; j++) {
                if (g_unk0x00831198[i][j] != NULL) {
                    CFileBuffer::FreeGenericFileBuffer(g_unk0x00831198[i][j]);
                    g_unk0x00831198[i][j] = NULL;
                }
            }
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00831198[i]);
            g_unk0x00831198[i] = NULL;
        }
    }
    for (i = 0; i < 16; i++) {
        if (g_unk0x0082d1dc[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x0082d1dc[i]);
            g_unk0x0082d1dc[i] = NULL;
        }
    }
    for (i = 0; i < 16; i++) {
        if (g_unk0x00831088[i].pUnk0x0 != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00831088[i].pUnk0x0);
            g_unk0x00831088[i].pUnk0x0 = NULL;
        }
        g_unk0x00831088[i].pUnk0x0 = NULL;
        g_unk0x00831088[i].pUnk0x8 = NULL;
        g_unk0x00831088[i].pUnk0x4 = NULL;
    }
    for (i = 0; i < (BYTE)CGameInfo::GetConfiguredPlayerCount(); i++) {
        if (g_unk0x0082cb78[i].pNode != NULL)
            SceneNode_Destroy(g_unk0x0082cb78[i].pNode);
        if (g_unk0x0082cb78[i].field_0x0c != 0)
            SceneNode_Destroy((SceneNode *)g_unk0x0082cb78[i].field_0x0c);
        if (g_unk0x0082cb78[i].field_0x10 != 0)
            SceneNode_Destroy((SceneNode *)g_unk0x0082cb78[i].field_0x10);
        if (g_unk0x0082d19c[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x0082d19c[i]);
            g_unk0x0082d19c[i] = NULL;
        }
        if (g_unk0x008311d8[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x008311d8[i]);
            g_unk0x008311d8[i] = NULL;
        }
        if (g_unk0x00831158[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00831158[i]);
            g_unk0x00831158[i] = NULL;
        }
    }
    g_unk0x00831318 = 0;
    return 1;
}

// Per-frame callback of the option menu: ages the two message timers, resets the
// option rows when the highlighted entry changes, animates the row states and
// normalises the country and stage selections of the session.
// FUNCTION: CMR2 0x004ffab0
void OptionMenu_UpdateMessagesAndRecordStates(Menu *pMenu)
{
    int i;
    int value;
    int old;

    if (g_unk0x0082ac4c > 0) {
        g_unk0x0082ac4c--;
        if (g_unk0x0082ac4c == 0)
            OptionMenu_AdvanceSharedModeValue();
    }
    if (g_unk0x0082ac50 > 0) {
        g_unk0x0082ac50--;
        if (g_unk0x0082ac50 == 0)
            OptionMenu_Start(0);
    }
    g_unk0x0082a928 = CMain::GetFrameDelta();
    if (pMenu->cursor != g_unk0x00526f44) {
        OptionMenu_ClearRecordTransition(0);
        OptionMenu_ClearRecordTransition(1);
        OptionMenu_ClearRecordTransition(3);
        OptionMenu_ClearRecordTransition(2);
        OptionMenu_ClearRecordTransition(5);
        OptionMenu_ClearRecordTransition(6);
        OptionMenu_ClearRecordTransition(7);
        if ((pMenu->cursor != 2 || g_unk0x00526f44 != 1) &&
            (pMenu->cursor != 1 || g_unk0x00526f44 != 2))
            OptionMenu_ClearRecordTransition(4);
        g_unk0x0082a930 = g_unk0x0082a928;
        g_unk0x0082a90c[g_unk0x00526f44] = 2;
        g_unk0x00526f44 = pMenu->cursor;
        OptionMenu_ClearTimeout();
    }
    for (i = 0; i < 6; i++) {
        switch (g_unk0x0082a90c[i]) {
        case 0:
            if (i == pMenu->cursor)
                Menu_SetFlags(pMenu, 0, 0, 0, 0);
            if ((unsigned int)(g_unk0x0082a928 - g_unk0x0082a930) > 1)
                g_unk0x0082a90c[i] = 1;
            break;
        case 1:
            if (i == pMenu->cursor && pMenu->cursor != Menu_FindItem(pMenu, 4))
                Menu_SetFlags(pMenu, 1, 1, 1, 1);
            break;
        case 2:
            if (i == pMenu->cursor)
                Menu_SetFlags(pMenu, 0, 0, 0, 0);
            if ((unsigned int)(g_unk0x0082a928 - g_unk0x0082a930) > 1) {
                g_unk0x0082a930 = g_unk0x0082a928;
                g_unk0x0082a90c[i] = 3;
                g_unk0x0082a90c[pMenu->cursor] = 0;
            }
            break;
        case 3:
            if (i == pMenu->cursor)
                Menu_SetFlags(pMenu, 0, 0, 0, 0);
            break;
        }
    }
    if (pMenu->cursor != Menu_FindItem(pMenu, 2) && pMenu->cursor != Menu_FindItem(pMenu, 1)) {
        for (i = 0; i < (BYTE)CGameInfo::GetConfiguredPlayerCount(); i++)
            OptionPreview_SetWheelNodesVisible(i, 0);
    } else {
        for (i = 0; i < (BYTE)CGameInfo::GetConfiguredPlayerCount(); i++) {
            if (i == CGameInfo::GetActiveOptionSlot())
                OptionPreview_SetWheelNodesVisible(i, 1);
            else
                OptionPreview_SetWheelNodesVisible(i, 0);
        }
    }
    if (pMenu->cursor != Menu_FindItem(pMenu, 5))
        ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 5)].max = 1;
    OptionMenu_ConfirmHighlightedItem(pMenu);
    value = ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1)].max;
    while (OptionMenu_IsSlotEnabled(OptionMenu_GetColumnLabelId(value),
                        (int)CFrontend::GetArchivePrimaryIDEntry(
                            RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot()) & 0xff)) == 0) {
        if (g_unk0x00526f48 < value)
            value++;
        else
            value--;
    }
    ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 1)].max = (BYTE)value;
    if (g_unk0x00526f48 != value) {
        OptionMenu_ClearTimeout();
        OptionMenu_ClearRecordTransition(5);
        OptionMenu_ClearRecordTransition(7);
    }
    g_unk0x00526f48 = value;
    value = ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2)].max;
    while (OptionMenu_IsSlotEnabled(OptionMenu_GetColumnWeight(value),
                        (int)CFrontend::GetArchivePrimaryIDEntry(
                            RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot()) & 0xff)) == 0) {
        if (g_unk0x00526f4c < value)
            value++;
        else
            value--;
    }
    ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 2)].max = (BYTE)value;
    if (g_unk0x00526f4c != value) {
        OptionMenu_ClearTimeout();
        OptionMenu_ClearRecordTransition(5);
        OptionMenu_ClearRecordTransition(7);
    }
    g_unk0x00526f4c = value;
    old = ((Menu *)OptionMenu_GetControlSetupMenu())->items[Menu_FindItem((Menu *)OptionMenu_GetControlSetupMenu(), 0)].max;
    if (g_unk0x00526f50 != old)
        OptionMenu_ClearRecordTransition(7);
    g_unk0x00526f50 = old;
    OptionMenu_UpdateTimeout();
    OptionMenu_FillValueSlider();
    if (pMenu->cursor == Menu_FindItem(pMenu, 4))
        OptionMenu_UpdateLoadGameAndNameEntry(pMenu);
    else
        g_unk0x00526f40 = 0;
}

// The frontend and geometry loader use the same scratch buffer in the original.
#define g_unk0x00663b60 CFrontend::m_stringDest
SceneNode *SceneNode_FindByType(SceneNode *, unsigned int);
// Dependencias de OptionPreview_LoadStageGeometryRecord: las funciones de clase se toman de sus cabeceras.
#include "Game.h"
#include "Graphics.h"
#include "FileBuffer.h"
#include "SceneNode.h"
int  Sector_BuildC3DModelScene(unsigned int, unsigned int, unsigned int);
char *Car_GetTextureSetPath(int, int, int);
int  OptionPreview_UsesWheelVariantSix(int, int);
int  OptionPreview_UsesLowWheelVariant(int, int);
void OptionPreview_BuildDeformGeometry(Unk0x0082d220 *, int);
void OptionPreview_ProjectWheelVertices(int);
int  OptionMenu_GetBackgroundRoot(void);


// Compone el nombre de la geometria del stage (base .c3d con los sufijos A1N/L/S y su .bfl), registra y
// carga los recursos, rellena el registro del stage (matrices de las ruedas, mallas y contadores) y libera
// lo temporal. Devuelve 1 si el stage se cargo.
// FUNCTION: CMR2 0x005062d0
int OptionPreview_LoadStageGeometryRecord(int index)
{
    Unk0x0082cb78 *pEntry;
    int hC3D;
    int hL;
    int hS;
    unsigned int stage;
    unsigned char model;
    unsigned char variant;
    int *pRecord;
    int root;
    int nodeL;
    int nodeS;
    int i;

    stage = OptionMenu_GetBackgroundRoot();
    CGameInfo::GetConfiguredGameMode();
    if (g_unk0x00831318 == 0) {
        CGame::RegisterCallback((void *)OptionPreview_ReleaseSessionResources, 0);
        g_unk0x00831318 = 1;
        for (i = 0; i < 16; i++) {
            g_unk0x00831088[i].pUnk0x0 = NULL;
            g_unk0x00831088[i].pUnk0x8 = NULL;
            g_unk0x00831088[i].pUnk0x4 = NULL;
        }
    }
    model = (BYTE)(int)RallyData_GetDriverRecordSelectionValue(index);
    variant = (BYTE)(int)CFrontend::GetArchivePrimaryIDEntry(model);
    sprintf(g_unk0x00663b60, "%s.c3d", Car_GetTextureSetPath(model, 0, 0));
    if (CGameInfo::GetPreviewMode() == 0) {
        strncpy(g_unk0x00663b60 + strlen(g_unk0x00663b60) - 6, "A1N.c3d", 8);
        hC3D = (int)CFileBuffer::GetGenericFileBuffer(g_unk0x00663b60, 0);
        if (!(OptionPreview_UsesLowWheelVariant(variant, 1) == 0)) {
            strncpy(g_unk0x00663b60 + strlen(g_unk0x00663b60) - 5, "L.c3d", 5);
            hL = (int)CFileBuffer::GetGenericFileBuffer(g_unk0x00663b60, 0);
        } else {
            hL = 0;
        }
        if (OptionPreview_UsesWheelVariantSix(variant, 6) != 0) {
            strncpy(g_unk0x00663b60 + strlen(g_unk0x00663b60) - 5, "S.c3d", 5);
            hS = (int)CFileBuffer::GetGenericFileBuffer(g_unk0x00663b60, 0);
        } else {
            hS = 0;
        }
    } else {
        hC3D = (int)CFileBuffer::GetGenericFileBuffer(g_unk0x00663b60, 0);
        hL = 0;
        hS = 0;
    }
    g_unk0x0082d19c[index] = (BYTE *)hC3D;
    g_unk0x008311d8[index] = (BYTE *)hL;
    g_unk0x00831158[index] = (BYTE *)hS;

    sprintf(g_unk0x00663b60, "%s.bfl", Car_GetTextureSetPath(model, 0, 0));
    if (CGameInfo::GetPreviewMode() == 0)
        strncpy(g_unk0x00663b60 + strlen(g_unk0x00663b60) - 6, "A1.bfl", 6);

    pEntry = &g_unk0x0082cb78[index];
    // the .bfl and the geometry relocations go to the 12-byte loader record of
    // this entry (0x831088, cleared on the first call above), not to pEntry
    pRecord = (int *)&g_unk0x00831088[index];
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)pRecord, g_unk0x00663b60);
    if (hC3D == 0)
        return 0;

    nodeL = 0;
    nodeS = 0;
    root = Sector_BuildC3DModelScene(hC3D, stage, (unsigned int)pRecord);
    if (CGameInfo::GetPreviewMode() == 0) {
        int *p = (int *)&pEntry->field_0x24[0];
        for (i = 1; i <= 4; i++)
            *p++ = *(int *)((int)SceneNode_FindByType((SceneNode *)root, (unsigned int)i) + 0xc);
        if (hL != 0) {
            int *q = (int *)&pEntry->field_0x34[0];
            nodeL = Sector_BuildC3DModelScene(hL, stage, (unsigned int)pRecord);
            for (i = 1; i <= 4; i++)
                *q++ = *(int *)((int)SceneNode_FindByType((SceneNode *)nodeL, (unsigned int)i) + 0xc);
            SceneNode_SetViewMaskTree((SceneNode *)nodeL, 0);
        }
        if (hS != 0) {
            int *q = (int *)&pEntry->field_0x44[0];
            nodeS = Sector_BuildC3DModelScene(hS, stage, (unsigned int)pRecord);
            for (i = 1; i <= 4; i++)
                *q++ = *(int *)((int)SceneNode_FindByType((SceneNode *)nodeS, (unsigned int)i) + 0xc);
            SceneNode_SetViewMaskTree((SceneNode *)nodeS, 0);
        }
    }
    *(int *)((BYTE *)pEntry + 0x14) = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)1);
    *(int *)((BYTE *)pEntry + 0x18) = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)2);
    *(int *)((BYTE *)pEntry + 0x1c) = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)3);
    *(int *)((BYTE *)pEntry + 0x20) = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)4);
    *(BYTE *)pEntry = variant;
    *(int *)((BYTE *)pEntry + 0x8) = root;
    *(int *)((BYTE *)pEntry + 0x4) = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)5);
    *(int *)((BYTE *)pEntry + 0xc) = nodeL;
    *(int *)((BYTE *)pEntry + 0x10) = nodeS;
    OptionPreview_RebuildStageMeshRecord(index);
    OptionPreview_BuildDeformGeometry(&g_unk0x0082d220[index], index);
    {
        int h = (int)SceneNode_FindByType((SceneNode *)root, (unsigned int)0xe);
        if (h != 0) {
            int tex = *(int *)((char *)CGraphics::m_pTextureManager + *(int *)(*(int *)(*(int *)(h + 0xc) + 0x24) + 4) * 4 + 900);
            CGraphics::RemapTextureAlpha((Texture *)tex, 0xbf, 0, 0x40, 0, 0x80, 0, index);
            CGraphics::RemapTextureAlpha((Texture *)tex, 0xe0, 0, 0xe0, 0, 0xe0, 0, index);
        }
    }
    g_unk0x0082d118 = 0;
    OptionPreview_ApplyStageDeformations(index);
    g_unk0x0082d15c[index] = -1;
    OptionPreview_ProjectWheelVertices(variant);
    return 1;
}

// ---------------------------------------------------------------------------
// Identifier tables of the profile screens (defined in FrontendMenus.cpp).
// ---------------------------------------------------------------------------
extern BYTE g_unk0x00825398[0x4c];
extern BYTE g_unk0x008253e4[0x98];
extern BYTE g_unk0x0082547c[0x898];
extern BYTE g_unk0x00825d14[0x258];
extern BYTE g_unk0x00825f6c[0x1cc];

BYTE *RallyData_GetAvailableCategorySaveRecord(int index);
void FrontendProfile_ScrambleIdentifier(unsigned int param_1, unsigned int *pNumber, char *pByte, char *pOut);

// Rewrites every best-time record of the selected rally as a scrambled
// identifier string (see FrontendProfile_ScrambleIdentifier/FrontendText_FormatCodeGroups) into the five tables the
// profile screens display.
// FUNCTION: CMR2 0x004f8b30
void FrontendRecords_BuildScrambledBestTimeTables(void)
{
    int iVar2;
    int iVar3;
    int iVar4;
    unsigned int uVar5;
    unsigned int uVar8;
    unsigned int *puVar10;
    BYTE *puVar9;
    unsigned int *puVar7;
    union CodeTag {
        unsigned word;
        struct {
            unsigned char kind : 3;
            unsigned char driver : 5;
        } bits;
    } local_4;
    // The packed word is carried between all five passes, including its XOR.
    // Only the rally view is assigned through fields; the other layouts keep
    // their original masks so MSVC6 preserves the original store grouping.
    union CodeWord {
        unsigned word;
        struct {
            unsigned minutes : 6;
            unsigned country : 3;
            unsigned car : 6;
            unsigned automatic : 1;
            unsigned seconds : 6;
            unsigned hundredths : 7;
            unsigned difficulty : 2;
            unsigned rest : 1;
        } rally;
    } local_8;
    int local_c;
    BYTE *local_10;
    int local_14;
    BYTE *local_18;
    int local_1c;

    local_8.word = 0;
    local_4.word = 0;
    local_10 = 0;
    local_14 = 0x150;
    do {
        local_c = ((int)local_10 % 2) + 10;
        local_1c = local_14;
        for (uVar8 = 0; (int)uVar8 < local_c; uVar8++) {
            {
                puVar7 = (unsigned int *)(RallyData_GetAvailableCategorySaveRecord(0) + 4 + local_1c);
                puVar10 = (unsigned int *)(RallyData_GetAvailableCategorySaveRecord(0) + local_1c);
                if (((*puVar10 & 0x80) == 0) || (*puVar7 / 6000 >= 16)) {
                    g_unk0x0082547c[((int)local_10 * 11 + uVar8) * 0x19] = 0;
                } else {
                    local_8.word = (local_8.word & 0xfffff00f) |
                              (((uVar8 & 0xf) << 4 | (unsigned int)local_10 & 0xf) << 4);
                    local_8.word = (local_8.word & 0xfffc0fff) | (*puVar10 & 0x3f) << 0xc;
                    local_8.word = (local_8.word & 0x7fffffff) | (~*puVar10 & 0xffffffc0) << 0x19;
                    local_8.word = local_8.word ^ ((*puVar7 / 6000 ^ local_8.word) & 0xf);
                    local_8.word = (local_8.word & 0xff03ffff) | ((*puVar7 / 100) % 0x3c & 0x3f) << 0x12;
                    local_8.word = (local_8.word & 0x80ffffff) | (*puVar7 % 100 & 0x7f) << 0x18;
                    local_4.bits.kind = 0;
                    local_4.bits.driver = *puVar10 >> 8;
                    puVar7 = &local_8.word;
                    iVar2 = 4;
                    do {
                        *(BYTE *)puVar7 ^= (BYTE)(local_4.bits.driver << 1);
                        puVar7 = (unsigned int *)((int)puVar7 + 1);
                        iVar2--;
                    } while (iVar2 != 0);
                    FrontendProfile_ScrambleIdentifier(0, &local_8.word, (char *)&local_4,
                                 (char *)&g_unk0x0082547c[((int)local_10 * 11 + uVar8) * 0x19]);
                    FrontendText_FormatCodeGroups((char *)&g_unk0x0082547c[((int)local_10 * 11 + uVar8) * 0x19]);
                }
                local_1c += 8;
            }
        }
        local_14 += 0x60;
        local_10 = (BYTE *)((int)local_10 + 1);
    } while (local_14 < 0x450);

    local_10 = g_unk0x00825d14;
    local_1c = 0;
    local_14 = 0;
    do {
        uVar5 = 0;
        local_18 = local_10;
        local_c = local_14;
        do {
            iVar3 = (int)(RallyData_GetAvailableCategorySaveRecord(0) + 0x34 + local_c);
            iVar4 = (int)(RallyData_GetAvailableCategorySaveRecord(0) + 0x30 + local_c);
            puVar7 = (unsigned int *)iVar4;
            if (((*(BYTE *)iVar4 & 0x80) == 0) || (*(unsigned int *)(iVar3 + 4) / 6000 >= 64)) {
                *local_18 = 0;
            } else {
                local_8.rally.country = uVar5;
                local_8.rally.car = *puVar7 & 0x3f;
                local_8.rally.automatic = (~*puVar7 & 0x40) >> 6;
                local_8.rally.minutes = *(unsigned int *)(iVar3 + 4) / 6000;
                local_8.rally.seconds = (*(unsigned int *)(iVar3 + 4) / 100) % 60;
                local_8.rally.hundredths = *(unsigned int *)(iVar3 + 4) % 100;
                local_8.rally.difficulty = local_1c;

                puVar7 = &local_8.word;
                local_4.bits.kind = 1;
                local_4.bits.driver = *(unsigned *)iVar4 >> 8;
                iVar3 = 4;
                do {
                    *(BYTE *)puVar7 ^= (BYTE)(local_4.bits.driver << 1);
                    puVar7 = (unsigned int *)((int)puVar7 + 1);
                    iVar3--;
                } while (iVar3 != 0);
                FrontendProfile_ScrambleIdentifier(0, &local_8.word, (char *)&local_4, (char *)local_18);
                FrontendText_FormatCodeGroups((char *)local_18);
            }
            uVar5++;
            local_c += 0x24;
            local_18 += 0x4b;
        } while ((int)uVar5 < 8);
        local_1c++;
        local_14 += 0xc;
        local_10 += 0x19;
    } while (local_10 < g_unk0x00825d14 + 0x4b);

    uVar5 = 0;
    local_c = 0;
    puVar9 = g_unk0x00825398;
    do {
        puVar7 = (unsigned int *)(RallyData_GetAvailableCategorySaveRecord(0) + 4 + local_c);
        puVar10 = (unsigned int *)(RallyData_GetAvailableCategorySaveRecord(0) + local_c);
        if ((*(BYTE *)puVar10 & 0x80) != 0) {
            local_8.word = (local_8.word & 0xfff83fff) | (*puVar7 & 0xf) << 0xe;
            local_8.word = (local_8.word & 0xffe7c07f) |
                      ((uVar5 & 3) << 0x12 | *puVar7 & 0x1fc0) << 1;
            local_8.word = local_8.word ^ ((*puVar10 ^ local_8.word) & 0x3f);
            local_8.word = local_8.word ^ ((~*puVar10 ^ local_8.word) & 0x40);
            local_4.bits.kind = 2;
            local_4.bits.driver = *puVar10 >> 8;

            puVar10 = &local_8.word;
            iVar2 = 4;
            do {
                *(BYTE *)puVar10 ^= (BYTE)(local_4.bits.driver << 1);
                puVar10 = (unsigned int *)((int)puVar10 + 1);
                iVar2--;
            } while (iVar2 != 0);
            FrontendProfile_ScrambleIdentifier(0, &local_8.word, (char *)&local_4, (char *)puVar9);
            FrontendText_FormatCodeGroups((char *)puVar9);
        } else {
            *puVar9 = 0;
        }
        puVar9 += 0x19;
        uVar5++;
        local_c += 0x10;
    } while (puVar9 < g_unk0x00825398 + 0x4b);

    uVar5 = 0;
    local_14 = 0x450;
    local_18 = g_unk0x008253e4;
    do {
        uVar8 = 0;
        local_10 = (BYTE *)local_14;
        local_c = (int)local_18;
        do {
            // The original keeps puVar7 from the championship pass here and
            // discards the first provider result. Preserve both calls.
            RallyData_GetAvailableCategorySaveRecord(0);
            iVar2 = (int)RallyData_GetAvailableCategorySaveRecord(0);
            puVar10 = (unsigned int *)(iVar2 + (int)local_10);
            if ((*puVar10 & 0x80) != 0) {
                local_8.word = (local_8.word & 0xffff8fff) | (*puVar7 & 7) << 0xc;
                local_8.word = (local_8.word & 0xfffe707f) |
                          ((uVar5 & 3) << 0xe | *puVar7 & 0x7c0) << 1;
                local_8.word = local_8.word ^ ((*puVar10 ^ local_8.word) & 0x3f);
                local_8.word = (local_8.word & 0xfff9ffbf) | (~*puVar10 & 0x40) | (uVar8 & 3) << 0x11;
                iVar2 = 4;
                local_4.bits.kind = 3;
                local_4.bits.driver = *puVar10 >> 8;

                puVar10 = &local_8.word;
                do {
                    *(BYTE *)puVar10 ^= (BYTE)(local_4.bits.driver << 1);
                    puVar10 = (unsigned int *)((int)puVar10 + 1);
                    iVar2--;
                } while (iVar2 != 0);
                FrontendProfile_ScrambleIdentifier(0, &local_8.word, (char *)&local_4, (char *)local_c);
                FrontendText_FormatCodeGroups((char *)local_c);
            } else {
                *(BYTE *)local_c = 0;
            }
            uVar8++;
            local_10 = (BYTE *)((int)local_10 + 0x24);
            local_c += 0x4b;
        } while ((int)uVar8 < 2);
        local_14 += 0xc;
        uVar5++;
        local_18 += 0x19;
    } while (local_14 < 0x474);

    uVar5 = 0;
    local_10 = g_unk0x00825f6c;
    do {
        iVar2 = ((int)uVar5 / 3) * 3 + (int)uVar5 % 3;
        puVar7 = (unsigned int *)(RallyData_GetAvailableCategorySaveRecord(0) + 0x4c0 + iVar2 * 8);
        iVar3 = (int)(RallyData_GetAvailableCategorySaveRecord(0) + 0x4bc + iVar2 * 8);
        uVar8 = *(unsigned int *)iVar3;
        puVar10 = (unsigned int *)iVar3;
        if (((uVar8 & 0x80) == 0) || (*puVar7 / 6000 >= 64)) {
            *local_10 = 0;
        } else {
            local_8.word = local_8.word ^ ((uVar8 ^ local_8.word) & 0x3f);
            local_8.word = (local_8.word & 0xfffffc3f) | (~*puVar10 & 0x40) | (uVar5 & 7) << 7;
            local_8.word = (local_8.word & 0xe07fffff) | (*puVar7 / 6000 & 0x3f) << 0x17;
            local_8.word = (local_8.word & 0xffff03ff) | ((*puVar7 / 100) % 0x3c & 0x3f) << 10;
            local_8.word = (local_8.word & 0xff80ffff) | (*puVar7 % 100 & 0x7f) << 0x10;
            iVar2 = 4;

            puVar7 = &local_8.word;
            local_4.bits.kind = 4;
            local_4.bits.driver = *puVar10 >> 8;
            do {
                *(BYTE *)puVar7 ^= (BYTE)(local_4.bits.driver << 1);
                puVar7 = (unsigned int *)((int)puVar7 + 1);
                iVar2--;
            } while (iVar2 != 0);
            FrontendProfile_ScrambleIdentifier(0, &local_8.word, (char *)&local_4, (char *)local_10);
            FrontendText_FormatCodeGroups((char *)local_10);
        }
        local_10 += 0x19;
        uVar5++;
    } while (local_10 < g_unk0x00825f6c + 0xc8);
}
