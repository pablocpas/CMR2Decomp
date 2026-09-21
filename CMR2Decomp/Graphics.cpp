#include "Graphics.h"
#include "Frontend.h"
#include "FileBuffer.h"
#include "GameInfo.h"
#include "RegKey.h"
#include "main.h"
#include "Sound.h"
#include "Game.h"
#include <basetsd.h>
#include <cstring>
#include <stdlib.h>
#include <windef.h>
#include <wingdi.h>
#include <winnt.h>
#include <winuser.h>

// GLOBAL: CMR2 0x00660830
Graphics g_graphics;

// GLOBAL: CMR2 0x00520b74
Graphics *g_pGraphics = &g_graphics;
D3DTextureManager *CGraphics::m_pTextureManager;

char CGraphics::m_strSettingConfigurationToDefault[36] = "Setting configuration to defaults";

BOOL CGraphics::m_unk0x00520b7c = TRUE;
unsigned int CGraphics::m_unk0x0065fa2c;
unsigned int CGraphics::m_textureCount;
unsigned int CGraphics::m_lockedTextureCount;
LockedTexture CGraphics::m_lockedTextures[7];
Unk0x0065aee8 CGraphics::m_unk0x0065aee8[64];
unsigned int CGraphics::m_unk0x00520b2c = 0xff;
unsigned int CGraphics::m_unk0x00520b30 = 0xff;
int CGraphics::m_unk0x0065fa44;
int CGraphics::m_unk0x0065fa48;
float CGraphics::m_oneOver128 = 1.0f / 128.0f;
float CGraphics::m_unk0x00520b34 = 1.0f;
float CGraphics::m_unk0x00520b38 = 1.0f;
TGAImageInfo CGraphics::m_tgaImageInfo;
int CGraphics::m_unk0x00816a80;
int CGraphics::m_unk0x00816a84;
IDirectDrawSurface7 *CGraphics::m_mipMapSurfaces[2];
DWORD CGraphics::m_cubeMapSize = 64;
int CGraphics::m_unk0x00520b14 = 1;
char CGraphics::m_strSuffixBU[4] = "BU";
char CGraphics::m_strSuffixRU[4] = "RU";
char CGraphics::m_strSuffixBR[4] = "BR";
char CGraphics::m_strSuffixBODF[8] = "BODF";
char CGraphics::m_strSuffixDIGIT[8] = "DIGIT";
char CGraphics::m_strSuffixREVCT[8] = "REVCT";
DWORD CGraphics::m_clearColour;
int CGraphics::m_cullMode;
int CGraphics::m_zEnable;
int CGraphics::m_textureAddressClamp;
int CGraphics::m_zWriteEnable;
int CGraphics::m_texCoordIndex[8];
Unk0x006e0bb0 CGraphics::m_d3dDeviceDesc7;
double CGraphics::m_oneOver65536 = 1.0 / 65536.0;
double CGraphics::m_65536 = 65536.0;
float CGraphics::m_projectionScale = 0.5f;
float CGraphics::m_nearPlane = 1.0f;
float CGraphics::m_farPlane = 250.0f;
float CGraphics::m_fovX = 2.337f;
float CGraphics::m_fovY = 4.197f;
int CGraphics::m_farPlaneFixed = 65536000;
float CGraphics::m_projection11;
float CGraphics::m_projection22;
float CGraphics::m_projection33;
float CGraphics::m_projection43;
char CGraphics::m_strSetDesktopTo16Bit[48] = "Set windows desktop to 16 bit for this 3D card";
int CGraphics::m_unk0x0072d56c;
BOOL CGraphics::m_unk0x00660bfc;
TextureFormat CGraphics::m_texFormat16;
TextureFormat CGraphics::m_texFormat16Alpha;
TextureFormat CGraphics::m_texFormatDXT1_16;
TextureFormat CGraphics::m_texFormatDXT5_16;
TextureFormat CGraphics::m_texFormatBump16;
TextureFormat CGraphics::m_texFormat24;
TextureFormat CGraphics::m_texFormat32;
TextureFormat CGraphics::m_texFormatDXT1_32;
TextureFormat CGraphics::m_texFormatDXT5_32;
TextureFormat CGraphics::m_texFormatBump32;
BOOL CGraphics::m_hasTexFormat16;
BOOL CGraphics::m_hasTexFormatDXT1_16;
BOOL CGraphics::m_hasTexFormat16Alpha;
BOOL CGraphics::m_hasTexFormatDXT5_16;
BOOL CGraphics::m_hasTexFormat24;
BOOL CGraphics::m_hasTexFormatDXT1_32;
BOOL CGraphics::m_hasTexFormat32;
BOOL CGraphics::m_hasTexFormatDXT5_32;
BOOL CGraphics::m_hasTexFormatBump16;
BOOL CGraphics::m_hasTexFormatBump32;
unsigned int CGraphics::m_unk0x0065fa28;
int CGraphics::m_unk0x006dd890;
int CGraphics::m_unk0x00663b1c;
Unk0x0065ff90 CGraphics::m_unk0x0065ff90[10];
BOOL CGraphics::m_unk0x0081709c;
DDDeviceEnumBuffer CGraphics::m_unk0x0065fd08;
DDDeviceEnumBuffer CGraphics::m_displayDevicePool;
int CGraphics::m_lifetimeDisplayDeviceCount = 0;
int CGraphics::m_totalPixelsForScreen = 0;
Unk0x00660040 CGraphics::m_unk0x00660040[10];
int CGraphics::m_unk0x00663b18 = 0;
int CGraphics::m_unk0x00663b20 = 0;
int CGraphics::m_unk0x00663b24 = 0;
int CGraphics::m_selectedDisplayDeviceIx = 0;
DWORD CGraphics::m_displayCount = 0;
Entry CGraphics::m_unk0x006634d8[10];
DisplayMode CGraphics::m_displays[10];
int CGraphics::m_releaseSurfaceCallbackID;
char CGraphics::m_direct3DHAL[13] = "Direct3D HAL";
char CGraphics::m_direct3DTLHAL[18] = "Direct3D T&L HAL";

// FUNCTION: CMR2 0x00405830
bool CGraphics::InitializeDirectX(void) {
    LPDIRECTDRAW lpDD;
    LPDIRECTDRAW7 lpDD7;
    DDDEVICEIDENTIFIER2 lpDDIdenitifer;

    DirectDrawCreateEx(NULL, (LPVOID*)&lpDD, IID_IDirectDraw7, 0);
    lpDD->QueryInterface(IID_IDirectDraw7, (LPVOID*)&lpDD7);
    lpDD7->GetDeviceIdentifier(&lpDDIdenitifer, 0);

    if (strcmp(lpDDIdenitifer.szDescription, CGameInfo::m_gameInfo.graphicsCardName) == 0) {
        if (g_pGraphics->pDD7 != NULL) {
            if (g_pGraphics->pDD7->Release() == 0)
                g_pGraphics->pDD7 = NULL;
        }

        if (g_pGraphics->pDD != NULL) {
            if (g_pGraphics->pDD->Release() == 0)
                g_pGraphics->pDD = NULL;
        }

        return false;
    }

    wsprintfA(CGameInfo::m_gameInfo.graphicsCardName, CRegKey::m_regKeyPathFormatValue, lpDDIdenitifer.szDescription);

    if (g_pGraphics->pDD7 != NULL) {
        if (g_pGraphics->pDD7->Release() == 0)
            g_pGraphics->pDD7 = NULL;
    }

    if (g_pGraphics->pDD != NULL) {
        if (g_pGraphics->pDD->Release() == 0)
            g_pGraphics->pDD = NULL;
    }

    MessageBoxA(CMain::m_hWndList[CMain::m_hWndIx], m_strSettingConfigurationToDefault, CMain::m_logFileBlankLine, MB_TOPMOST | MB_TASKMODAL);

    return true;
}

// FUNCTION: CMR2 0x00405990
void CGraphics::SetDefaults(void) {
    CGameInfo::m_gameInfo.unknownGraphicsOptions |= 0x40000000;
    CGameInfo::m_gameInfo.screenWidth = 0x280;
    g_pGraphics->resX = 0x280;
    CGameInfo::m_gameInfo.screenHeight = 0x1e0;
    g_pGraphics->resY = 0x1e0;
    CGameInfo::m_gameInfo.screenColourDepth = 0x10;
    g_pGraphics->depth = 0x10;
    CGameInfo::m_gameInfo.unknownGraphicsOptions |= 1;
    g_pGraphics->isFullscreen = 1;
    CGameInfo::m_gameInfo.unknownGraphicsOptions &= 0xfffffff9;
    g_pGraphics->field913_0x3bc &= 0xfffffff7;
    g_pGraphics->field913_0x3bc &= 0xffffffef;
    g_pGraphics->field913_0x3bc &= 0xffffff7f;
    CGameInfo::m_gameInfo.unknownGraphicsOptions &= 0xfffffff7;
    g_pGraphics->field913_0x3bc &= 0xffffffdf;
    CGameInfo::m_gameInfo.unknownGraphicsOptions &= 0xffffffef;
    g_pGraphics->field913_0x3bc &= 0xffffffbf;
    CGameInfo::m_gameInfo.unknownGraphicsOptions &= 0xfff3e01f;
    g_pGraphics->field913_0x3bc &= 0xfffffffe;
    g_pGraphics->field913_0x3bc &= 0xfffffffd;

    unsigned int unknownGraphicsOptions = CGameInfo::m_gameInfo.unknownGraphicsOptions;
    CGameInfo::m_gameInfo.unknownGraphicsOptions = (unknownGraphicsOptions & 0xfe3fffff) | 0x200000;
    
    g_pGraphics->field917_0x3c0 = 1;
    
    CGameInfo::m_gameInfo.field_0x34 =  (CGameInfo::m_gameInfo.field_0x34 & 0xfffffffe) | 2;
    CGameInfo::m_gameInfo.unknownGraphicsOptions = (unknownGraphicsOptions & 0xcbffffff) | 0xa000000;
    
    g_pGraphics->field913_0x3bc &= 0xfffffffb;
}

// FUNCTION: CMR2 0x004a78a0
void CGraphics::FUN_004a78a0(unsigned int screenWidth, unsigned int screenHeight, unsigned int colourDepth, unsigned int param4, unsigned int param5) {
    if (m_unk0x00520b7c == 0) {
        CSound::FUN_004a2b50(TRUE);
        FUN_004a5be0(); // TODO: UNFINISHED
        ReleaseDirect3D();
        ReleaseSurfaces();
    }

    FUN_004a8bd0(param4);
    FUN_004a8d90(param5);

    BOOL b = FUN_004a7910(screenWidth, screenHeight, colourDepth);
}

// FUNCTION: CMR2 0x004a5be0
BOOL CGraphics::FUN_004a5be0(void) {
    int index, textureID, iVar4, iVar7;

    m_pTextureManager->pDD->EvictManagedTextures();
    m_unk0x0065fa2c = 0;

    index = 0;
    textureID = 0;
    do {
        Texture* pTexture = m_pTextureManager->textureBuffer[index];
        if (pTexture != NULL && pTexture->pSurface != NULL && textureID == pTexture->textureId) {
            
            if (pTexture->pSurface->Release() == 0) {
                pTexture->pSurface = NULL;
            }
        }
        
        index++;
        textureID++;
    } while (index < 2048);

    FUN_004a5ba0();
    index = 0;

    if (m_unk0x0065fa28 != 0) {
        do {
            iVar4 = 0x5f0;
            iVar7 = 0x734;

            do {
                Texture* pTexture = m_pTextureManager->textureBuffer2[index];
                if (pTexture != NULL) {
                    IDirectDrawSurface7* pOther = pTexture->pSurface;
                    if (pOther->Release() == 0) {
                        pTexture->pSurface = NULL;
                    }
                }
            } while (0x71f < iVar7);
            
            index++;
        } while (index < m_unk0x0065fa28);
    }

    return TRUE;
}

// FUNCTION: CMR2 0x004a8810
BOOL CGraphics::ReleaseDirect3D(void)
{
    ReleaseVertexBuffers();

    if (m_pTextureManager->pD3D != NULL && m_pTextureManager->pD3D->Release() == 0)
        m_pTextureManager->pD3D = NULL;
    
    m_pTextureManager->pD3D = NULL;
    
    if (m_pTextureManager->pDD != NULL && m_pTextureManager->pDD->Release() == 0)
        m_pTextureManager->pDD = NULL;
    
    m_pTextureManager->pDD = NULL;
    return TRUE;
}

// GLOBAL: CMR2 0x0065fa30
unsigned int g_unk0x0065fa30;

// Releases the 64 cached surfaces held by m_unk0x0065aee8.
// FUNCTION: CMR2 0x004a5ba0
void CGraphics::FUN_004a5ba0(void)
{
    int i;

    for (i = 0; i < 64; i++) {
        if (m_unk0x0065aee8[i].pSurface != NULL) {
            if (m_unk0x0065aee8[i].pSurface->Release() == 0)
                m_unk0x0065aee8[i].pSurface = NULL;
        }
    }
    g_unk0x0065fa30 = 0;
}

// FUNCTION: CMR2 0x004b1de0
void CGraphics::ReleaseVertexBuffers(void) {
    int index = 99;
    if (m_pTextureManager->pVertexBuffer3 != NULL && m_pTextureManager->pVertexBuffer3->Release() == 0)
        m_pTextureManager->pVertexBuffer3 = NULL;
    
    if (m_pTextureManager->pVertexBuffer2 != NULL && m_pTextureManager->pVertexBuffer2->Release() == 0)
        m_pTextureManager->pVertexBuffer2 = NULL;

    if (m_pTextureManager->pVertexBuffer1 != NULL && m_pTextureManager->pVertexBuffer1->Release() == 0)
        m_pTextureManager->pVertexBuffer1 = NULL;

    // not sure if this loop is fully correct or not
    do {
        if (m_pTextureManager->pVertexBuffers[index] != NULL && m_pTextureManager->pVertexBuffers[index]->Release() == 0)
            m_pTextureManager->pVertexBuffers[index] = NULL;

        index--;
    } while (index >= 0);

    m_unk0x006dd890 = 0;
}

// FUNCTION: CMR2 0x004a8040
void CGraphics::ReleaseSurfaces(void) {
    if (g_pGraphics->pSurface3 != NULL && g_pGraphics->pSurface3->Release() == 0)
        g_pGraphics->pSurface3 = NULL;

    g_pGraphics->pSurface3 = NULL;

    if (g_pGraphics->pBackBufferSurface != NULL && g_pGraphics->pBackBufferSurface->Release() == 0)
        g_pGraphics->pBackBufferSurface = NULL;

    g_pGraphics->pBackBufferSurface = NULL;

    if (g_pGraphics->pPrimarySurface != NULL && g_pGraphics->pPrimarySurface->Release() == 0)
        g_pGraphics->pPrimarySurface = NULL;

    g_pGraphics->pPrimarySurface = NULL;
    
    g_pGraphics->pDD7->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DDSCL_NORMAL);
    if (g_pGraphics->isFullscreen != 0) {
        g_pGraphics->pDD7->RestoreDisplayMode();
    }

    if (g_pGraphics->pDD7 != NULL && g_pGraphics->pDD7->Release() == 0) {
        g_pGraphics->pDD7 = NULL;
    }
}

// FUNCTION: CMR2 0x004a8bd0
void CGraphics::FUN_004a8bd0(int param1) {
    m_unk0x00663b1c = param1;
}

// FUNCTION: CMR2 0x004a8d90
void CGraphics::FUN_004a8d90(int param1) {
    m_unk0x00663b24 = param1;
}

struct GraphicsStack
{
    LPDIRECTDRAWCLIPPER pDDClipper;
    DDSURFACEDESC2 ddsd;
    tagRECT lpWindowRect;
    tagRECT lpClientRect;
    DDSURFACEDESC2 ddsdDisplayMode;
};

// FUNCTION: CMR2 0x004a7910
BOOL CGraphics::FUN_004a7910(int screenWidth, int screenHeight, int colourDepth) {
    BOOL findMatchingDevice = FALSE;
    DWORD tier = 0;
    HDC hdc = 0;
    LPDIRECTDRAW7 pDD7;
    DWORD capFlag = 0x10004000;
    GraphicsStack s;

    int iHorzRes = 0, iVertRes = 0, iVar6 = 0, cx = 0;

    m_pTextureManager->textureInfo2 = NULL;
    m_pTextureManager->textureInfo5 = NULL;
    m_pTextureManager->textureInfo1 = NULL;

    FUN_004bdb60(&m_unk0x0065fd08,CMain::m_hWndList[CMain::m_hWndIx]);
    tier = FUN_004a96c0(m_unk0x00663b1c);
    while (tier == 0) {
        if (m_unk0x00663b1c + 1 > m_unk0x0065fd08.count - 1) {
            return FALSE;
        }

        FUN_004a8bd0(m_unk0x00663b1c + 1U);
        tier = FUN_004a96c0(m_unk0x00663b1c);
    }

    m_unk0x0065fd08.reserved = m_unk0x00663b1c;
    DirectDrawCreateEx(m_unk0x0065fd08.entries[m_unk0x00663b1c].device.pGUID, (LPVOID*)&g_pGraphics->pDD, IID_IDirectDraw7, NULL);

    g_pGraphics->pDD->QueryInterface(IID_IDirectDraw7, (LPVOID*)&g_pGraphics->pDD7);
    if (g_pGraphics->pDD != NULL && g_pGraphics->pDD->Release() == 0) {
        g_pGraphics->pDD = NULL;
    }

    g_pGraphics->resX = screenWidth;
    g_pGraphics->resY = screenHeight;
    g_pGraphics->depth = colourDepth;
    g_pGraphics->screenResX = GetDeviceCaps(GetDC(NULL), HORZRES);
    g_pGraphics->screenResY = GetDeviceCaps(GetDC(NULL), VERTRES);
    ReleaseDC(NULL, GetDC(NULL));

    tier = FUN_004a8bc0();
    tier = FUN_004a96e0(tier);
    if (tier == 0) {
        g_pGraphics->isFullscreen = 1;
        FUN_004a8d90(0);
        m_unk0x00660040[tier].surfaceCap = 1;
    }

    g_pGraphics->isFullscreen = 1;
    
    if (g_pGraphics->isFullscreen == 0) {
        g_pGraphics->pDD7->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DDSCL_NORMAL);
        GetWindowRect(CMain::m_hWndList[CMain::m_hWndIx], &s.lpWindowRect);
        GetClientRect(CMain::m_hWndList[CMain::m_hWndIx], &s.lpClientRect);

        SetWindowPos(CMain::m_hWndList[CMain::m_hWndIx], NULL,
            GetSystemMetrics(SM_CXSCREEN) / 2 - 0x140,
            GetSystemMetrics(SM_CYSCREEN) / 2 - 0xf0,
            s.lpWindowRect.right + 0x280 - s.lpWindowRect.left + s.lpClientRect.left - s.lpClientRect.right,
            s.lpWindowRect.bottom + 0x1e0 - s.lpWindowRect.top + s.lpClientRect.top - s.lpClientRect.bottom,
            4);
        UpdateWindow(CMain::m_hWndList[CMain::m_hWndIx]);
        ShowWindow(CMain::m_hWndList[CMain::m_hWndIx], SW_SHOWNORMAL);
    } else {
        g_pGraphics->pDD7->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], 0x851);
    }

    g_pGraphics->pDD7->QueryInterface(IID_IDirect3D7, (LPVOID*)&m_pTextureManager);
    m_unk0x00663b18 = 0;
    m_unk0x00663b20 = 0;

    DirectDrawEnumerateExA(&FUN_004a8b30_DDEnumCallback, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);

    m_pTextureManager->pDD->EnumDevices(FUN_004a8c30_DDEnumCallback, NULL);

    m_displayCount = 0;
    g_pGraphics->pDD7->EnumDisplayModes(0, NULL, NULL, FUN_004a8da0);

    findMatchingDevice = FUN_004a8f60(screenWidth, screenHeight, colourDepth);
    s.ddsdDisplayMode.dwSize = sizeof(DDSURFACEDESC2);
    if (findMatchingDevice != FALSE) {
        g_pGraphics->resX = screenWidth;
        g_pGraphics->resY = screenHeight;
        g_pGraphics->depth = colourDepth;
    } else {
        g_pGraphics->resX = 640;
        g_pGraphics->resY = 480;
        g_pGraphics->pDD7->GetDisplayMode(&s.ddsdDisplayMode);
        g_pGraphics->depth = s.ddsdDisplayMode.ddpfPixelFormat.dwRGBBitCount;
    }

    FUN_004a8ec0(g_pGraphics->resX, g_pGraphics->resY, g_pGraphics->depth);

    Unk0x0065ff90* deviceEntry = &m_unk0x0065ff90[m_unk0x00663b24];
    D3DTextureManager* textureManager = m_pTextureManager;

    textureManager->deviceGUID = deviceEntry->guid;

    if (g_pGraphics->isFullscreen != 0) {
        g_pGraphics->pDD7->SetDisplayMode(g_pGraphics->resX, g_pGraphics->resY, g_pGraphics->depth, 0, 0);
        DWORD isFullScreen = g_pGraphics->isFullscreen;
        if (isFullScreen == 0) {
            memset(&s.ddsd, 0, sizeof(DDSURFACEDESC2));
            s.ddsd.dwSize = sizeof(DDSURFACEDESC2);
            s.ddsd.dwFlags = DDSD_CAPS;
            s.ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
            
            if (FUN_004a8d60() != 1 && FUN_004a8d60() != 2) {
                s.ddsd.ddsCaps.dwCaps |= 0x800;
            } else {
                s.ddsd.ddsCaps.dwCaps |= capFlag;
            }

            if (g_pGraphics->pDD7->CreateSurface(&s.ddsd, &g_pGraphics->pPrimarySurface, 0) != 0)
                return FALSE;

            if (g_pGraphics->pDD7->CreateClipper(0, &s.pDDClipper, 0) != 0)
                return FALSE;

            s.pDDClipper->SetHWnd(0, CMain::m_hWndList[CMain::m_hWndIx]);
            g_pGraphics->pPrimarySurface->SetClipper(s.pDDClipper);
            if (s.pDDClipper != NULL) {
                if (s.pDDClipper->Release() == 0) {
                    s.pDDClipper = NULL;
                }
            }

            memset(&s.ddsd, 0, sizeof(DDSURFACEDESC2));
            s.ddsd.dwSize = sizeof(DDSURFACEDESC2);
            s.ddsd.dwHeight = g_pGraphics->resY;
            s.ddsd.dwWidth = g_pGraphics->resX;
            s.ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
            s.ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_OFFSCREENPLAIN;

            if (FUN_004a8d60() != 1) {
                if (FUN_004a8d60() != 2) {
                    s.ddsd.ddsCaps.dwCaps |= 0x800;
                }
            }

            if (g_pGraphics->pDD7->CreateSurface(&s.ddsd, &g_pGraphics->pBackBufferSurface, 0) != 0)
                return FALSE;            
        } else {
            memset(&s.ddsd, 0, sizeof(DDSURFACEDESC2));
            s.ddsd.dwSize = sizeof(DDSURFACEDESC2);
            s.ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
            s.ddsd.dwBackBufferCount = 1;
            s.ddsd.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

            if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2) {
                s.ddsd.ddsCaps.dwCaps |= capFlag;
            } else {
                s.ddsd.ddsCaps.dwCaps |= 0x800;
            }

            if (g_pGraphics->pDD7->CreateSurface(&s.ddsd, &g_pGraphics->pPrimarySurface, 0) != 0)
                return FALSE;

            memset(&s.ddsd, 0, sizeof(DDSURFACEDESC2));
            s.ddsd.dwSize = sizeof(DDSURFACEDESC2);
            s.ddsd.dwFlags = DDSD_CAPS;
            s.ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_3DDEVICE;

            if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2) {
                s.ddsd.ddsCaps.dwCaps |= capFlag;
            } else {
                s.ddsd.ddsCaps.dwCaps |= 0x800;
            }

            if (g_pGraphics->pPrimarySurface->GetAttachedSurface(&s.ddsd.ddsCaps, &g_pGraphics->pBackBufferSurface) != 0)
                return FALSE;
        }
    }

    g_pGraphics->field590_0x26c = 0;
    g_pGraphics->field591_0x26e = 0;
    g_pGraphics->field592_0x270 = (WORD)g_pGraphics->resX;
    g_pGraphics->field593_0x272 = (WORD)g_pGraphics->resY;

    LPDWORD pField590AsDword = (LPDWORD)&g_pGraphics->field590_0x26c;
    g_pGraphics->field295_0x13c = pField590AsDword[0];
    g_pGraphics->field296_0x140 = pField590AsDword[1];
    
    if (m_unk0x00520b7c != 0) {
        m_releaseSurfaceCallbackID = CGame::RegisterCallback(ReleaseSurfaces,NULL);
    }    

    return TRUE;
}

// FUNCTION: CMR2 0x004bdb60
BOOL CGraphics::FUN_004bdb60(DDDeviceEnumBuffer* param1, HWND hWnd) {
    LPDIRECTDRAW7 pDirectDraw = NULL;
    LPDIRECTDRAW7 pDirectDrawConfirm = NULL;
    DDEnumDeviceBufferEntry* pEntry;
    int index = 0;

    if (m_unk0x0081709c == FALSE)  {
        m_displayDevicePool.count = 0;
        memset(param1, 0, 0x288);

        DirectDrawEnumerateExA(&FUN_004bdb60_DDEnumCallback, param1, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
        param1->count = m_displayDevicePool.count;

        if (m_displayDevicePool.count > 0) {
            pEntry = param1->entries;
            do {
                DirectDrawCreateEx(pEntry->device.pGUID, (LPVOID*)&pDirectDraw, IID_IDirectDraw7, NULL);
                pDirectDraw->QueryInterface(IID_IDirectDraw7, (LPVOID*)&pDirectDrawConfirm);

                pDirectDrawConfirm->SetCooperativeLevel(hWnd, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX);
                FUN_004bdd30(pEntry, pDirectDrawConfirm);

                pDirectDrawConfirm->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

                FUN_004bde20(pEntry, pDirectDrawConfirm);

                if (pDirectDrawConfirm != NULL && pDirectDrawConfirm->Release() == 0) {
                    pDirectDrawConfirm = NULL;
                }
                
                if (pDirectDraw != NULL && pDirectDraw->Release() == 0) {
                    pDirectDraw = NULL;
                }

                index++;
                pEntry++;
            } while (index < param1->count);
        }

        // this suggests that `m_displayDevicePool` isnt correct? or they're doing something gnarly
        memcpy((BYTE*)&m_displayDevicePool + 0x20, param1, sizeof(DDDeviceEnumBuffer));
        m_unk0x0081709c = TRUE;
    }
    
    return TRUE;
}

// FUNCTION: CMR2 0x004bdc80
BOOL CGraphics::FUN_004bdb60_DDEnumCallback(GUID* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName,
                                             LPVOID lpContext, HMONITOR hMonitor)
{
    DDDeviceEnumBuffer* pBuffer = (DDDeviceEnumBuffer*)lpContext;

    if (m_displayDevicePool.count == 10)
        return FALSE;

    if (lpGUID == NULL) {
        pBuffer->entries[m_displayDevicePool.count].device.pGUID = NULL;
    } else {
        pBuffer->entries[m_displayDevicePool.count].device.guid = *lpGUID;
        pBuffer->entries[m_displayDevicePool.count].device.pGUID = &pBuffer->entries[m_displayDevicePool.count].device.guid;
    }

    m_lifetimeDisplayDeviceCount++;
    m_displayDevicePool.count++;

    return TRUE;
}

// FUNCTION: CMR2 0x004bdd30
BOOL CGraphics::FUN_004bdd30(DDEnumDeviceBufferEntry *pEnumDevice,IDirectDraw7 *pDevice) {
    HDC hdc;
    int hRes, vRes, bpp;
    LPDDCAPS pDriverCaps;
    DDCAPS driverCaps, helCaps;
    DWORD totalLocalVidMem;

    if (pEnumDevice->device.pGUID == NULL) {
        hdc = GetDC(NULL);
        hRes = GetDeviceCaps(hdc, HORZRES);
        vRes = GetDeviceCaps(hdc, VERTRES);
        bpp = GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(NULL, hdc);

        if (bpp == 8 || bpp != 0x10) {
            m_totalPixelsForScreen = vRes * hRes;
        } else {
            m_totalPixelsForScreen = vRes * hRes * 2;
        }
    }

    pDriverCaps = &driverCaps;

    driverCaps.dwSize = sizeof(DDCAPS);
    helCaps.dwSize = sizeof(DDCAPS);

    pDevice->GetCaps(pDriverCaps, &helCaps);
    pEnumDevice->capFlag1 = driverCaps.dwCaps & 1;
    pEnumDevice->capFlag200 = driverCaps.dwCaps & 0x200;
    pEnumDevice->capFlag80000 = driverCaps.dwCaps2 & 0x80000;

    m_displayDevicePool.entries[0].caps.caps.dwCaps = DDSCAPS_LOCALVIDMEM;
    pDevice->GetAvailableVidMem((LPDDSCAPS2)&m_displayDevicePool.entries[0].device.pGUID, &totalLocalVidMem, NULL);

    if (totalLocalVidMem < 0x1c2000)
        pEnumDevice->capFlag1 = 0;

    return TRUE;
}

// FUNCTION: CMR2 0x004bde20
void CGraphics::FUN_004bde20(DDEnumDeviceBufferEntry *pEnumDevice,IDirectDraw7 *pDevice) {
    pDevice->QueryInterface(IID_IDirect3D7, (LPVOID*)&pDevice);

    ((IDirect3D7*)pDevice)->EnumDevices(FUN_004bde60, pEnumDevice);

    if (pDevice != NULL)
        pDevice->Release();
}

// FUNCTION: CMR2 0x004bde60
HRESULT CGraphics::FUN_004bde60(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpD3DDeviceDesc, LPVOID lpUserArg) {
    if ((lpD3DDeviceDesc->dpcTriCaps.dwTextureFilterCaps & 1) == 0)
        return 1;

    DDEnumDeviceBufferEntry* pEntry = (DDEnumDeviceBufferEntry*)lpUserArg;
    pEntry->capTextureFilter1 = 1;
    pEntry->capTextureFilter3 = 1;

    if ((lpD3DDeviceDesc->dpcTriCaps.dwTextureFilterCaps & 2) != 0)
        pEntry->capTextureFilter2 = 1;

    if ((lpD3DDeviceDesc->dwDevCaps & 0x100) != 0)
        pEntry->capHardwareRasterization = 1;

    DWORD deviceZBufferBitDepth = lpD3DDeviceDesc->dwDeviceZBufferBitDepth;
    if ((deviceZBufferBitDepth & 0x400) != 0) {
        pEntry->zBufferBitDepth = 0x10;
        pEntry->hasZBuffer = 1;
    } else if ((deviceZBufferBitDepth & 0x200) != 0) {
        pEntry->zBufferBitDepth = 0x18;
        pEntry->hasZBuffer = 1;
    } else if ((deviceZBufferBitDepth & 0x100) != 0) {
        pEntry->zBufferBitDepth = 0x20;
        pEntry->hasZBuffer = 1;
    }
    
    if ((lpD3DDeviceDesc->dwDeviceRenderBitDepth & 0x100) != 0) {
        pEntry->capRender16Bit = 1;
        return 1;
    }
    
    pEntry->capRender16Bit = 0;
    return 1;
}

// FUNCTION: CMR2 0x004a96c0
DWORD CGraphics::FUN_004a96c0(int param1) {
    return m_unk0x0065fd08.entries[param1].capFlag1;
}

// FUNCTION: CMR2 0x004a8bc0
INT32 CGraphics::FUN_004a8bc0(void) {
    return m_unk0x00663b1c;
}

// FUNCTION: CMR2 0x004a96e0
DWORD CGraphics::FUN_004a96e0(int param_1) {
    return m_unk0x0065fd08.entries[param_1].capFlag80000;
}

// FUNCTION: CMR2 0x004a8b30
BOOL CGraphics::FUN_004a8b30_DDEnumCallback(GUID* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hMonitor) {
    LPDIRECTDRAW7 lplpDD;
    DDDEVICEIDENTIFIER2 ddDeviceIdent;
    DirectDrawCreateEx(lpGUID, (LPVOID*)&lplpDD, IID_IDirectDraw7, NULL);
    
    lplpDD->GetDeviceIdentifier(&ddDeviceIdent, 0);
    if (lplpDD != NULL) {
        if (lplpDD->Release() == 0)
            lplpDD = NULL;
    }

    wsprintfA(m_unk0x006634d8[m_unk0x00663b18].unk_0x00, CRegKey::m_regKeyPathFormatValue, ddDeviceIdent.szDescription);

    m_unk0x00663b18++;

    return TRUE;
}

// FUNCTION: CMR2 0x004a8da0
HRESULT CGraphics::FUN_004a8da0(DDSURFACEDESC2* lpDDSurfaceDesc2, void* lpContext) {
    int width  = lpDDSurfaceDesc2->dwWidth, height = lpDDSurfaceDesc2->dwHeight, bpp = lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount;
    DWORD canRender16Bit = 0, dwTextureMem = 0, dwVidMem = 0;

    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    g_pGraphics->pDD7->GetDisplayMode(&ddsd);

    canRender16Bit = FUN_004a8bc0();
    canRender16Bit = DeviceCanRender16Bit(canRender16Bit);

    if (((canRender16Bit != 0 || bpp != 0x20) &&  (g_pGraphics->isFullscreen != 0 || ddsd.ddpfPixelFormat.dwFlags == bpp)) && (width >= 0x280 && height >= 0x1e0) && (bpp == 0x10 || bpp == 0x20)) {
        dwTextureMem = FUN_004bdd00(DDSCAPS_TEXTURE);
        dwVidMem = FUN_004bdd00(DDSCAPS_LOCALVIDMEM);
        if (dwTextureMem < dwVidMem) {
            dwTextureMem = FUN_004bdd00(DDSCAPS_LOCALVIDMEM);            
            dwVidMem = FUN_004bdd00(DDSCAPS_TEXTURE);
            dwTextureMem = dwTextureMem + -dwVidMem;
        } else {
            dwTextureMem = FUN_004bdd00(DDSCAPS_LOCALVIDMEM);
        }

        if ((bpp / 8) * height * width * 3 < dwTextureMem) {
            m_displays[m_displayCount].width = width;
            m_displays[m_displayCount].height = height;
            m_displays[m_displayCount].colourDepth = bpp;
            m_displayCount++;
        }
    }

    return DDENUMRET_OK;
}

// FUNCTION: CMR2 0x004a96f0
int CGraphics::DeviceCanRender16Bit(int param1) {
    return CGraphics::m_unk0x0065fd08.entries[param1].capRender16Bit;
}

// FUNCTION: CMR2 0x004bdd00
DWORD CGraphics::FUN_004bdd00(DWORD caps) {
  DDEnumDeviceBufferEntry* pDVar1 = &m_displayDevicePool.entries[0];
  
  pDVar1->caps.caps.dwCaps = caps;
  g_pGraphics->pDD7->GetAvailableVidMem(&pDVar1->caps.caps, &caps, NULL);

  return caps;
}

// FUNCTION: CMR2 0x004a8f60
BOOL CGraphics::FUN_004a8f60(int width, int height, int colourDepth)
{
    for (int i = 0; i < m_displayCount; i++) {
        if (m_displays[i].width == width && m_displays[i].height == height && m_displays[i].colourDepth == colourDepth)
            return TRUE;
    }

    return FALSE;
}

// FUNCTION: CMR2 0x004a8ec0
void CGraphics::FUN_004a8ec0(int width, int height, int colourDepth)
{
    for (int i = 0; i < m_displayCount; i++) {
        if (m_displays[i].width == width && m_displays[i].height == height && m_displays[i].colourDepth == colourDepth)
            m_selectedDisplayDeviceIx = i;
    }
}

// FUNCTION: CMR2 0x004a8d60
DWORD CGraphics::FUN_004a8d60(void) {
  return m_unk0x00660040[m_unk0x00663b24].surfaceCap;
}

// FUNCTION: CMR2 0x004a8c30
HRESULT CGraphics::FUN_004a8c30_DDEnumCallback(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpD3DDeviceDesc, LPVOID lpUserArg) {
    if (strcmp(lpDeviceName, m_direct3DHAL) == 0 && m_unk0x00660040[0].surfaceCap != 2) {
        m_unk0x0065ff90[0].guid = lpD3DDeviceDesc->deviceGUID;
        m_unk0x00660040[0].surfaceCap = 1;
    } else if (strcmp(lpDeviceName, m_direct3DTLHAL) == 0) {
        m_unk0x0065ff90[0].guid = lpD3DDeviceDesc->deviceGUID;
        m_unk0x00660040[0].surfaceCap = 2;
    }

    wsprintfA(m_unk0x0065ff90[0].deviceDesc, CRegKey::m_regKeyPathFormatValue, lpDeviceDescription);
    wsprintfA(m_unk0x0065ff90[0].deviceName, CRegKey::m_regKeyPathFormatValue, lpDeviceName);

    m_unk0x00663b20 = 1;

    return TRUE;
}

// FUNCTION: CMR2 0x004a5080
void CGraphics::BltTexture(Texture *pTexture, int surfaceIndex)
{
    RECT rect;

    rect.left = 0;
    rect.right = pTexture->width;
    rect.top = 0;
    rect.bottom = pTexture->height;
    if (pTexture->pSurface != NULL && m_unk0x0065aee8[surfaceIndex].pSurface != NULL)
        pTexture->pSurface->Blt(&rect, m_unk0x0065aee8[surfaceIndex].pSurface, NULL, DDBLT_WAIT, NULL);
}

// FUNCTION: CMR2 0x004a56c0
void CGraphics::UnlockTexture(Texture *pTexture)
{
    unsigned int i;

    for (i = 0; i < m_lockedTextureCount; i++) {
        if (pTexture == m_lockedTextures[i].pTexture)
            break;
    }
    if (i != m_lockedTextureCount) {
        m_lockedTextures[i].pTexture->pSurface->Unlock(NULL);
        m_lockedTextures[i].pTexture = NULL;
        m_lockedTextureCount--;
    }
}

// FUNCTION: CMR2 0x004a5730
unsigned int CGraphics::GetPixelRed(DDSURFACEDESC2 *pDesc, int x, int y)
{
    WORD *pPixel;
    int pad;
    unsigned int mask;
    unsigned int bits;
    int shift;
    int count;
    int i;

    pad = pDesc->lPitch - (pDesc->dwWidth * pDesc->ddpfPixelFormat.dwRGBBitCount >> 3);
    if (pDesc->ddpfPixelFormat.dwRGBBitCount == 16) {
        pPixel = (WORD *)pDesc->lpSurface + (pDesc->dwWidth + pad) * y + x;
        mask = pDesc->ddpfPixelFormat.dwRBitMask;
        bits = mask;
        for (shift = 0; shift < 32; shift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }

        count = 0;
        bits = mask;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        return (((*pPixel & mask) >> shift) & 0xff) << (8 - count);
    } else if (pDesc->ddpfPixelFormat.dwRGBBitCount == 32) {
        return (((DWORD *)pDesc->lpSurface)[(pDesc->dwWidth + pad) * y + x] & 0xff0000) >> 16;
    }
    return 0;
}

// FUNCTION: CMR2 0x004a57e0
unsigned int CGraphics::GetPixelAlpha(DDSURFACEDESC2 *pDesc, int x, int y)
{
    WORD *pPixel;
    int pad;
    unsigned int mask;
    unsigned int bits;
    int shift;
    int count;
    int i;

    pad = pDesc->lPitch - (pDesc->dwWidth * pDesc->ddpfPixelFormat.dwRGBBitCount >> 3);
    if (pDesc->ddpfPixelFormat.dwRGBBitCount == 16) {
        pPixel = (WORD *)pDesc->lpSurface + (pDesc->dwWidth + pad) * y + x;
        mask = pDesc->ddpfPixelFormat.dwRGBAlphaBitMask;
        bits = mask;
        for (shift = 0; shift < 32; shift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }

        count = 0;
        bits = mask;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        return (((*pPixel & mask) >> shift) & 0xff) << (8 - count);
    } else if (pDesc->ddpfPixelFormat.dwRGBBitCount == 32) {
        return ((DWORD *)pDesc->lpSurface)[(pDesc->dwWidth + pad) * y + x] >> 24;
    }
    return 0;
}

// FUNCTION: CMR2 0x004a5d10
BOOL CGraphics::FreeTextureBuffers(void)
{
    int i;

    for (i = 0; i < 2048; i++) {
        if (m_pTextureManager->textureBuffer[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(m_pTextureManager->textureBuffer[i]);
            m_pTextureManager->textureBuffer[i] = NULL;
            if (m_textureCount > 0)
                m_textureCount--;
        }
    }
    for (i = 0; i < 20; i++) {
        if (m_pTextureManager->textureBuffer2[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(m_pTextureManager->textureBuffer2[i]);
            m_pTextureManager->textureBuffer2[i] = NULL;
            if (m_unk0x0065fa28 > 0)
                m_unk0x0065fa28--;
        }
    }
    return TRUE;
}

// FUNCTION: CMR2 0x004a5fe0
unsigned int CGraphics::FUN_004a5fe0(void)
{
    return m_unk0x0065fa2c >> 10;
}

// FUNCTION: CMR2 0x004a5ff0
void CGraphics::FUN_004a5ff0(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x00520b2c = param1;
}

// FUNCTION: CMR2 0x004a6040
void CGraphics::FUN_004a6040(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x0065fa44 = param1 - 0x80;
}

// FUNCTION: CMR2 0x004a6060
void CGraphics::FUN_004a6060(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x00520b30 = param1;
}

// FUNCTION: CMR2 0x004a60b0
void CGraphics::FUN_004a60b0(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x0065fa48 = param1 - 0x80;
}

// FUNCTION: CMR2 0x004a87c0
HRESULT CALLBACK CGraphics::CopyZBufferPixelFormat(DDPIXELFORMAT *pSrc, LPVOID lpContext)
{
    DDPIXELFORMAT *pDst = (DDPIXELFORMAT *)lpContext;

    if (pSrc != NULL && pDst != NULL) {
        if (pDst->dwZBufferBitDepth != pSrc->dwZBufferBitDepth || (pSrc->dwFlags & DDPF_ZBUFFER) == 0) {
            pDst->dwZBufferBitDepth = 0;
            return D3DENUMRET_OK;
        }
        memcpy(pDst, pSrc, sizeof(DDPIXELFORMAT));
    }
    return D3DENUMRET_CANCEL;
}

// FUNCTION: CMR2 0x004a8be0
int CGraphics::FUN_004a8be0(void)
{
    return m_unk0x00663b18;
}

// FUNCTION: CMR2 0x004a8bf0
void CGraphics::GetDisplayDeviceNames(int index, LPSTR description, LPSTR name)
{
    wsprintfA(description, CRegKey::m_regKeyPathFormatValue, m_unk0x006634d8[index].unk_0x00);
    wsprintfA(name, CRegKey::m_regKeyPathFormatValue, m_unk0x006634d8[index].name);
}

// FUNCTION: CMR2 0x004a8d80
int CGraphics::FUN_004a8d80(void)
{
    return m_unk0x00663b24;
}

// FUNCTION: CMR2 0x004a8eb0
int CGraphics::GetSelectedDisplayDeviceIx(void)
{
    return m_selectedDisplayDeviceIx;
}

// FUNCTION: CMR2 0x004a8f10
DWORD CGraphics::GetDisplayCount(void)
{
    return m_displayCount;
}

// FUNCTION: CMR2 0x004a8f20
void CGraphics::GetDisplayMode(int index, DWORD *pWidth, DWORD *pHeight, DWORD *pColourDepth)
{
    *pWidth = m_displays[index].width;
    *pHeight = m_displays[index].height;
    *pColourDepth = m_displays[index].colourDepth;
}

// FUNCTION: CMR2 0x004a96d0
DWORD CGraphics::FUN_004a96d0(int param1)
{
    return m_unk0x0065fd08.entries[param1].capFlag200;
}

// FUNCTION: CMR2 0x004a6010
void CGraphics::FUN_004a6010(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x00520b34 = (float)param1 * m_oneOver128;
}

// FUNCTION: CMR2 0x004a6080
void CGraphics::FUN_004a6080(BYTE param1)
{
    if (param1 <= 0xff)
        m_unk0x00520b38 = (float)param1 * m_oneOver128;
}

// FUNCTION: CMR2 0x004a6670
TGAImageInfo *CGraphics::ParseTGAHeader(BYTE *pHeader)
{
    if (pHeader == NULL || pHeader[1] != 0 || *(short *)(pHeader + 8) != 0 || *(short *)(pHeader + 10) != 0 ||
        (pHeader[17] & 0x70) != 0 || pHeader[2] == 3 || pHeader[2] != 2)
        return NULL;

    if (pHeader[16] == 24) {
        m_tgaImageInfo.bytesPerPixel = 3;
    } else {
        if (pHeader[16] != 32)
            return NULL;
        if ((pHeader[17] & 0xf) != 8)
            return NULL;
        m_tgaImageInfo.bytesPerPixel = 4;
    }
    m_tgaImageInfo.width = *(unsigned short *)(pHeader + 12);
    m_tgaImageInfo.height = *(unsigned short *)(pHeader + 14);
    m_tgaImageInfo.pixels = pHeader + pHeader[0] + 18;
    return &m_tgaImageInfo;
}

#define RENDER_TEXTURE(i) ((RenderTexture *)m_pTextureManager->textureBuffer2[i])

// FUNCTION: CMR2 0x004a82c0
void CGraphics::RestoreSurfaces(void)
{
    unsigned int i;
    int face;

    if (g_pGraphics != NULL) {
        if (g_pGraphics->pPrimarySurface != NULL && g_pGraphics->pPrimarySurface->IsLost() != 0)
            g_pGraphics->pPrimarySurface->Restore();
        if (g_pGraphics->pBackBufferSurface != NULL && g_pGraphics->pBackBufferSurface->IsLost() != 0)
            g_pGraphics->pBackBufferSurface->Restore();
        if (g_pGraphics->pSurface3 != NULL && g_pGraphics->pSurface3->IsLost() != 0)
            g_pGraphics->pSurface3->Restore();

        for (i = 0; i < m_textureCount; i++) {
            if (m_pTextureManager->textureBuffer[i]->pSurface != NULL && m_pTextureManager->textureBuffer[i]->pSurface->IsLost() != 0)
                m_pTextureManager->textureBuffer[i]->pSurface->Restore();
        }

        for (i = 0; i < m_unk0x0065fa28; i++) {
            for (face = 0; face < 6; face++) {
                if (RENDER_TEXTURE(i)->faces[face].pSurface != NULL) {
                    if (RENDER_TEXTURE(i)->faces[face].pSurface->IsLost() != 0)
                        RENDER_TEXTURE(i)->faces[face].pSurface->Restore();
                    if (RENDER_TEXTURE(i)->pZBuffers[face]->IsLost() != 0)
                        RENDER_TEXTURE(i)->pZBuffers[face]->Restore();
                }
            }
        }
    }
}

// FUNCTION: CMR2 0x004bd970
void CGraphics::SetMipMapCount(DDSURFACEDESC2 *pDesc)
{
    unsigned int dim;
    int count;

    dim = pDesc->dwHeight;
    pDesc->dwFlags |= DDSD_MIPMAPCOUNT;
    if (pDesc->dwWidth > dim) {
        dim = pDesc->dwWidth;
        for (count = 0; count < 32; count++) {
            if (dim & 1)
                break;
            dim >>= 1;
        }
    } else {
        for (count = 0; count < 32; count++) {
            if (dim & 1)
                break;
            dim >>= 1;
        }
    }
    pDesc->dwMipMapCount = count & 0xff;
    if (pDesc->dwMipMapCount > 3)
        pDesc->dwMipMapCount = 3;
    pDesc->ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
}

// FUNCTION: CMR2 0x004bd9d0
void CGraphics::GetMipMapSurfaces(Texture *pTexture)
{
    IDirectDrawSurface7 *pSurface;
    DDSCAPS2 caps = { 0 };
    int i;

    pSurface = pTexture->pSurface;
    m_mipMapSurfaces[0] = NULL;
    m_mipMapSurfaces[1] = NULL;
    caps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
    if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2)
        caps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else
        caps.dwCaps2 = 0;
    caps.dwCaps3 = 0;
    caps.dwCaps4 = 0;

    for (i = 0; i < 2; i++) {
        if (FAILED(pSurface->GetAttachedSurface(&caps, &m_mipMapSurfaces[i])))
            return;
        pSurface = m_mipMapSurfaces[i];
    }
}

// FUNCTION: CMR2 0x004bda60
int CGraphics::GetMipMapDataSize(Texture *pTexture)
{
    int width;
    int height;
    int levels;
    int total;
    int i;
    int size;

    width = m_unk0x00816a80;
    height = m_unk0x00816a84;
    total = 0;
    levels = 0;
    for (i = 0; i < 2; i++) {
        if (m_mipMapSurfaces[i] == NULL)
            break;
        levels++;
    }
    for (i = levels; i > 0; i--) {
        width /= 2;
        height /= 2;
        total += height * width;
    }

    size = total * 2;
    if (pTexture->bitsPerPixel != 16)
        size = total * 4;
    return size;
}

// FUNCTION: CMR2 0x004bdad0
int CGraphics::GetMipMapPixelCount(Texture *pTexture)
{
    int width;
    int height;
    int levels;
    int total;
    int i;

    width = m_unk0x00816a80;
    height = m_unk0x00816a84;
    total = 0;
    levels = 0;
    for (i = 0; i < 2; i++) {
        if (m_mipMapSurfaces[i] == NULL)
            break;
        levels++;
    }
    for (i = levels; i > 0; i--) {
        width /= 2;
        height /= 2;
        total += height * width;
    }
    return total;
}

// FUNCTION: CMR2 0x004bdb20
void CGraphics::BltMipMaps(Texture *pTexture)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (m_mipMapSurfaces[i] != NULL)
            m_mipMapSurfaces[i]->Blt(NULL, pTexture->pSurface, NULL, DDBLT_WAIT, NULL);
    }
}

// FUNCTION: CMR2 0x004a50f0
void CGraphics::LockTexture(Texture *pTexture, RECT *pRect)
{
    unsigned int index;
    unsigned int i;
    unsigned int bits;
    int count;

    index = m_lockedTextureCount;
    if (pTexture == NULL || pTexture->pSurface == NULL)
        return;

    for (i = 0; i < m_lockedTextureCount; i++) {
        if (pTexture == m_lockedTextures[i].pTexture)
            return;
    }

    m_lockedTextures[m_lockedTextureCount].pTexture = pTexture;
    memset(&m_lockedTextures[m_lockedTextureCount].desc, 0, sizeof(DDSURFACEDESC2));
    m_lockedTextures[m_lockedTextureCount].desc.dwSize = sizeof(DDSURFACEDESC2);
    m_lockedTextures[index].pTexture->pSurface->Lock(pRect, &m_lockedTextures[m_lockedTextureCount].desc, DDLOCK_WAIT, NULL);

    if (m_lockedTextures[m_lockedTextureCount].desc.ddpfPixelFormat.dwRGBBitCount == 16) {
        m_lockedTextures[m_lockedTextureCount].masks[0] = m_lockedTextures[m_lockedTextureCount].desc.ddpfPixelFormat.dwRBitMask;
        m_lockedTextures[m_lockedTextureCount].masks[1] = m_lockedTextures[m_lockedTextureCount].desc.ddpfPixelFormat.dwGBitMask;
        m_lockedTextures[m_lockedTextureCount].masks[2] = m_lockedTextures[m_lockedTextureCount].desc.ddpfPixelFormat.dwBBitMask;
        m_lockedTextures[m_lockedTextureCount].masks[3] = m_lockedTextures[m_lockedTextureCount].desc.ddpfPixelFormat.dwRGBAlphaBitMask;

        bits = m_lockedTextures[m_lockedTextureCount].masks[0];
        for (count = 0; count < 32; count++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].shifts[0] = (BYTE)count;
        bits = m_lockedTextures[m_lockedTextureCount].masks[1];
        for (count = 0; count < 32; count++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].shifts[1] = (BYTE)count;
        bits = m_lockedTextures[m_lockedTextureCount].masks[2];
        for (count = 0; count < 32; count++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].shifts[2] = (BYTE)count;
        bits = m_lockedTextures[m_lockedTextureCount].masks[3];
        for (count = 0; count < 32; count++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].shifts[3] = (BYTE)count;

        bits = m_lockedTextures[m_lockedTextureCount].masks[0];
        for (count = 0; count < 32; count++) {
            if (bits == 0)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].depths[0] = (BYTE)count - 8;
        bits = m_lockedTextures[m_lockedTextureCount].masks[1];
        for (count = 0; count < 32; count++) {
            if (bits == 0)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].depths[1] = (BYTE)count - 8;
        bits = m_lockedTextures[m_lockedTextureCount].masks[2];
        for (count = 0; count < 32; count++) {
            if (bits == 0)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].depths[2] = (BYTE)count - 8;
        bits = m_lockedTextures[m_lockedTextureCount].masks[3];
        for (count = 0; count < 32; count++) {
            if (bits == 0)
                break;
            bits >>= 1;
        }
        m_lockedTextures[m_lockedTextureCount].depths[3] = (BYTE)count - 8;
    }
    m_lockedTextureCount++;
}

// FUNCTION: CMR2 0x004a76d0
RenderTexture *CGraphics::CreateCubeMapSurfaces(RenderTexture *pTexture)
{
    DDSURFACEDESC2 desc;
    int i;

    memset(&desc, 0, sizeof(desc));
    desc.dwHeight = m_cubeMapSize;
    desc.dwWidth = m_cubeMapSize;
    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX;
    desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES;
    desc.ddsCaps.dwCaps3 = 0;
    desc.ddsCaps.dwCaps4 = 0;
    desc.ddpfPixelFormat = m_pTextureManager->textureInfo1->desc.ddpfPixelFormat;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    g_pGraphics->pDD7->CreateSurface(&desc, &pTexture->faces[0].pSurface, NULL);

    for (i = 1; i < 6; i++) {
        if (i == 1)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX;
        else if (i == 2)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY;
        else if (i == 3)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY;
        else if (i == 4)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ;
        else if (i == 5)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ;
        pTexture->faces[0].pSurface->GetAttachedSurface(&desc.ddsCaps, &pTexture->faces[i].pSurface);
    }

    for (i = 0; i < 6; i++) {
        desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        desc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
        desc.ddsCaps.dwCaps2 = 0;
        desc.ddsCaps.dwCaps3 = 0;
        desc.ddsCaps.dwCaps4 = 0;
        desc.ddpfPixelFormat = m_pTextureManager->ddpfZBuffer;
        desc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
        g_pGraphics->pDD7->CreateSurface(&desc, &pTexture->pZBuffers[i], NULL);
        pTexture->faces[i].pSurface->AddAttachedSurface(pTexture->pZBuffers[i]);
    }
    return pTexture;
}

// FUNCTION: CMR2 0x004a91b0
HRESULT CALLBACK CGraphics::EnumTextureFormatsCallback(DDPIXELFORMAT *pddpf, LPVOID lpContext)
{
    int i;
    int count;
    unsigned int bits;
    unsigned int alphaMask;
    unsigned int rBits;
    unsigned int gBits;
    unsigned int bBits;
    unsigned int aBits;
    int rShift;
    int gShift;
    int bShift;
    int aShift;
    TextureFormat *pFormat;
    HRESULT result;

    result = D3DENUMRET_OK;
    if (pddpf->dwFlags & DDPF_PALETTEINDEXED8)
        return result;

    if (pddpf->dwFlags & DDPF_RGB) {
        alphaMask = ~(pddpf->dwRBitMask | pddpf->dwGBitMask | pddpf->dwBBitMask) & pddpf->dwRGBAlphaBitMask;
        bits = pddpf->dwRBitMask;
        count = 0;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        rBits = (BYTE)count;
    rBits = rBits;
        bits = pddpf->dwGBitMask;
        count = 0;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        gBits = (BYTE)count;
    gBits = gBits;
        bits = pddpf->dwBBitMask;
        count = 0;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        bBits = (BYTE)count;
    bBits = bBits;
        bits = alphaMask;
        count = 0;
        for (i = 32; i != 0; i--) {
            if (bits & 1)
                count++;
            bits >>= 1;
        }
        aBits = (BYTE)count;
    aBits = aBits;

        if (rBits < 1 || gBits < 1 || bBits < 1)
            return result;

        bits = pddpf->dwRBitMask;
        for (rShift = 0; rShift < 32; rShift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        bits = pddpf->dwGBitMask;
        for (gShift = 0; gShift < 32; gShift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        bits = pddpf->dwBBitMask;
        for (bShift = 0; bShift < 32; bShift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }
        bits = alphaMask;
        for (aShift = 0; aShift < 32; aShift++) {
            if (bits & 1)
                break;
            bits >>= 1;
        }

        if (bBits + aBits + gBits + rBits <= 16) {
            if (aBits == 0) {
                if (rBits == 5 && (gBits == 5 || gBits == 6) && bBits == 5 && m_texFormat16.bits[2] != 6) {
                    m_texFormat16.desc.ddpfPixelFormat = *pddpf;
                    m_texFormat16.bits[2] = (BYTE)gBits;
                    m_texFormat16.bits[1] = (BYTE)bBits;
                    m_texFormat16.shifts[2] = (BYTE)gShift;
                    m_texFormat16.shifts[1] = (BYTE)bShift;
                    m_texFormat16.bits[0] = 5;
                    m_texFormat16.bits[3] = 0;
                    m_texFormat16.shifts[0] = (BYTE)rShift;
                    m_texFormat16.shifts[3] = (BYTE)aShift;
                    m_hasTexFormat16 = TRUE;
                    return result;
                }
            } else {
                if (rBits == 5) {
                    if (gBits != 5)
                        return result;
                    if (bBits != 5)
                        return result;
                    if (aBits != 1)
                        return result;
                } else {
                    if (rBits != 4)
                        return result;
                    if (gBits != 4)
                        return result;
                    if (bBits != 4)
                        return result;
                    if (aBits != 4)
                        return result;
                }
                if (m_texFormat16Alpha.bits[0] != 4) {
                m_texFormat16Alpha.desc.ddpfPixelFormat = *pddpf;
            m_texFormat16Alpha.bits[2] = (BYTE)gBits;
            m_texFormat16Alpha.bits[0] = (BYTE)rBits;
            m_texFormat16Alpha.bits[1] = (BYTE)bBits;
            m_texFormat16Alpha.bits[3] = (BYTE)aBits;
            m_texFormat16Alpha.shifts[0] = (BYTE)rShift;
            m_texFormat16Alpha.shifts[1] = (BYTE)bShift;
            m_texFormat16Alpha.shifts[2] = (BYTE)gShift;
            m_texFormat16Alpha.shifts[3] = (BYTE)aShift;
                m_hasTexFormat16Alpha = TRUE;
                    return result;
                }
            }
        } else {
            if (bBits + aBits + gBits + rBits == 24) {
            m_texFormat24.desc.ddpfPixelFormat = *pddpf;
            m_texFormat24.bits[2] = (BYTE)gBits;
            m_texFormat24.bits[0] = (BYTE)rBits;
            m_texFormat24.bits[1] = (BYTE)bBits;
            m_texFormat24.bits[3] = (BYTE)aBits;
            m_texFormat24.shifts[0] = (BYTE)rShift;
            m_texFormat24.shifts[1] = (BYTE)bShift;
            m_texFormat24.shifts[2] = (BYTE)gShift;
            m_texFormat24.shifts[3] = (BYTE)aShift;
                m_hasTexFormat24 = TRUE;
                return result;
            }
            if (bBits + aBits + gBits + rBits == 32) {
            m_texFormat32.desc.ddpfPixelFormat = *pddpf;
            m_texFormat32.bits[2] = (BYTE)gBits;
            m_texFormat32.bits[0] = (BYTE)rBits;
            m_texFormat32.bits[1] = (BYTE)bBits;
            m_texFormat32.bits[3] = (BYTE)aBits;
            m_texFormat32.shifts[0] = (BYTE)rShift;
            m_texFormat32.shifts[1] = (BYTE)bShift;
            m_texFormat32.shifts[2] = (BYTE)gShift;
            m_texFormat32.shifts[3] = (BYTE)aShift;
                m_hasTexFormat32 = TRUE;
                return result;
            }
        }
    } else {
        if (pddpf->dwFlags & DDPF_BUMPLUMINANCE) {
            pFormat = NULL;
            if (pddpf->dwBumpBitCount == 16) {
                pFormat = &m_texFormatBump16;
                m_hasTexFormatBump16 = TRUE;
            } else if (pddpf->dwBumpBitCount == 24 || pddpf->dwBumpBitCount == 32) {
                pFormat = &m_texFormatBump32;
                m_hasTexFormatBump32 = TRUE;
            }
            pFormat->desc.ddpfPixelFormat = *pddpf;
            bits = pddpf->dwBumpDuBitMask;
            rBits = 0;
            for (i = 32; i != 0; i--) {
                if (bits & 1)
                    rBits++;
                bits >>= 1;
            }
        pFormat->bits[0] = (BYTE)rBits;
            bits = pddpf->dwBumpDvBitMask;
            rBits = 0;
            for (i = 32; i != 0; i--) {
                if (bits & 1)
                    rBits++;
                bits >>= 1;
            }
        pFormat->bits[1] = (BYTE)rBits;
            bits = pddpf->dwBumpLuminanceBitMask;
            rBits = 0;
            for (i = 32; i != 0; i--) {
                if (bits & 1)
                    rBits++;
                bits >>= 1;
            }
        pFormat->bits[2] = (BYTE)rBits;
            bits = pddpf->dwBumpDuBitMask;
            for (rShift = 0; rShift < 32; rShift++) {
                if (bits & 1)
                    break;
                bits >>= 1;
            }
        pFormat->shifts[0] = (BYTE)rShift;
            bits = pddpf->dwBumpDvBitMask;
            for (rShift = 0; rShift < 32; rShift++) {
                if (bits & 1)
                    break;
                bits >>= 1;
            }
        pFormat->shifts[1] = (BYTE)rShift;
            bits = pddpf->dwBumpLuminanceBitMask;
            for (rShift = 0; rShift < 32; rShift++) {
                if (bits & 1)
                    break;
                bits >>= 1;
            }
        pFormat->shifts[2] = (BYTE)rShift;

            return result;
        }

        if (pddpf->dwFlags & DDPF_FOURCC) {
            if (pddpf->dwFourCC == MAKEFOURCC('D', 'X', 'T', '1')) {
                m_texFormatDXT1_16.desc.ddpfPixelFormat = *pddpf;
                m_hasTexFormatDXT1_16 = TRUE;
                m_texFormatDXT1_32.desc.ddpfPixelFormat = *pddpf;
                m_hasTexFormatDXT1_32 = TRUE;
            } else if (pddpf->dwFourCC == MAKEFOURCC('D', 'X', 'T', '5')) {
                m_texFormatDXT5_16.desc.ddpfPixelFormat = *pddpf;
                m_hasTexFormatDXT5_16 = TRUE;
                m_texFormatDXT5_32.desc.ddpfPixelFormat = *pddpf;
                m_hasTexFormatDXT5_32 = TRUE;
                return result;
            }
        }
    }
    return result;
}

// FUNCTION: CMR2 0x004a8fb0
void CGraphics::SelectTextureFormats(void)
{
    memset(&m_texFormat16, 0, sizeof(TextureFormat));
    memset(&m_texFormatDXT1_16, 0, sizeof(TextureFormat));
    memset(&m_texFormat16Alpha, 0, sizeof(TextureFormat));
    memset(&m_texFormatDXT5_16, 0, sizeof(TextureFormat));
    memset(&m_texFormat24, 0, sizeof(TextureFormat));
    memset(&m_texFormatDXT1_32, 0, sizeof(TextureFormat));
    memset(&m_texFormat32, 0, sizeof(TextureFormat));
    memset(&m_texFormatDXT5_32, 0, sizeof(TextureFormat));
    memset(&m_texFormatBump16, 0, sizeof(TextureFormat));
    memset(&m_texFormatBump32, 0, sizeof(TextureFormat));
    m_hasTexFormat16 = FALSE;
    m_hasTexFormatDXT1_16 = FALSE;
    m_hasTexFormat16Alpha = FALSE;
    m_hasTexFormatDXT5_16 = FALSE;
    m_hasTexFormat24 = FALSE;
    m_hasTexFormatDXT1_32 = FALSE;
    m_hasTexFormat32 = FALSE;
    m_hasTexFormatDXT5_32 = FALSE;
    m_hasTexFormatBump16 = FALSE;
    m_hasTexFormatBump32 = FALSE;

    m_pTextureManager->pD3D->EnumTextureFormats(EnumTextureFormatsCallback, NULL);

    if (m_texFormat24.bits[0] != 8) {
        m_hasTexFormat24 = TRUE;
        m_texFormat24 = m_texFormat32;
    }
    if (m_hasTexFormatBump16 && !m_hasTexFormatBump32) {
        m_hasTexFormatBump32 = TRUE;
        m_texFormatBump32 = m_texFormatBump16;
        m_texFormatBump32.desc.ddpfPixelFormat = m_texFormatBump16.desc.ddpfPixelFormat;
    }

    if (g_pGraphics->depth != 16) {
        if (g_pGraphics->depth == 32) {
            m_pTextureManager->textureInfo1 = &m_texFormat24;
            m_pTextureManager->textureInfo2 = &m_texFormat32;
            m_pTextureManager->textureInfo3 = &m_texFormatDXT1_32;
            m_pTextureManager->textureInfo4 = &m_texFormatDXT5_32;
            if (m_hasTexFormatBump32)
                m_pTextureManager->textureInfo5 = &m_texFormatBump32;
        }
    } else {
        m_pTextureManager->textureInfo1 = &m_texFormat16;
        m_pTextureManager->textureInfo2 = &m_texFormat16Alpha;
        m_pTextureManager->textureInfo3 = &m_texFormatDXT1_16;
        m_pTextureManager->textureInfo4 = &m_texFormatDXT5_16;
        if (m_hasTexFormatBump16)
            m_pTextureManager->textureInfo5 = &m_texFormatBump16;
    }
}

// Allocates the shared vertex buffers: 100 small ones plus three larger
// ones, bumping the vertex memory counter after each allocation.
// FUNCTION: CMR2 0x004b1980
void CGraphics::FUN_004b1980(void)
{
    D3DVERTEXBUFFERDESC desc;
    int i;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = 0x10;
    desc.dwCaps = 0x10000;
    desc.dwFVF = 0x2d2;
    if (FUN_004a8d60() != 2)
        desc.dwCaps |= 0x800;
    desc.dwNumVertices = 2000;

    for (i = 0; i < 100; i++) {
        m_pTextureManager->pDD->CreateVertexBuffer(&desc, &m_pTextureManager->pVertexBuffers[i], 0);
        m_pTextureManager->pVertexBuffers[i + 100] = NULL;
        m_unk0x006dd890 += 0x17700;
    }

    desc.dwNumVertices = 0x1800;
    m_pTextureManager->pDD->CreateVertexBuffer(&desc, &m_pTextureManager->pVertexBuffer1, 0);
    m_unk0x006dd890 += 0x48000;

    desc.dwNumVertices = 0x7080;
    m_pTextureManager->pDD->CreateVertexBuffer(&desc, &m_pTextureManager->pVertexBuffer2, 0);
    m_unk0x006dd890 += 0x151800;

    desc.dwNumVertices = 1;
    m_pTextureManager->pDD->CreateVertexBuffer(&desc, &m_pTextureManager->pVertexBuffer3, 0);
    m_unk0x006dd890 += 0x30;
}

// FUNCTION: CMR2 0x0049df90
void CGraphics::FUN_0049df90(BOOL param1, int param2)
{
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x8, 3);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x9, 2);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x17, 4);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x18, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0xf, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x19, 7);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1b, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1c, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1a, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1d, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x2, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x4, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x21, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x29, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x8d, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x91, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x92, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x93, 0);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x94, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x88, 1);
	m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x8a, 0);

	SetZEnable(1);
	SetZWriteEnable(1);
	SetCullMode(CGame::FUN_0049dcb0());

	m_pTextureManager->pD3D->SetTextureStageState(0, (D3DTEXTURESTAGESTATETYPE)0xc, 3);
	m_pTextureManager->pD3D->SetTextureStageState(1, (D3DTEXTURESTAGESTATETYPE)0xc, 3);
	m_pTextureManager->pD3D->SetTextureStageState(2, (D3DTEXTURESTAGESTATETYPE)0xc, 3);
	m_pTextureManager->pD3D->SetTextureStageState(0, (D3DTEXTURESTAGESTATETYPE)0x10, 2);
	m_pTextureManager->pD3D->SetTextureStageState(1, (D3DTEXTURESTAGESTATETYPE)0x10, 2);
	m_pTextureManager->pD3D->SetTextureStageState(2, (D3DTEXTURESTAGESTATETYPE)0x10, 2);

	SetTextureAddressClamp(1);
}

// FUNCTION: CMR2 0x004a8450
BOOL CGraphics::CreateDirect3DDevice(int param1, int param2, int param3)
{
    DDSURFACEDESC2 zBufferDesc;
    DDSURFACEDESC2 displayMode;

    if (m_unk0x00520b7c != 0) {
        m_unk0x0072d56c = 0;
        CGame::RegisterCallback(ReleaseDirect3D, NULL);
        CGame::RegisterCallback(FreeTextureBuffers, NULL);
        CGame::RegisterCallback(FUN_004a5be0, NULL);
    }
    m_unk0x00660bfc = TRUE;

    memset(&m_pTextureManager->ddpfZBuffer, 0, sizeof(DDPIXELFORMAT));
    if (g_pGraphics->depth == 32) {
        m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth = 32;
        m_pTextureManager->pDD->EnumZBufferFormats(m_pTextureManager->deviceGUID, CopyZBufferPixelFormat, &m_pTextureManager->ddpfZBuffer);
        if (m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth != 32) {
            m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth = 24;
            m_pTextureManager->pDD->EnumZBufferFormats(m_pTextureManager->deviceGUID, CopyZBufferPixelFormat, &m_pTextureManager->ddpfZBuffer);
            if (m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth != 24) {
                m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth = 16;
                m_pTextureManager->pDD->EnumZBufferFormats(m_pTextureManager->deviceGUID, CopyZBufferPixelFormat, &m_pTextureManager->ddpfZBuffer);
            }
        }
    }
    if (g_pGraphics->depth == 16) {
        m_pTextureManager->ddpfZBuffer.dwZBufferBitDepth = 16;
        m_pTextureManager->pDD->EnumZBufferFormats(m_pTextureManager->deviceGUID, CopyZBufferPixelFormat, &m_pTextureManager->ddpfZBuffer);
    }

    if (!FUN_004b74b0()) {
        memset(&zBufferDesc, 0, sizeof(zBufferDesc));
        zBufferDesc.dwSize = sizeof(DDSURFACEDESC2);
        zBufferDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        zBufferDesc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
        if (g_pGraphics->field913_0x3bc & 0x20)
            zBufferDesc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
        zBufferDesc.dwHeight = g_pGraphics->resY;
        zBufferDesc.dwWidth = g_pGraphics->resX;
        zBufferDesc.ddpfPixelFormat = m_pTextureManager->ddpfZBuffer;
        if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2)
            zBufferDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
        else
            zBufferDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        g_pGraphics->pDD7->CreateSurface(&zBufferDesc, &g_pGraphics->pSurface3, NULL);
        if (g_pGraphics->pBackBufferSurface->AddAttachedSurface(g_pGraphics->pSurface3) != DD_OK)
            return FALSE;
    }

    displayMode.dwSize = sizeof(DDSURFACEDESC2);
    g_pGraphics->pDD7->GetDisplayMode(&displayMode);
    if (g_pGraphics->isFullscreen == 0 && displayMode.ddpfPixelFormat.dwRGBBitCount == 32) {
        if (DeviceCanRender16Bit(FUN_004a8bc0()) == 0)
            MessageBoxA(CMain::m_hWndList[CMain::m_hWndIx], m_strSetDesktopTo16Bit, CMain::m_logFileBlankLine, MB_TASKMODAL | MB_TOPMOST);
    }

    if (FUN_004a8d60() == 0)
        m_pTextureManager->pDD->CreateDevice(IID_IDirect3DRGBDevice, g_pGraphics->pBackBufferSurface, &m_pTextureManager->pD3D);
    if (FUN_004a8d60() == 1)
        m_pTextureManager->pDD->CreateDevice(IID_IDirect3DHALDevice, g_pGraphics->pBackBufferSurface, &m_pTextureManager->pD3D);
    if (FUN_004a8d60() == 2)
        m_pTextureManager->pDD->CreateDevice(IID_IDirect3DTnLHalDevice, g_pGraphics->pBackBufferSurface, &m_pTextureManager->pD3D);
    if (FUN_004a8d60() == 3)
        m_pTextureManager->pDD->CreateDevice(IID_IDirect3DRefDevice, g_pGraphics->pBackBufferSurface, &m_pTextureManager->pD3D);

    FUN_004b7210();
    SelectTextureFormats();
    FUN_0049df90(m_unk0x00520b7c, 1);
    FUN_004b1980();
    m_unk0x00520b7c = FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x004a8890
void CGraphics::SetProjection(int fovX, int fovY, int farPlane, int nearPlane)
{
    float scale;
    float range;
    D3DMATRIX matrix;
    D3DMATRIX tmp;

    scale = m_projectionScale;
    if (fovX != 0)
        m_fovX = (float)fovX * m_oneOver65536;
    if (fovY != 0)
        m_fovY = (float)fovY * m_oneOver65536;
    if (farPlane != 0)
        m_farPlane = (float)farPlane * m_oneOver65536;
    if (nearPlane != 0)
        m_nearPlane = (float)nearPlane * m_oneOver65536;

    matrix._12 = 0.0f;
    matrix._13 = 0.0f;
    range = m_farPlane - m_nearPlane;
    m_projection33 = m_farPlane / range;
    matrix._14 = 0.0f;
    matrix._21 = 0.0f;
    matrix._23 = 0.0f;
    matrix._24 = 0.0f;
    matrix._31 = 0.0f;
    matrix._32 = 0.0f;
    matrix._34 = 1.0f;
    matrix._41 = 0.0f;
    matrix._42 = 0.0f;
    matrix._44 = 0.0f;
    m_projection43 = -(m_nearPlane * m_farPlane) / range;
    matrix._33 = m_projection33;
    m_projection11 = m_fovX * scale;
    matrix._11 = m_projection11;
    matrix._22 = m_projection22 = scale * m_fovY;
    matrix._43 = m_projection43;
    m_pTextureManager->fixedProjection[0] = (int)(__int64)(m_projection11 * m_65536);
    m_pTextureManager->fixedProjection[1] = 0;
    m_pTextureManager->fixedProjection[2] = 0;
    m_pTextureManager->fixedProjection[3] = 0;
    m_pTextureManager->fixedProjection[4] = 0;
    m_pTextureManager->fixedProjection[5] = (int)(__int64)(matrix._22 * m_65536);
    m_pTextureManager->fixedProjection[6] = 0;
    m_pTextureManager->fixedProjection[7] = 0;
    m_pTextureManager->fixedProjection[8] = 0;
    m_pTextureManager->fixedProjection[8] = 0;
    m_pTextureManager->fixedProjection[10] = (int)(__int64)(matrix._33 * m_65536);
    m_pTextureManager->fixedProjection[11] = 0x10000;
    m_pTextureManager->fixedProjection[12] = 0;
    m_pTextureManager->fixedProjection[12] = 0;
    m_pTextureManager->fixedProjection[14] = (int)(__int64)(matrix._43 * m_65536);
    m_pTextureManager->fixedProjection[15] = 0;

    tmp = matrix;
    m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &tmp);
    m_farPlaneFixed = farPlane;
}

// FUNCTION: CMR2 0x004a5880
void CGraphics::GenerateBumpMap(Texture *pSrc, Texture *pDst)
{
    DDSURFACEDESC2 srcDesc;
    DDSURFACEDESC2 dstDesc;
    unsigned int duMask;
    unsigned int dvMask;
    unsigned int lumMask;
    unsigned int bits;
    int count;
    int duDrop;
    int dvDrop;
    int lumDrop;
    WORD duShift;
    WORD dvShift;
    WORD lumShift;
    int i;
    int x;
    int y;
    int xn;
    int yn;
    int h;
    int h1;
    int h2;
    int du;
    int dv;
    int lum;
    BYTE *pOut;
    WORD *pOut16;

    memset(&srcDesc, 0, sizeof(srcDesc));
    memset(&dstDesc, 0, sizeof(dstDesc));
    srcDesc.dwSize = sizeof(DDSURFACEDESC2);
    dstDesc.dwSize = sizeof(DDSURFACEDESC2);
    pSrc->pSurface->Lock(NULL, &srcDesc, DDLOCK_READONLY | DDLOCK_WAIT, NULL);
    pDst->pSurface->Lock(NULL, &dstDesc, DDLOCK_WRITEONLY | DDLOCK_WAIT, NULL);

    if (dstDesc.ddpfPixelFormat.dwBumpBitCount != 16 && dstDesc.ddpfPixelFormat.dwBumpBitCount != 24 &&
        dstDesc.ddpfPixelFormat.dwBumpBitCount != 32) {
        pSrc->pSurface->Unlock(NULL);
        pDst->pSurface->Unlock(NULL);
        return;
    }

    duMask = dstDesc.ddpfPixelFormat.dwBumpDuBitMask;
    count = 0;
    bits = duMask;
    for (i = 32; i != 0; i--) {
        if (bits & 1)
            count++;
        bits >>= 1;
    }
    duDrop = 8 - (WORD)(BYTE)count;
    dvMask = dstDesc.ddpfPixelFormat.dwBumpDvBitMask;
    count = 0;
    bits = dvMask;
    for (i = 32; i != 0; i--) {
        if (bits & 1)
            count++;
        bits >>= 1;
    }
    dvDrop = 8 - (WORD)(BYTE)count;
    lumMask = dstDesc.ddpfPixelFormat.dwBumpLuminanceBitMask;
    count = 0;
    bits = lumMask;
    for (i = 32; i != 0; i--) {
        if (bits & 1)
            count++;
        bits >>= 1;
    }
    lumDrop = 8 - (WORD)(BYTE)count;

    for (count = 0; count < 32; count++) {
        if (duMask & 1)
            break;
        duMask >>= 1;
    }
    duShift = (BYTE)count;
    for (count = 0; count < 32; count++) {
        if (dvMask & 1)
            break;
        dvMask >>= 1;
    }
    dvShift = (BYTE)count;
    for (count = 0; count < 32; count++) {
        if (lumMask & 1)
            break;
        lumMask >>= 1;
    }
    lumShift = (BYTE)count;

    pOut = (BYTE *)dstDesc.lpSurface;
    pOut16 = (WORD *)dstDesc.lpSurface;
    for (y = 0; y < pDst->height; y++) {
        for (x = 0; x < pDst->width; x++) {
            h = GetPixelRed(&srcDesc, x, y);
            xn = x;
            if (xn < pSrc->width - 1)
                xn++;
            else
                xn = 0;
            h1 = GetPixelRed(&srcDesc, xn, y);
            yn = y;
            if (yn < pSrc->height - 1)
                yn++;
            else
                yn = 0;
            h2 = GetPixelRed(&srcDesc, x, yn);
            du = abs(h - h1);
            dv = abs(h - h2);
            lum = GetPixelAlpha(&srcDesc, x, y);
            if (dstDesc.ddpfPixelFormat.dwBumpBitCount == 16) {
                *pOut16++ = (WORD)((lum >> lumDrop) << lumShift) | (WORD)((dv >> dvDrop) << dvShift) | (WORD)((du >> duDrop) << duShift);
            } else {
                *pOut++ = (BYTE)du;
                *pOut++ = (BYTE)dv;
                *pOut++ = (BYTE)lum;
                if (dstDesc.ddpfPixelFormat.dwBumpBitCount != 24)
                    *pOut++ = 0;
            }
        }
    }

    pSrc->pSurface->Unlock(NULL);
    pDst->pSurface->Unlock(NULL);
}

// FUNCTION: CMR2 0x004a7410
void CGraphics::CreateTextureSurface(Texture *pTexture, int width, int height, unsigned int flags)
{
    DDSURFACEDESC2 desc;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(DDSURFACEDESC2);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.dwWidth = width;
    desc.dwHeight = height;

    if (flags & 0x2) {
        if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2)
            desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
        else
            desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    } else {
        desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        if (FUN_004a8d60() == 1 || FUN_004a8d60() == 2)
            desc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
        else
            desc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }

    if (desc.ddsCaps.dwCaps & DDSCAPS_TEXTURE) {
        if (flags & 0x8)
            desc.ddsCaps.dwCaps2 |= DDSCAPS2_HINTDYNAMIC;
        else
            desc.ddsCaps.dwCaps2 |= DDSCAPS2_OPAQUE;
    }
    if (flags & 0x4)
        desc.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
    if (flags & 0x200) {
        desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX | DDSCAPS_BACKBUFFER;
        desc.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES;
        desc.ddsCaps.dwCaps3 = 0;
        desc.ddsCaps.dwCaps4 = 0;
    }
    if ((flags & 0x10) && (g_pGraphics->field913_0x3bc & 3))
        SetMipMapCount(&desc);

    if ((flags & 0x2000) && m_pTextureManager->textureInfo4 != NULL) {
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo4->desc.ddpfPixelFormat;
    } else if ((flags & 0x4000) && m_pTextureManager->textureInfo3 != NULL) {
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo3->desc.ddpfPixelFormat;
    } else if ((flags & 0x1) && m_pTextureManager->textureInfo2 != NULL) {
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo2->desc.ddpfPixelFormat;
    } else if ((flags & 0x20) && m_hasTexFormatBump16 && m_hasTexFormatBump32) {
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo5->desc.ddpfPixelFormat;
        desc.dwFlags |= 0x40000;
    } else {
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo1->desc.ddpfPixelFormat;
    }
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

    if (width != 2 && width != 4 && width != 8 && width != 16 && width != 32 && width != 64 &&
        width != 128 && width != 256 && width != 512 && width != 1024 && width != 2048)
        _splitpath(pTexture->name, NULL, NULL, CFrontend::m_stringDest, NULL);
    if (height != 2 && height != 4 && height != 8 && height != 16 && height != 32 && height != 64 &&
        height != 128 && height != 256 && height != 512 && height != 1024 && height != 2048)
        _splitpath(pTexture->name, NULL, NULL, CFrontend::m_stringDest, NULL);

    g_pGraphics->pDD7->CreateSurface(&desc, &pTexture->pSurface, NULL);
    pTexture->field_0x11c = 0;
    pTexture->field_0x11e = 0;
    pTexture->width = (short)width;
    pTexture->height = (short)height;
    pTexture->bitsPerPixel = (short)g_pGraphics->depth;
}

// Ported from upstream branch add/FUN_004a8450 (Matt Hadden)
// FUNCTION: CMR2 0x004b74b0
BOOL CGraphics::FUN_004b74b0(void) {
  return m_d3dDeviceDesc7.field0x84;
}

// FUNCTION: CMR2 0x004b7210
void CGraphics::FUN_004b7210(void) {
    D3DDEVICEDESC7 d3ddesc;
    HRESULT hr;

    memset(&m_d3dDeviceDesc7, 0, sizeof(Unk0x006e0bb0));
    hr = m_pTextureManager->pD3D->GetCaps(&d3ddesc);

    if ((d3ddesc.dwDevCaps & 0x100) != 0) {
        m_d3dDeviceDesc7.flag100 = 1;
    }

    if ((d3ddesc.dwDevCaps & 0x200) != 0) {
        m_d3dDeviceDesc7.flag200 = 1;
    }

    if ((d3ddesc.dwDevCaps & 0x1000) != 0) {
        m_d3dDeviceDesc7.flag1000 = 1;
    }

    m_d3dDeviceDesc7.field0x80 = d3ddesc.dwDeviceZBufferBitDepth;
    if ((d3ddesc.dpcTriCaps.dwRasterCaps & 0x8000) != 0) {
        m_d3dDeviceDesc7.field0x84 = 1;
    }

    if (((BYTE)d3ddesc.dpcTriCaps.dwZCmpCaps & 0x80) != 0) {
        m_d3dDeviceDesc7.field0x88 = 1;
    }

    if ((d3ddesc.wMaxSimultaneousTextures > 1) && (d3ddesc.dwTextureOpCaps & 8) != 0) {
        m_d3dDeviceDesc7.field0x8c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureAddressCaps & 4) != 0) {
        m_d3dDeviceDesc7.field0x1c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureFilterCaps & 1) != 0) {
        m_d3dDeviceDesc7.field0x20 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureFilterCaps & 2) != 0) {
        m_d3dDeviceDesc7.field0x2c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureFilterCaps & 0x20) != 0) {
        m_d3dDeviceDesc7.field0x30 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureCaps & 1) != 0) {
        m_d3dDeviceDesc7.field0x24 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureCaps & 0x20) != 0) {
        m_d3dDeviceDesc7.field0x28 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureCaps & 8) != 0) {
        m_d3dDeviceDesc7.field0x34 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 0x80) != 0) {
        m_d3dDeviceDesc7.field0x48 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 0x40) != 0) {
        m_d3dDeviceDesc7.field0x4c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 1) != 0) {
        m_d3dDeviceDesc7.field0x50 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 4) != 0) {
        m_d3dDeviceDesc7.field0x54 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 2) != 0) {
        m_d3dDeviceDesc7.field0x58 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 8) != 0) {
        m_d3dDeviceDesc7.field0x5c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureBlendCaps & 0x20) != 0) {
        m_d3dDeviceDesc7.field0x60 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwAlphaCmpCaps & 0x40) != 0) {
        m_d3dDeviceDesc7.field0x64 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureCaps & 4) != 0) {
        m_d3dDeviceDesc7.field0x68 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwRasterCaps & 0x200) != 0) {
        m_d3dDeviceDesc7.field0x6c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwShadeCaps & 0x1000) != 0) {
        m_d3dDeviceDesc7.field0x70 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwShadeCaps & 0x2000) != 0) {
        m_d3dDeviceDesc7.field0x74 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwShadeCaps & 0x4000) != 0) {
        m_d3dDeviceDesc7.field0x78 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwShadeCaps & 0x8000) != 0) {
        m_d3dDeviceDesc7.field0x7c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwSrcBlendCaps & 0x10) != 0 &&
        (d3ddesc.dpcTriCaps.dwDestBlendCaps & 0x20) != 0) {
        m_d3dDeviceDesc7.field0x38 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwSrcBlendCaps & 2) != 0 &&
        (d3ddesc.dpcTriCaps.dwDestBlendCaps & 2) != 0) {
        m_d3dDeviceDesc7.field0x3c = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwSrcBlendCaps & 4) != 0 &&
        (d3ddesc.dpcTriCaps.dwDestBlendCaps & 2) != 0) {
        m_d3dDeviceDesc7.field0x40 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwSrcBlendCaps & 2) != 0 &&
        (d3ddesc.dpcTriCaps.dwDestBlendCaps & 8) != 0) {
        m_d3dDeviceDesc7.field0x44 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwMiscCaps & 0x40) != 0) {
        m_d3dDeviceDesc7.field0xc = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwMiscCaps & 0x20) != 0) {
        m_d3dDeviceDesc7.field0x8 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwMiscCaps & 0x10) != 0) {
        m_d3dDeviceDesc7.field0x4 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwMiscCaps & 0x80) != 0) {
        m_d3dDeviceDesc7.field0x90 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwRasterCaps & 0x100) != 0) {
        m_d3dDeviceDesc7.field0x94 = 1;
    }

    if (d3ddesc.wMaxTextureBlendStages >= 2) {
        m_d3dDeviceDesc7.field0x98 = 1;
    }

    m_d3dDeviceDesc7.field0xa8 = 1;

    if ((d3ddesc.dwTextureOpCaps & 0x400000) != 0) {
        m_d3dDeviceDesc7.field0x9c = 1;
    }

    if (((d3ddesc.dwStencilCaps & 0x40) != 0) &&
        ((d3ddesc.dwStencilCaps & 0x80) != 0) &&
        ((d3ddesc.dwStencilCaps & 1) != 0) &&
        ((d3ddesc.dwStencilCaps & 4) != 0)) {
        m_d3dDeviceDesc7.field0xa0 = 1;
    }

    if ((d3ddesc.dpcTriCaps.dwTextureCaps & 0x800) != 0) {
        m_d3dDeviceDesc7.field0xa4 = 1;
    }

    m_d3dDeviceDesc7.minTextureWidth = d3ddesc.dwMinTextureWidth;
    m_d3dDeviceDesc7.minTextureHeight = d3ddesc.dwMinTextureHeight;
    m_d3dDeviceDesc7.maxTextureWidth = d3ddesc.dwMaxTextureWidth;
    m_d3dDeviceDesc7.maxTextureHeight = d3ddesc.dwMaxTextureHeight;
}

// FUNCTION: CMR2 0x0049d940
void CGraphics::SetClearColour(int unused, BYTE r, BYTE g, BYTE b)
{
    ((BYTE *)&m_clearColour)[0] = r;
    ((BYTE *)&m_clearColour)[1] = g;
    ((BYTE *)&m_clearColour)[2] = b;
}

// FUNCTION: CMR2 0x0049d960
void CGraphics::ClearTarget(void)
{
    D3DRECT rect;

    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = g_pGraphics->resX;
    rect.y2 = g_pGraphics->resY;
    m_pTextureManager->pD3D->Clear(1, &rect, D3DCLEAR_TARGET,
        RGBA_MAKE(((BYTE *)&m_clearColour)[0], ((BYTE *)&m_clearColour)[1], ((BYTE *)&m_clearColour)[2], 0xff), 1.0f, 0);
}

// FUNCTION: CMR2 0x0049d9d0
BOOL CGraphics::ClearZBuffer(void)
{
    D3DRECT rect;
    DWORD flags;

    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = g_pGraphics->resX;
    rect.y2 = g_pGraphics->resY;
    if (!FUN_004b74b0()) {
        flags = D3DCLEAR_ZBUFFER;
        if (g_pGraphics->field913_0x3bc & 0x20)
            flags = D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;
        m_pTextureManager->pD3D->Clear(1, &rect, flags, 0xff000000, 1.0f, 0);
    }
    return TRUE;
}

// FUNCTION: CMR2 0x0049dc70
void CGraphics::SetCullMode(int mode)
{
    if (mode != m_cullMode) {
        m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CULLMODE, mode);
        m_cullMode = mode;
    }
}

// FUNCTION: CMR2 0x0049dd40
void CGraphics::SetZEnable(int enable)
{
    if (enable != m_zEnable) {
        m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_ZENABLE, enable);
        m_zEnable = enable;
    }
}

// FUNCTION: CMR2 0x0049dd70
void CGraphics::SetTextureAddressClamp(int clamp)
{
    if (clamp != m_textureAddressClamp) {
        if (clamp != 0)
            m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
        else
            m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
        m_textureAddressClamp = clamp;
    }
}

// FUNCTION: CMR2 0x0049ddc0
void CGraphics::SetZWriteEnable(int enable)
{
    if (enable != m_zWriteEnable) {
        m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, enable);
        m_zWriteEnable = enable;
    }
}

// FUNCTION: CMR2 0x0049ddf0
void CGraphics::SetTexCoordIndex(int stage, int index)
{
    m_pTextureManager->pD3D->SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, index);
    m_pTextureManager->pD3D->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
    m_texCoordIndex[stage] = index;
}

// FUNCTION: CMR2 0x004a6e30
Texture *CGraphics::LoadDDSTexture(DDSFile *pDDS, Texture *pTexture)
{
    DDSURFACEDESC2 desc;
    IDirectDrawSurface7 *pTemp;
    Texture tmpTexture;
    BYTE *pSrc;
    BYTE *pDst;
    unsigned int rowBytes;
    unsigned int y;
    int width;
    int height;

    width = pDDS->desc.dwWidth;
    height = pDDS->desc.dwHeight;
    if (width != 2 && width != 4 && width != 8 && width != 16 && width != 32 && width != 64 &&
        width != 128 && width != 256 && width != 512 && width != 1024 && width != 2048)
        _splitpath(pTexture->name, NULL, NULL, CFrontend::m_stringDest, NULL);
    if (height != 2 && height != 4 && height != 8 && height != 16 && height != 32 && height != 64 &&
        height != 128 && height != 256 && height != 512 && height != 1024 && height != 2048)
        _splitpath(pTexture->name, NULL, NULL, CFrontend::m_stringDest, NULL);

    desc = pDDS->desc;
    if (desc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '1'))
        pTexture->flags |= 0x4000;
    else if (desc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '5'))
        pTexture->flags |= 0x2000;
    desc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    g_pGraphics->pDD7->CreateSurface(&desc, &pTemp, NULL);
    pTemp->Lock(NULL, &desc, DDLOCK_WAIT, NULL);
    pSrc = pDDS->data;
    if (!(desc.dwFlags & DDSD_LINEARSIZE)) {
        rowBytes = desc.ddpfPixelFormat.dwRGBBitCount * desc.dwWidth >> 3;
        pDst = (BYTE *)desc.lpSurface;
        for (y = 0; y < desc.dwHeight; y++) {
            memcpy(pDst, pSrc, rowBytes);
            pDst += desc.lPitch;
            pSrc += desc.dwHeight;
        }
    } else {
        memcpy(desc.lpSurface, pSrc, desc.dwLinearSize);
    }
    pTemp->Unlock(NULL);

    if (desc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '1')) {
        if (!m_hasTexFormatDXT1_16 || !m_hasTexFormatDXT1_32)
            pTexture->flags &= ~0x4000;
        else
            pTexture->flags |= 0x4000;
    } else if (desc.ddpfPixelFormat.dwFourCC == MAKEFOURCC('D', 'X', 'T', '5')) {
        if (!m_hasTexFormatDXT5_16 || !m_hasTexFormatDXT5_32)
            pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
        else
            pTexture->flags |= 0x2000;
    }

    if (strncmp(pTexture->name + strlen(pTexture->name) - 6, m_strSuffixBR, 2) == 0)
        pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
    if (strncmp(pTexture->name + strlen(pTexture->name) - 6, m_strSuffixRU, 2) == 0)
        pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
    if (m_unk0x00520b14 == 0) {
        if (strncmp(pTexture->name + strlen(pTexture->name) - 8, m_strSuffixBODF, 4) == 0)
            pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
    }
    if (strncmp(pTexture->name + strlen(pTexture->name) - 9, m_strSuffixDIGIT, 5) == 0)
        pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
    if (strncmp(pTexture->name + strlen(pTexture->name) - 9, m_strSuffixREVCT, 5) == 0)
        pTexture->flags = (pTexture->flags & ~0x6000) | 0x1;
    if (g_pGraphics->field913_0x3bc & 0x10) {
        if (strncmp(pTexture->name + strlen(pTexture->name) - 6, m_strSuffixBU, 2) == 0)
            pTexture->flags = (pTexture->flags & ~0x6001) | 0x20;
    }

    if (!(pTexture->flags & 0x20) || !(g_pGraphics->field913_0x3bc & 0x10)) {
        CreateTextureSurface(pTexture, pDDS->desc.dwWidth, pDDS->desc.dwHeight, pTexture->flags);
        pTexture->pSurface->Blt(NULL, pTemp, NULL, DDBLT_WAIT, NULL);
        if (pTemp != NULL && pTemp->Release() == 0)
            pTemp = NULL;
    } else {
        memset(&desc, 0, sizeof(desc));
        desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
        desc.ddpfPixelFormat = m_pTextureManager->textureInfo2->desc.ddpfPixelFormat;
        desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        g_pGraphics->pDD7->CreateSurface(&desc, &tmpTexture.pSurface, NULL);
        tmpTexture.pSurface->Blt(NULL, pTemp, NULL, DDBLT_WAIT, NULL);
        CreateTextureSurface(pTexture, pDDS->desc.dwWidth, pDDS->desc.dwHeight, pTexture->flags);
        GenerateBumpMap(&tmpTexture, pTexture);
        if (tmpTexture.pSurface != NULL && tmpTexture.pSurface->Release() == 0)
            tmpTexture.pSurface = NULL;
        if (pTemp != NULL && pTemp->Release() == 0)
            pTemp = NULL;
    }

    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        GetMipMapSurfaces(pTexture);
    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        m_unk0x0065fa2c += GetMipMapPixelCount(pTexture);
    m_unk0x0065fa2c += pDDS->desc.dwHeight * pDDS->desc.dwWidth;
    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        BltMipMaps(pTexture);
    if (!(pTexture->flags & 0x1000))
        CFileBuffer::FreeGenericFileBuffer(pDDS);
    return pTexture;
}

// GLOBAL: CMR2 0x008164c8
int g_unk0x008164c8;
// GLOBAL: CMR2 0x00816298
BYTE g_unk0x00816298[0x230];

// FUNCTION: CMR2 0x004bca70
void FUN_004bca70(short *param1)
{
    if (g_unk0x008164c8 != 0)
        return;
    CGame::RegisterCallback((void *)0x4bcac0, NULL);
    if (param1[2] != 0 && param1[3] != 0) {
        CGraphics::CreateTextureSurface((Texture *)g_unk0x00816298, param1[2], param1[3], 8);
        g_unk0x008164c8 = 1;
    }
}

// GLOBAL: CMR2 0x006db200
unsigned short g_unk0x006db200[800 * 6];
// GLOBAL: CMR2 0x006dd784
int g_unk0x006dd784;
// GLOBAL: CMR2 0x006dd788
int g_unk0x006dd788;
// GLOBAL: CMR2 0x004b1500
BYTE g_unk0x004b1500[1];

// Builds the 800-entry triangle-strip index table.
// TODO: CMR2 0x004b1150 (implemented, match 45%)
void FUN_004b1150(void)
{
    unsigned short *pIndex;
    int i;

    g_unk0x006dd784 = 0;
    g_unk0x006dd788 = 0;
    pIndex = g_unk0x006db200;
    for (i = 0; i < 800; i++) {
        int v = i * 4 + 3;

        pIndex[0] = v - 3;
        pIndex[1] = v;
        pIndex[2] = v - 1;
        pIndex[3] = v - 3;
        pIndex[4] = v - 2;
        pIndex[5] = v;
        pIndex += 6;
    }
    CGame::RegisterCallback(g_unk0x004b1500, NULL);
}

// Takes a free texture slot, copies the 0x130-byte texture into it and
// returns the slot index (or -1 when the table is full).
// FUNCTION: CMR2 0x004a4bd0
int FUN_004a4bd0(void *pSource, int param2)
{
    int i;

    for (i = 0; i < 0x800; i++) {
        if (CGraphics::m_pTextureManager->textureBuffer[i] == NULL) {
            CGraphics::m_pTextureManager->textureBuffer[i] =
                (Texture *)CFileBuffer::AllocateLockedBuffer(0x130);
            memcpy(CGraphics::m_pTextureManager->textureBuffer[i], pSource, 0x130);
            CGraphics::m_textureCount++;
            return i;
        }
    }
    return -1;
}

// GLOBAL: CMR2 0x006a2a98
void *g_unk0x006a2a98;
// GLOBAL: CMR2 0x006a2bcc
int g_unk0x006a2bcc;
// GLOBAL: CMR2 0x006a2a20
int g_unk0x006a2a20;
// GLOBAL: CMR2 0x006a2a24
int g_unk0x006a2a24;
// GLOBAL: CMR2 0x006a2a38
int g_unk0x006a2a38;
// GLOBAL: CMR2 0x006a2a3c
int g_unk0x006a2a3c;
// GLOBAL: CMR2 0x006a2a50
int g_unk0x006a2a50;
// GLOBAL: CMR2 0x006a2a54
int g_unk0x006a2a54;
// GLOBAL: CMR2 0x006a2a68
int g_unk0x006a2a68;
// GLOBAL: CMR2 0x006a2a6c
int g_unk0x006a2a6c;
// GLOBAL: CMR2 0x004ae200
BYTE g_unk0x004ae200[1];

// TODO: CMR2 0x004ae170 (implemented, match 73%)
void FUN_004ae170(int param1)
{
    if (g_unk0x006a2a98 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x006a2a98);
        g_unk0x006a2a98 = NULL;
    }
    g_unk0x006a2a98 = CFileBuffer::AllocateLockedBuffer((param1 & 0xff) * 92);
    g_unk0x006a2bcc = param1 & 0xff;
    g_unk0x006a2a20 = 0;
    g_unk0x006a2a24 = 0;
    g_unk0x006a2a38 = 0xfff9;
    g_unk0x006a2a3c = 0;
    g_unk0x006a2a50 = 0xfff9;
    g_unk0x006a2a54 = 0xfff9;
    g_unk0x006a2a68 = 0;
    g_unk0x006a2a6c = 0xfff9;
    CGame::RegisterCallback(g_unk0x004ae200, NULL);
}

// GLOBAL: CMR2 0x004ae120
BYTE g_unk0x004ae120[1];

// Creates the 0x1000-vertex write-only buffer once and registers its callback.
// FUNCTION: CMR2 0x004ae0a0
void FUN_004ae0a0(void)
{
    D3DVERTEXBUFFERDESC desc = {0};

    desc.dwSize = 0x10;
    desc.dwCaps = 0x10000;
    desc.dwFVF = 0x2d2;
    if (CGraphics::FUN_004a8d60() != 2)
        desc.dwCaps |= 0x800;
    desc.dwNumVertices = 0x1000;
    CGraphics::m_pTextureManager->pDD->CreateVertexBuffer(
        &desc, (LPDIRECT3DVERTEXBUFFER7 *)0x6a2a08, 0);
    CGame::RegisterCallback(g_unk0x004ae120, NULL);
}
