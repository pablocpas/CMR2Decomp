#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Graphics.h"
#include "Car.h"
#include "Sprite.h"
#include "../third_party/dx7sdk-7001/include/d3dxmath.h"
#pragma comment(lib, "third_party/dx7sdk-7001/lib/d3dx.lib")
#include "Frontend.h"
#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include "GameInfo.h"
#include "InstallInfo.h"
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

// Not declared in Mesh.h yet; the other two are not analysed yet.
void Mesh_ReuploadAll(void);
void Scene_RestoreLights(void);
void FUN_004a2ba0(void);

// GLOBAL: CMR2 0x00660830
Graphics g_graphics;

// GLOBAL: CMR2 0x00520b74
Graphics *g_pGraphics = &g_graphics;
// The Direct3D device state (vertex buffers, textures, matrices).
// GLOBAL: CMR2 0x00660d90
D3DTextureManager g_textureManager;
D3DTextureManager *CGraphics::m_pTextureManager = &g_textureManager;

char CGraphics::m_strSettingConfigurationToDefault[36] = "Setting configuration to defaults";

BOOL CGraphics::m_unk0x00520b7c = TRUE;
unsigned int CGraphics::m_unk0x0065fa2c;
unsigned int CGraphics::m_textureCount;
unsigned int CGraphics::m_lockedTextureCount;
LockedTexture CGraphics::m_lockedTextures[5];
Texture CGraphics::m_textureCache[64];
unsigned int CGraphics::m_unk0x00520b2c = 0xff;
unsigned int CGraphics::m_unk0x00520b30 = 0xff;
int CGraphics::m_unk0x0065fa44;
int CGraphics::m_unk0x0065fa48;
float CGraphics::m_oneOver128 = 1.0f / 128.0f;
float CGraphics::m_unk0x00520b34 = 1.0f;
float CGraphics::m_unk0x00520b38 = 1.0f;
BYTE CGraphics::m_tgaPixel[4];
float CGraphics::m_bumpScale = 1.0f;
char CGraphics::m_ddsExtension[8] = ".DDS";
char CGraphics::m_tgaExtension[8] = ".TGA";
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

    CGameInfo::m_gameInfo.unknownGraphicsOptions =
        (CGameInfo::m_gameInfo.unknownGraphicsOptions & 0xfe3fffff) | 0x200000;

    g_pGraphics->field917_0x3c0 = 1;

    CGameInfo::m_gameInfo.unknownGraphicsOptions =
        (CGameInfo::m_gameInfo.unknownGraphicsOptions & 0xebffffff) | 0xa000000;
    CGameInfo::m_gameInfo.field_0x34 =  (CGameInfo::m_gameInfo.field_0x34 & 0xfffffffe) | 2;
    CGameInfo::m_gameInfo.unknownGraphicsOptions &= 0xdfffffff;

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

    if (FUN_004a7910(screenWidth, screenHeight, colourDepth) != 0) {
        if (CreateDirect3DDevice(screenWidth, screenHeight, colourDepth) != 0) {
            Mesh_ReuploadAll();
            Scene_RestoreLights();
            FUN_004a2ba0();
        }
    }
}

// FUNCTION: CMR2 0x004a5be0
BOOL CGraphics::FUN_004a5be0(void) {
    int index, textureID, face;

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

    // Cube maps: for each one, its 6 z-buffers and its 6 face surfaces, last first
    // (the original walks both arrays downwards from face 5)
    for (index = 0; index < m_unk0x0065fa28; index++) {
        for (face = 5; face >= 0; face--) {
            RenderTexture *pCube = (RenderTexture *)m_pTextureManager->textureBuffer2[index];
            IDirectDrawSurface7 **ppFace;
            if (pCube->pZBuffers[face] != NULL && pCube->pZBuffers[face]->Release() == 0)
                ((RenderTexture *)m_pTextureManager->textureBuffer2[index])->pZBuffers[face] = NULL;
            pCube = (RenderTexture *)m_pTextureManager->textureBuffer2[index];
            ppFace = (IDirectDrawSurface7 **)((BYTE *)pCube + 0x114 + face * sizeof(RenderTextureFace));
            if (*ppFace != NULL && (*ppFace)->Release() == 0)
                *(IDirectDrawSurface7 **)((BYTE *)m_pTextureManager->textureBuffer2[index] + 0x114 + face * sizeof(RenderTextureFace)) = NULL;
        }
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

// Releases the 64 cached surfaces held by m_textureCache.
// FUNCTION: CMR2 0x004a5ba0
void CGraphics::FUN_004a5ba0(void)
{
    unsigned int i;

    for (i = 0; i < 64; i++) {
        if (m_textureCache[i].pSurface != NULL) {
            if (m_textureCache[i].pSurface->Release() == 0)
                m_textureCache[i].pSurface = NULL;
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

    // The target is the wrapper's pDD field (offset 0), not the pointer itself:
    // QueryInterface stores the IDirect3D7 there and m_pTextureManager keeps
    // pointing at g_textureManager.
    g_pGraphics->pDD7->QueryInterface(IID_IDirect3D7, (LPVOID*)&m_pTextureManager->pDD);
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
        if (width == m_displays[i].width && height == m_displays[i].height && colourDepth == m_displays[i].colourDepth)
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

// Replaces three alpha values of a texture (4444 or 8888 formats), e.g. to
// recolour masked areas. The untouched original is first saved in cache slot
// cacheSlot so BltTexture can restore it.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a4d30
void CGraphics::RemapTextureAlpha(Texture *pTexture, WORD from0, WORD to0, WORD from1, WORD to1, WORD from2, WORD to2,
                                  int cacheSlot)
{
    DDSURFACEDESC2 desc;
    RECT rect;
    unsigned int width;
    unsigned int height;
    unsigned int x;
    unsigned int y;
    unsigned int mask;
    BYTE bits;
    int i;
    DWORD *p32;
    WORD *p16;
    int skip;

    desc.dwSize = sizeof(DDSURFACEDESC2);
    rect.left = 0;
    rect.right = pTexture->width;
    rect.top = 0;
    rect.bottom = pTexture->height;
    if (m_textureCache[cacheSlot].pSurface == NULL) {
        CreateTextureSurface(&m_textureCache[cacheSlot], pTexture->width, pTexture->height, 9);
        m_textureCache[cacheSlot].pSurface->Blt(&rect, pTexture->pSurface, NULL, DDBLT_WAIT, NULL);
        g_unk0x0065fa30++;
    }
    pTexture->pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);
    width = pTexture->width;
    height = pTexture->height;
    bits = 0;
    mask = desc.ddpfPixelFormat.dwRGBAlphaBitMask;
    for (i = 32; i != 0; i--) {
        if (mask & 1)
            bits++;
        mask >>= 1;
    }
    switch (bits) {
    case 4:
        p16 = (WORD *)desc.lpSurface;
        skip = desc.lPitch - width * 2;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                if ((WORD)((*p16 >> 8) & 0xf0) == (WORD)(((int)from0 << 8 >> 8) & 0xf0))
                    *p16 = ((to0 & 0xf0) << 8) | (*p16 & 0xfff);
                if ((WORD)((*p16 >> 8) & 0xf0) == (WORD)(((int)from1 << 8 >> 8) & 0xf0))
                    *p16 = ((to1 & 0xf0) << 8) | (*p16 & 0xfff);
                if ((WORD)((*p16 >> 8) & 0xf0) == (WORD)(((int)from2 << 8 >> 8) & 0xf0))
                    *p16 = ((to2 & 0xf0) << 8) | (*p16 & 0xfff);
                p16++;
            }
            p16 += skip;
        }
        break;
    case 8:
        p32 = (DWORD *)desc.lpSurface;
        skip = (unsigned int)(desc.lPitch - width * 4) >> 2;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                if ((*p32 >> 24) == from0)
                    *p32 = (*p32 & 0xffffff) | ((DWORD)to0 << 24);
                if ((*p32 >> 24) == from1)
                    *p32 = (*p32 & 0xffffff) | ((DWORD)to1 << 24);
                if ((*p32 >> 24) == from2)
                    *p32 = (*p32 & 0xffffff) | ((DWORD)to2 << 24);
                p32++;
            }
            p32 += skip;
        }
        break;
    }
    pTexture->pSurface->Unlock(NULL);
}

// FUNCTION: CMR2 0x004a5080
void CGraphics::BltTexture(Texture *pTexture, int surfaceIndex)
{
    RECT rect;

    rect.left = 0;
    rect.right = pTexture->width;
    rect.top = 0;
    rect.bottom = pTexture->height;
    if (pTexture->pSurface != NULL && m_textureCache[surfaceIndex].pSurface != NULL)
        pTexture->pSurface->Blt(&rect, m_textureCache[surfaceIndex].pSurface, NULL, DDBLT_WAIT, NULL);
}

// Moves an 8-bit channel value into a 16-bit pixel channel whose top bit is
// `depth` bits above bit 7 (negative: below).
#define PACK_CHANNEL(mask, depth, v)                                                \
    ((depth) < 0 ? (((mask) << abs(depth)) & (v)) >> abs(depth)                     \
                 : (((mask) >> abs(depth)) & (v)) << abs(depth))

// Blends pColour (r, g, b, a) into pixel (x, y) of a locked texture.
// match 24%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a52d0
void CGraphics::BlendPixel(Texture *pTexture, unsigned int x, unsigned int y, BYTE *pColour)
{
    LockedTexture *pLocked;
    unsigned int i;
    int stride;
    int r, g, b;
    float f, inv;
    BYTE px;
    BYTE dst[3];
    WORD *p16;
    DWORD *p32;

    r = pColour[0];
    g = pColour[1];
    b = pColour[2];
    if (x >= (unsigned int)pTexture->width || y >= (unsigned int)pTexture->height)
        return;
    for (i = 0; i < m_lockedTextureCount; i++) {
        if (pTexture == m_lockedTextures[i].pTexture)
            break;
    }
    if (i == m_lockedTextureCount)
        return;
    pLocked = &m_lockedTextures[i];
    stride = pLocked->desc.lPitch - ((pLocked->desc.dwWidth * pLocked->desc.ddpfPixelFormat.dwRGBBitCount) >> 3);
    if (pLocked->desc.ddpfPixelFormat.dwRGBBitCount == 16) {
        p16 = (WORD *)pLocked->desc.lpSurface + ((pLocked->desc.dwWidth + stride) * y + x);
        if (pColour[3] != 0xff) {
            px = *(BYTE *)p16;
            dst[0] = (BYTE)((BYTE)pLocked->masks[0] & px) >> (BYTE)pLocked->depths[0];
            dst[1] = (BYTE)((BYTE)pLocked->masks[1] & px) << (BYTE)pLocked->depths[1];
            dst[2] = (BYTE)((BYTE)pLocked->masks[2] & px) << (BYTE)-pLocked->depths[2];
            f = (float)pColour[3] * (1.0f / 255.0f);
            inv = 1.0f - f;
            r = (int)(__int64)((float)r * f + (float)dst[0] * inv);
            g = (int)(__int64)((float)g * f + (float)dst[1] * inv);
            b = (int)(__int64)((float)b * f + (float)dst[2] * inv);
        }
        *p16 = (WORD)(PACK_CHANNEL(pLocked->masks[3], (short)pLocked->depths[3], 0xff) |
                      PACK_CHANNEL(pLocked->masks[0], (short)pLocked->depths[0], r) |
                      PACK_CHANNEL(pLocked->masks[1], (short)pLocked->depths[1], g) |
                      PACK_CHANNEL(pLocked->masks[2], (short)pLocked->depths[2], b));
    } else if (pLocked->desc.ddpfPixelFormat.dwRGBBitCount == 32) {
        p32 = (DWORD *)pLocked->desc.lpSurface + ((pLocked->desc.dwWidth + stride) * y + x);
        if (pColour[3] != 0xff) {
            dst[0] = 0;
            dst[1] = 0;
            dst[2] = *(BYTE *)p32;
            f = (float)pColour[3] * (1.0f / 255.0f);
            inv = 1.0f - f;
            r = (int)(__int64)((float)r * f + (float)dst[0] * inv);
            g = (int)(__int64)((float)g * f + (float)dst[1] * inv);
            b = (int)(__int64)((float)b * f + (float)dst[2] * inv);
        }
        *p32 = RGBA_MAKE(r, g, b, 0xff);
    }
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
    {
        DWORD maxCount = 3;
        if (pDesc->dwMipMapCount > maxCount)
            pDesc->dwMipMapCount = maxCount;
    }
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
        m_pTextureManager->vertexBufferFill[i] = 0;
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

// FUNCTION: CMR2 0x004b74c0
DWORD FUN_004b74c0(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x90;
}

// FUNCTION: CMR2 0x004b74d0
DWORD FUN_004b74d0(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x94;
}

// FUNCTION: CMR2 0x004b74e0
DWORD FUN_004b74e0(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x3c;
}

// FUNCTION: CMR2 0x004b74f0
DWORD FUN_004b74f0(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x40;
}

// FUNCTION: CMR2 0x004b7500
DWORD FUN_004b7500(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x44;
}

// FUNCTION: CMR2 0x004b7510
DWORD FUN_004b7510(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x64;
}

// FUNCTION: CMR2 0x004b7530
DWORD FUN_004b7530(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x98;
}

// FUNCTION: CMR2 0x004b7540
DWORD FUN_004b7540(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0x9c;
}

// FUNCTION: CMR2 0x004b7550
DWORD FUN_004b7550(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0xa4;
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
void CGraphics::SetClearColour(int unused, int r, int g, int b)
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

// Reloads every texture from its archive (.DDS first, else .TGA) and
// recreates the cube map surfaces, e.g. after the device was lost.
// match 89%: MSVC puts the LoadDDS branch inline here; the original defers it past
// the function epilogue (same CFG, different block order). No source shape tried reproduces it.
// FUNCTION: CMR2 0x004a4c40
void Graphics_ReloadAllTextures(void)
{
    Texture *pTexture;
    char *pExt;
    void *pData;
    unsigned int i;

    for (i = 0; i < CGraphics::m_textureCount; i++) {
        pTexture = CGraphics::m_pTextureManager->textureBuffer[i];
        if (pTexture->textureId == i) {
            pExt = &pTexture->name[strlen(pTexture->name) - 4];
            strncpy(pExt, CGraphics::m_ddsExtension, 4);
            pData = CGenericFileLoader::FindFile((GenericFile *)pTexture->pArchive, pTexture->name, 0, 0, 0);
            if (pData == NULL) {
                strncpy(pExt, CGraphics::m_tgaExtension, 4);
                pData = CGenericFileLoader::FindFile((GenericFile *)pTexture->pArchive, pTexture->name, 0, 0, 0);
                CGraphics::LoadTGATexture((BYTE *)pData, pTexture);
            } else {
                CGraphics::LoadDDSTexture((DDSFile *)pData, pTexture);
            }
        }
    }
    for (i = 0; i < CGraphics::m_unk0x0065fa28; i++)
        CGraphics::CreateCubeMapSurfaces((RenderTexture *)CGraphics::m_pTextureManager->textureBuffer2[i]);
}

// Whether a 2D point lies inside a triangle (x0, y0, x1, y1, x2, y2), all
// 16.16; the edge tests work on values scaled by 0.01 to avoid overflow.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049da50
int Tri2D_Contains(int *pPoint, int *pTri)
{
    int ex0, ey0, ex1, ey1, ex2, ey2;
    int px0, py0, px1, py1, px2, py2;

    if ((pTri[0] > pPoint[0] && pTri[2] > pPoint[0] && pTri[4] > pPoint[0]) ||
        (pPoint[0] > pTri[0] && pPoint[0] > pTri[2] && pPoint[0] > pTri[4]))
        return 0;
    if ((pTri[1] > pPoint[1] && pTri[3] > pPoint[1] && pTri[5] > pPoint[1]) ||
        (pPoint[1] > pTri[1] && pPoint[1] > pTri[3] && pPoint[1] > pTri[5]))
        return 0;
    ey0 = FixMul(pTri[3] - pTri[1], 0x28f);
    ex0 = FixMul(pTri[2] - pTri[0], 0x28f);
    ey1 = FixMul(pTri[5] - pTri[3], 0x28f);
    ex1 = FixMul(pTri[4] - pTri[2], 0x28f);
    ey2 = FixMul(pTri[1] - pTri[5], 0x28f);
    ex2 = FixMul(pTri[0] - pTri[4], 0x28f);
    px0 = FixMul(pPoint[0] - pTri[0], 0x28f);
    py0 = FixMul(pPoint[1] - pTri[1], 0x28f);
    px1 = FixMul(pPoint[0] - pTri[2], 0x28f);
    py1 = FixMul(pPoint[1] - pTri[3], 0x28f);
    px2 = FixMul(pPoint[0] - pTri[4], 0x28f);
    py2 = FixMul(pPoint[1] - pTri[5], 0x28f);
    if (FixMul(px0, ey0) + FixMul(py0, -ex0) >= 0 && FixMul(py1, -ex1) + FixMul(px1, ey1) >= 0 &&
        FixMul(py2, -ex2) + FixMul(px2, ey2) >= 0)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x0049dc70
void CGraphics::SetCullMode(int mode)
{
    if (mode != m_cullMode) {
        m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_CULLMODE, mode);
        m_cullMode = mode;
    }
}

// GLOBAL: CMR2 0x0059ce30
int g_unk0x0059ce30;

// Switches alpha blending; with alpha test support the reference value follows.
// FUNCTION: CMR2 0x0049dcc0
void FUN_0049dcc0(int enable)
{
    if (enable != g_unk0x0059ce30) {
        CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1b, enable);
        if (FUN_004b7510()) {
            if (enable != 0) {
                CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x18, 1);
            } else {
                CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x18, 0x80);
            }
            g_unk0x0059ce30 = enable;
            return;
        }
        CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1b, 1);
        g_unk0x0059ce30 = enable;
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

int Args_Has(char *pArg);

extern int g_sceneStatCopied;
extern int g_sceneStatMultiplied;
extern int g_sceneStatClean;
extern int g_sceneStatHidden;
extern int g_fixMatrixMultiplyCount;

// Frame rate counter argument ("gamegauge"): when present the vsync wait is
// skipped.
// GLOBAL: CMR2 0x005207fc
char g_str0x005207fc[] = "gamegauge";

// Ends the frame: resets the per frame counters and presents the back buffer,
// which in windowed mode means blitting it into the client area of the game
// window and in fullscreen mode flipping (waiting for the vertical blank only
// when the frame rate counter is off).
// FUNCTION: CMR2 0x0049de40
void FUN_0049de40(void)
{
    POINT pt;
    POINT corners[2];
    RECT rect;

    CMain::UpdateFrameTime();
    g_sceneStatCopied = 0;
    g_sceneStatMultiplied = 0;
    g_sceneStatClean = 0;
    g_sceneStatHidden = 0;
    g_fixMatrixMultiplyCount = 0;
    CGraphics::m_unk0x0065fa24 = 0;
    if (g_pGraphics->isFullscreen == 0) {
        GetClientRect(CMain::m_hWndList[CMain::m_hWndIx], &rect);
        pt.x = rect.left;
        pt.y = rect.top;
        ClientToScreen(CMain::m_hWndList[CMain::m_hWndIx], &pt);
        GetClientRect(CMain::m_hWndList[CMain::m_hWndIx], (LPRECT)corners);
        ClientToScreen(CMain::m_hWndList[CMain::m_hWndIx], corners);
        ClientToScreen(CMain::m_hWndList[CMain::m_hWndIx], corners + 1);
        if (Args_Has(g_str0x005207fc) == 0)
            g_pGraphics->pDD7->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
        g_pGraphics->pPrimarySurface->Blt((LPRECT)corners, g_pGraphics->pBackBufferSurface,
                                          NULL, DDBLT_WAIT, NULL);
        return;
    }
    if (Args_Has(g_str0x005207fc) != 0) {
        g_pGraphics->pPrimarySurface->Flip(NULL, DDFLIP_NOVSYNC);
        return;
    }
    g_pGraphics->pPrimarySurface->Flip(NULL, DDFLIP_WAIT);
}

// Drawing view of a sector's static stage objects (StageObject in Sector.h):
// the same layout as the copy Game.cpp keeps locally, for the same reason
// (touching Sector.h perturbs the codegen of unrelated files).
struct StageObjectDraw {
    FixVector position;         // 0x0  world position (16.16)
    Mesh *pMesh;                // 0xc
    int field_0x10;
    BYTE field_0x14;            // 0x14 non-zero while the object is drawn
    BYTE field_0x15[3];
    int scaleX;                 // 0x18 scale of the first world matrix row (16.16)
    int field_0x1c[4];
    int scaleY;                 // 0x2c scale of the second world matrix row (16.16)
    int field_0x30[4];
    int scaleZ;                 // 0x40 scale of the third world matrix row (16.16)
    int field_0x44;
    int offsetX;                // 0x48 world matrix translation (16.16)
    int offsetY;                // 0x4c
    int offsetZ;                // 0x50
    int field_0x54;
    float matrix[16];           // 0x58 world matrix, built from the camera matrix
    StageObjectDraw *pNext;     // 0x98 next object of the sector
    int lightLevel;             // 0x9c
};

extern D3DMATRIX g_unk0x00597cc0;
extern D3DMATRIX g_unk0x005207b8;
extern unsigned short g_unk0x006ed5f0[];
D3DMATRIX *FixMatrix_ToFloat(D3DMATRIX *pOut, FixMatrix *pIn);
void FUN_004b2970(int value);
void FUN_0049d290(int param1);
void Game_DrawSortedNodes(int bit);

// Scratch view matrix rebuilt for every cube map face (0x0049d3f0 uses it too).
// GLOBAL: CMR2 0x0059bd28
FixMatrix g_unk0x0059bd28;

// Renders the scene into the six faces of the cube map the mesh of pNode
// belongs to: for each face it derives a view matrix from the node's transform,
// points the render target at that face and draws the culled sectors (ground
// meshes, static objects and view-mask nodes). Restores the transforms and the
// back buffer afterwards.
// FUNCTION: CMR2 0x0049e1f0
int FUN_0049e1f0(SceneNode *pNode, int bit)
{
    D3DRECT rect;
    D3DVIEWPORT7 viewport;
    int farPlane;
    int cubeIndex;
    D3DMATRIX view;
    D3DMATRIX projection;
    FixMatrix face;
    FixMatrix inverse;
    D3DMATRIX floatMatrix;
    FixVector forward;
    FixVector tmp;
    FixMatrix *pCurrent;
    Mesh *pNodeMesh;
    Mesh *pMesh;
    StageObjectDraw *pObject;
    Sector *pSector;
    SceneNode *pChild;
    unsigned int i;
    unsigned int sectorIndex;

    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = CGraphics::m_cubeMapSize;
    rect.y2 = CGraphics::m_cubeMapSize;
    farPlane = CGraphics::m_farPlaneFixed;
    pNodeMesh = (Mesh *)pNode->pObject;
    cubeIndex = (pNodeMesh->flags >> 15) & 7;
    if (CGraphics::m_pTextureManager->textureBuffer2[cubeIndex] == NULL)
        return 1;

    FUN_004b2970(1);
    FUN_0049dcc0(0);

    memset(&viewport, 0, sizeof(viewport));
    viewport.dwX = 0;
    viewport.dwY = 0;
    viewport.dwWidth = CGraphics::m_cubeMapSize;
    viewport.dwHeight = CGraphics::m_cubeMapSize;
    viewport.dvMinZ = 0.0f;
    viewport.dvMaxZ = 1.0f;
    CGraphics::m_pTextureManager->pD3D->SetViewport(&viewport);
    CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_VIEW, &view);
    CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &projection);

    pCurrent = &pNode->current;
    for (i = 0; i < 6; i++) {
        switch (i) {
        case 0:
            face = *pCurrent;
            tmp = face.right;
            face.right.x = -face.forward.x;
            face.right.y = -face.forward.y;
            face.right.z = -face.forward.z;
            face.forward = tmp;
            break;
        case 1:
            face = *pCurrent;
            tmp = face.right;
            face.right = face.forward;
            face.forward.x = -tmp.x;
            face.forward.y = -tmp.y;
            face.forward.z = -tmp.z;
            break;
        case 2:
            face = *pCurrent;
            tmp = face.forward;
            face.forward = face.up;
            face.up.x = -tmp.x;
            face.up.y = -tmp.y;
            face.up.z = -tmp.z;
            break;
        case 3:
            face = *pCurrent;
            tmp = face.forward;
            face.forward.x = -face.up.x;
            face.forward.y = -face.up.y;
            face.forward.z = -face.up.z;
            face.up = tmp;
            break;
        case 4:
            face = *pCurrent;
            break;
        case 5:
            face = *pCurrent;
            face.right.x = -face.right.x;
            face.right.y = -face.right.y;
            face.right.z = -face.right.z;
            face.forward.x = -face.forward.x;
            face.forward.y = -face.forward.y;
            face.forward.z = -face.forward.z;
            break;
        }
        FixMatrix_Invert(&inverse, &face);
        FixMatrix_ToFloat(&floatMatrix, &inverse);
        CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_VIEW, &floatMatrix);
        forward.x = face.forward.x;
        forward.y = 0;
        forward.z = face.forward.z;
        FIX_NORMALIZE_INTO(forward, forward);
        g_unk0x0059bd28.right.x = forward.z;
        g_unk0x0059bd28.right.y = 0;
        g_unk0x0059bd28.right.z = -forward.x;
        g_unk0x0059bd28.rw = 0;
        g_unk0x0059bd28.up.x = 0;
        g_unk0x0059bd28.up.y = 0x10000;
        g_unk0x0059bd28.up.z = 0;
        g_unk0x0059bd28.uw = 0;
        g_unk0x0059bd28.forward.x = forward.x;
        g_unk0x0059bd28.forward.y = 0;
        g_unk0x0059bd28.forward.z = forward.z;
        g_unk0x0059bd28.fw = 0;
        g_unk0x0059bd28.position.x = 0;
        g_unk0x0059bd28.position.y = 0;
        g_unk0x0059bd28.position.z = 0;
        g_unk0x0059bd28.pw = 0x10000;
        FixMatrix_ToFloat(&g_unk0x00597cc0, &g_unk0x0059bd28);
        Graphics_SetRenderTarget(&CGraphics::m_pTextureManager->textureBuffer2[cubeIndex][i]);
        CGraphics::m_pTextureManager->pD3D->Clear(1, &rect, D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);
        CGraphics::m_pTextureManager->pD3D->BeginScene();
        CGraphics::SetProjection(0x20000, 0x20000, 0x780000, 0x1999);
        CGraphics::SetZEnable(0);
        CGraphics::SetZWriteEnable(0);
        for (sectorIndex = 0; sectorIndex < (unsigned int)g_sectorCullEnabled; sectorIndex++) {
            pChild = g_sectors[(unsigned short)g_unk0x006ed5f0[sectorIndex]]->pFirstNode;
            while (pChild != NULL) {
                if (pChild->visible == 0)
                    Game_DrawViewMaskNode(pChild, 0);
                pChild = pChild->pNextInSector;
            }
        }
        CGraphics::SetZEnable(1);
        CGraphics::SetZWriteEnable(1);
        CGraphics::SetProjection(0x20000, 0x20000, 0x500000, 0x1999);
        for (sectorIndex = 0; sectorIndex < (unsigned int)g_sectorCullEnabled; sectorIndex++) {
            pSector = g_sectors[(unsigned short)g_unk0x006ed5f0[sectorIndex]];
            pMesh = (Mesh *)pSector->pMesh;
            pObject = (StageObjectDraw *)pSector->pObjects;
            if (pMesh != NULL) {
                CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                                                &g_unk0x005207b8);
                Graphics_DrawMeshLOD(pMesh, 1, 0, 0);
                while (pObject != NULL) {
                    if (*(int *)((BYTE *)pObject->pMesh + 0x114) < 0x140000) {
                        if ((pObject->pMesh->flags & 2) == 0) {
                            CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                                                            &g_unk0x005207b8);
                        } else {
                            float scale;

                            *(D3DMATRIX *)pObject->matrix = g_unk0x00597cc0;
                            if (pObject->scaleX != 0x10000 || pObject->scaleY != 0x10000 ||
                                pObject->scaleZ != 0x10000) {
                                scale = (float)pObject->scaleX * CGraphics::m_oneOver65536;
                                pObject->matrix[0] = scale * pObject->matrix[0];
                                pObject->matrix[1] = scale * pObject->matrix[1];
                                pObject->matrix[2] = scale * pObject->matrix[2];
                                scale = (float)pObject->scaleY * CGraphics::m_oneOver65536;
                                pObject->matrix[4] = scale * pObject->matrix[4];
                                pObject->matrix[5] = scale * pObject->matrix[5];
                                pObject->matrix[6] = scale * pObject->matrix[6];
                                scale = (float)pObject->scaleZ * CGraphics::m_oneOver65536;
                                pObject->matrix[8] = scale * pObject->matrix[8];
                                pObject->matrix[9] = scale * pObject->matrix[9];
                                pObject->matrix[10] = scale * pObject->matrix[10];
                            }
                            pObject->matrix[12] = (float)pObject->offsetX * CGraphics::m_oneOver65536;
                            pObject->matrix[13] = (float)pObject->offsetY * CGraphics::m_oneOver65536;
                            pObject->matrix[14] = (float)pObject->offsetZ * CGraphics::m_oneOver65536;
                            CGraphics::m_pTextureManager->pD3D->SetTransform(
                                D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)pObject->matrix);
                        }
                        Graphics_DrawMeshLOD(pObject->pMesh, 1, 0, 0);
                    }
                    pObject = pObject->pNext;
                }
            }
        }
        FUN_0049d290(bit);
        Game_DrawSortedNodes(bit);
        CGraphics::m_pTextureManager->pD3D->EndScene();
    }
    CGraphics::SetProjection(0x20000, 0x20000, farPlane, 0x10000);
    Graphics_SetRenderTarget((Texture *)((BYTE *)g_pGraphics + 0x150));
    CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_VIEW, &view);
    CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &projection);
    FUN_0049dcc0(1);
    return 1;
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

// GLOBAL: CMR2 0x005210b8
int g_unk0x005210b8 = -1;
// GLOBAL: CMR2 0x005210bc
int g_unk0x005210bc = 1;
// GLOBAL: CMR2 0x005210c0
int g_unk0x005210c0 = 1;
// GLOBAL: CMR2 0x005210c4
int g_unk0x005210c4 = 5;
// GLOBAL: CMR2 0x00521138
BYTE g_unk0x00521138[32][0x30] = {
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },
    { 5 }
};

// FUNCTION: CMR2 0x004b2970
void FUN_004b2970(int value)
{
    g_unk0x005210bc = value;
}

// GLOBAL: CMR2 0x005210d0
float g_unk0x005210d0 = 0.5f;
// GLOBAL: CMR2 0x006dfdf8
float g_unk0x006dfdf8;

FixMatrix *FloatMatrix_ToFix(FixMatrix *pOut, D3DMATRIX *pIn);
D3DMATRIX *FixMatrix_ToFloat(D3DMATRIX *pOut, FixMatrix *pIn);
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);
extern unsigned int g_unk0x006de95c[20];
extern unsigned short g_unk0x006dd9bc[2000];

// Draws the triangles of a mesh in contiguous texture runs, with the reserved
// cube map of the mesh projected on them: the inverse of the view * world
// matrix (both translations removed) becomes the texture transform of stage 0,
// so the texture coordinates are generated from the camera space position.
// FUNCTION: CMR2 0x004b2980
void Mesh_DrawEnvMapped(Mesh *pMesh)
{
    D3DMATRIX world;
    D3DMATRIX view;
    FixMatrix worldFix;
    FixMatrix combined;
    FixMatrix inverse;
    FixMatrix viewFix;
    D3DMATRIX projected;
    int triangleCount;
    int count;
    int currentTexture;
    int i;
    int textureIndex;
    int texture;

    count = 0;
    triangleCount = pMesh->triangleCount;
    currentTexture = -1;
    if ((*(BYTE *)&g_pGraphics->field913_0x3bc & 0x80) != 0) {
        if ((int)g_unk0x006de95c[(pMesh->flags >> 15) & 7] < 0) {
            FUN_004b2610(pMesh);
            return;
        }
        CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_VIEW, &view);
        CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_WORLD, &world);
        view._41 = view._42 = view._43 = view._44 = 0.0f;
        world._41 = world._42 = world._43 = world._44 = 0.0f;
        FloatMatrix_ToFix(&worldFix, &world);
        FloatMatrix_ToFix(&viewFix, &view);
        FixMatrix_Multiply(&combined, &worldFix, &viewFix);
        FixMatrix_Invert(&inverse, &combined);
        FixMatrix_ToFloat(&projected, &inverse);
        CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_TEXTURE0, &projected);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, 1);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LOCALVIEWER, 1);
        FUN_0049dcc0(1);
        CGraphics::SetTextureAddressClamp(0);
        FUN_004a3e20(
            (Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer2[g_unk0x006de95c[(pMesh->flags >> 15) & 7]],
            0xb);
        CGraphics::FUN_004a4850(
            0, (int)CGraphics::m_pTextureManager->textureBuffer2[g_unk0x006de95c[(pMesh->flags >> 15) & 7]]);
        if ((g_unk0x005210b8 >= 0 || (int)g_unk0x006de95c[(pMesh->flags >> 15) & 7] >= 0) &&
            triangleCount > 0) {
            for (i = 0; i < triangleCount; i++) {
                textureIndex = pMesh->pTriangles[i].field_0x30;
                if (currentTexture != *(int *)((BYTE *)&pMesh->pTriangles[i] + 4 + textureIndex * 4)) {
                    if (count > 0) {
                        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
                            D3DPT_TRIANGLELIST,
                            CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
                            pMesh->vertexOffset, pMesh->field_0x10, g_unk0x006dd9bc, count, 0);
                    }
                    count = 0;
                    currentTexture = *(int *)((BYTE *)&pMesh->pTriangles[i] + 4 + textureIndex * 4);
                    if (currentTexture > -1) {
                        FUN_004a3e20(
                            (Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[currentTexture], 5);
                        if (g_unk0x005210bc != 0)
                            CGraphics::FUN_004a4850(
                                1, (int)CGraphics::m_pTextureManager->textureBuffer[currentTexture]);
                    }
                }
                if (currentTexture > -1) {
                    g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[0];
                    g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[1];
                    g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[2];
                    CGame::m_unk0x0059ce18++;
                }
            }
            if (count != 0) {
                texture = *(int *)((BYTE *)&pMesh->pTriangles[triangleCount - 1] + 4 + textureIndex * 4);
                if (texture > -1) {
                    FUN_004a3e20(
                        (Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[texture], 5);
                    if (g_unk0x005210bc != 0)
                        CGraphics::FUN_004a4850(1, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
                    CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
                        D3DPT_TRIANGLELIST,
                        CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
                        pMesh->vertexOffset, pMesh->field_0x10, g_unk0x006dd9bc, count, 0);
                }
            }
        }
        CGraphics::SetTextureAddressClamp(1);
        CGraphics::FUN_004a4850(1, 0);
        CGraphics::FUN_004a4850(2, 0);
        CGraphics::m_pTextureManager->pD3D->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS, 0);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LOCALVIEWER, 0);
    }
}

// 1.0 lives at the original's network constant block (0x511350); using the
// named global instead of a literal keeps reccmp's operand symbol identical.
extern const float g_netOne;

// Startup (C runtime .CRT$XCU) initializer of g_unk0x006dfdf8.
// FUNCTION: CMR2 0x004b2e20
void __cdecl FUN_004b2e20(void)
{
    g_unk0x006dfdf8 = g_netOne - g_unk0x005210d0;
}

#pragma data_seg(".CRT$XCU")
static void (__cdecl *s_graphicsInit)(void) = FUN_004b2e20;
#pragma data_seg()

// FUNCTION: CMR2 0x004b2e40
void FUN_004b2e40(BYTE *p, int value)
{
    *(int *)(p + 0x2c) = value;
}

// Shadow volumes: up to 9 cylinders built from meshes, cached by mesh name.
extern int g_sceneLightState2[10];
extern BYTE g_sceneLightFlag2;
// 4096 / (360 * 65536): 16.16 degrees to a sine table index.
extern double g_unk0x00511300;

// Vertex of a Mesh (0x30 bytes): position and normal, then colour/uv data.
struct MeshVertexF {
    float x, y, z;
    float nx, ny, nz;
    BYTE field_0x18[0x18];
};

// Returns a closed 10-sided cylinder (22 vertices, 20 triangles) enclosing pMesh:
// its radius is the furthest point where a triangle crosses z = 0, its height the
// mesh's z range. Cylinders are cached by name; returns pMesh when the cache is full.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b67f0
Mesh *Mesh_GetShadowCylinder(Mesh *pMesh)
{
    Mesh *pFound;
    Mesh *pCyl;
    MeshVertexF *pVerts;
    MeshVertexF *pA;
    MeshVertexF *pB;
    MeshVertexF *pC;
    MeshVertexF *pLone;
    MeshTriangle *pTri;
    float radius;
    float minZ;
    float maxZ;
    float d;
    int count;
    int angle;
    unsigned short idx;
    int i;
    int n;

    count = g_sceneLightFlag2;
    pFound = NULL;
    for (i = 0; i < count; i++) {
        if (strcmp((char *)g_sceneLightState2[i], (char *)pMesh) == 0) {
            pFound = (Mesh *)g_sceneLightState2[i];
            i = count;
        }
    }
    if (pFound != NULL)
        return pFound;

    if (g_sceneLightFlag2 < 9) {
        pVerts = (MeshVertexF *)pMesh->pVertexData;
        radius = 0.0f;
        minZ = 0.0f;
        maxZ = 0.0f;
        for (i = 0; i < pMesh->field_0x10; i++) {
            if (pVerts[i].z > maxZ)
                maxZ = pVerts[i].z;
            if (pVerts[i].z < minZ)
                minZ = pVerts[i].z;
        }
        pTri = pMesh->pTriangles;
        for (n = pMesh->triangleCount; n > 0; n--, pTri++) {
            pLone = NULL;
            pA = &pVerts[pTri->vertexIndex[0]];
            pB = &pVerts[pTri->vertexIndex[1]];
            pC = &pVerts[pTri->vertexIndex[2]];
            if ((pA->z < 0.0f && pB->z > 0.0f && pC->z > 0.0f) ||
                (pA->z > 0.0f && pB->z < 0.0f && pC->z < 0.0f))
                pLone = pA;
            if ((pB->z < 0.0f && pA->z > 0.0f && pC->z > 0.0f) ||
                (pB->z > 0.0f && pA->z < 0.0f && pC->z < 0.0f))
                pLone = pB;
            if (!((pC->z < 0.0f && pB->z > 0.0f && pA->z > 0.0f) ||
                  (pC->z > 0.0f && pB->z < 0.0f && pA->z < 0.0f)))
                pC = pLone;
            if (pC != NULL) {
                d = (float)sqrt(pC->x * pC->x + pC->y * pC->y);
                if (d > radius)
                    radius = d;
            }
        }

        pCyl = (Mesh *)CFileBuffer::AllocateLockedBuffer(0x108);
        g_sceneLightState2[g_sceneLightFlag2++] = (int)pCyl;
        memset(pCyl, 0, 0x108);
        strcpy((char *)pCyl, (char *)pMesh);
        pCyl->field_0x10 = 22;
        pCyl->triangleCount = 20;
        pCyl->pTriangles = (MeshTriangle *)CFileBuffer::AllocateLockedBuffer(20 * sizeof(MeshTriangle));
        pCyl->pVertexData = (DWORD *)CFileBuffer::AllocateLockedBuffer(pCyl->field_0x10 * sizeof(MeshVertexF));

        // Bottom ring: centre plus 10 points every 36 degrees.
        ((MeshVertexF *)pCyl->pVertexData)[0].x = 0.0f;
        ((MeshVertexF *)pCyl->pVertexData)[0].y = 0.0f;
        angle = 0;
        for (i = 1; i < 11; i++) {
            idx = (unsigned short)(__int64)((double)angle * g_unk0x00511300);
            ((MeshVertexF *)pCyl->pVertexData)[i].x =
                (float)g_sinTable[idx & 0xfff] * CGraphics::m_oneOver65536 * radius;
            ((MeshVertexF *)pCyl->pVertexData)[i].y =
                (float)g_sinTable[(idx + 0x400) & 0xfff] * CGraphics::m_oneOver65536 * radius;
            angle += FixDiv(360 << 16, 10 << 16);
        }
        for (i = 0; i < 11; i++) {
            ((MeshVertexF *)pCyl->pVertexData)[i].z = minZ;
            ((MeshVertexF *)pCyl->pVertexData)[i].nx = 0.0f;
            ((MeshVertexF *)pCyl->pVertexData)[i].ny = 0.0f;
            ((MeshVertexF *)pCyl->pVertexData)[i].nz = -1.0f;
        }
        // Top ring: copy of the bottom one at the top of the mesh.
        for (i = 11; i < pCyl->field_0x10; i++) {
            ((MeshVertexF *)pCyl->pVertexData)[i] = ((MeshVertexF *)pCyl->pVertexData)[i - 11];
            ((MeshVertexF *)pCyl->pVertexData)[i].z = maxZ;
            ((MeshVertexF *)pCyl->pVertexData)[i].nx = 0.0f;
            ((MeshVertexF *)pCyl->pVertexData)[i].ny = 0.0f;
            ((MeshVertexF *)pCyl->pVertexData)[i].nz = 1.0f;
        }
        // Bottom cap as a fan around the centre, top cap offset by 11 vertices.
        for (i = 0; i < 10; i++) {
            pCyl->pTriangles[i].vertexIndex[0] = 0;
            pCyl->pTriangles[i].vertexIndex[1] = i + 1;
            if (i == 9)
                pCyl->pTriangles[9].vertexIndex[2] = 1;
            else
                pCyl->pTriangles[i].vertexIndex[2] = i + 2;
        }
        for (i = 10; i < pCyl->triangleCount; i++) {
            pCyl->pTriangles[i].vertexIndex[0] = pCyl->pTriangles[i - 10].vertexIndex[0] + 11;
            pCyl->pTriangles[i].vertexIndex[1] = pCyl->pTriangles[i - 10].vertexIndex[1] + 11;
            pCyl->pTriangles[i].vertexIndex[2] = pCyl->pTriangles[i - 10].vertexIndex[2] + 11;
        }
        if (pCyl != NULL)
            return pCyl;
    }
    return pMesh;
}

extern void *g_sceneType1Objects[60];
extern int *g_sceneSectorFlags;
extern BYTE *g_sceneLightZones;
extern short *g_sceneSectorZone;

// Attenuates the light-zone vertices of a sector by D3D light `light` (by
// horizontal distance); outside the light's reach they go back to full
// intensity. Returns 1 when any intensity changed.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b6ca0
int Scene_AttenuateSectorLight(int sector, int light)
{
    D3DLIGHT7 *pLight;
    LightZone *pZone;
    int *pIntensity;
    int changed;
    int range;
    int att0;
    int base;
    int att2;
    int limit;
    FixVector lpos;
    FixVector d;
    int dx, dz;
    int wasLit;
    int old;
    int d2;
    int i;

    pLight = (D3DLIGHT7 *)g_sceneType1Objects[light];
    changed = 0;
    if (pLight == NULL)
        return changed;
    range = (int)(__int64)(pLight->dvRange * CGraphics::m_65536);
    att0 = (int)(__int64)(pLight->dvAttenuation0 * CGraphics::m_65536);
    base = FixMul(0x20000, att0);
    att2 = (int)(__int64)(pLight->dvAttenuation2 * CGraphics::m_65536);
    limit = g_sectorHalfSize + range + 0x140000;
    lpos.x = (int)(__int64)(pLight->dvPosition.x * CGraphics::m_65536);
    lpos.y = (int)(__int64)(pLight->dvPosition.y * CGraphics::m_65536);
    lpos.z = (int)(__int64)(pLight->dvPosition.z * CGraphics::m_65536);
    wasLit = g_sceneSectorFlags[sector];
    d.x = g_sectors[sector]->x - lpos.x;
    d.y = g_sectors[sector]->y - lpos.y;
    d.z = g_sectors[sector]->z - lpos.z;
    if (FIX_ABS(d.x) <= limit && FIX_ABS(d.z) <= limit)
        g_sceneSectorFlags[sector] = 1;
    else
        g_sceneSectorFlags[sector] = 0;
    if (g_sceneSectorFlags[sector] != 0) {
        pZone = &((LightZone *)g_sceneLightZones)[g_sceneSectorZone[sector]];
        pIntensity = (int *)(pZone->pVertices + 0x20);
        for (i = 0; i < pZone->vertexCount; i++) {
            old = *pIntensity;
            dx = lpos.x - pIntensity[-6];
            dz = lpos.z - pIntensity[-5];
            *pIntensity = 0x10000;
            if (dx <= range || dz <= range) {
                d2 = FixMul(dx, dx) + FixMul(dz, dz);
                d2 = FixMul(d2, att2) + base;
                d2 = FixDiv(d2, 0x1000000);
                if (d2 < 0)
                    d2 = 0;
                else if (d2 > 0x10000)
                    d2 = 0x10000;
                *pIntensity = d2;
            }
            if (*pIntensity != old)
                changed = 1;
            pIntensity += 12;
        }
    } else if (wasLit != 0) {
        pZone = &((LightZone *)g_sceneLightZones)[g_sceneSectorZone[sector]];
        if (pZone->vertexCount != 0) {
            pIntensity = (int *)(pZone->pVertices + 0x20);
            for (i = 0; i < pZone->vertexCount; i++) {
                old = *pIntensity;
                *pIntensity = 0x10000;
                if (old != 0x10000)
                    changed = 1;
                pIntensity += 12;
            }
            return changed;
        }
    }
    return changed;
}

// FUNCTION: CMR2 0x004b6ef0
void FUN_004b6ef0(int value)
{
    g_unk0x005210c0 = value;
}

extern DWORD g_sceneAmbientD3D;
// Ambient colour used outside the stage lighting.
// GLOBAL: CMR2 0x006e0bac
DWORD g_defaultAmbientD3D;

// Sets up Direct3D lighting for a rendering mode: 0/1 object lights (light 1
// or 2), 2/3 stage ambient with light 0, 4/5 unlit. Lights 1 and 2 stay off
// while g_unk0x005210c0 is clear.
// FUNCTION: CMR2 0x004b6f00
void Graphics_SetLightingMode(int mode)
{
    switch (mode) {
    case 0:
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_defaultAmbientD3D);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        CGraphics::m_pTextureManager->pD3D->LightEnable(0, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, TRUE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, FALSE);
        break;
    case 1:
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_defaultAmbientD3D);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        CGraphics::m_pTextureManager->pD3D->LightEnable(0, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, TRUE);
        break;
    case 2:
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_sceneAmbientD3D);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_COLORVERTEX, FALSE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        CGraphics::m_pTextureManager->pD3D->LightEnable(0, TRUE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, FALSE);
        break;
    case 3:
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_sceneAmbientD3D);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
        CGraphics::m_pTextureManager->pD3D->LightEnable(0, TRUE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, FALSE);
        break;
    case 4:
    case 5:
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_defaultAmbientD3D);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
        CGraphics::m_pTextureManager->pD3D->LightEnable(0, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, FALSE);
        break;
    }
    if (g_unk0x005210c0 == 0) {
        CGraphics::m_pTextureManager->pD3D->LightEnable(1, FALSE);
        CGraphics::m_pTextureManager->pD3D->LightEnable(2, FALSE);
    }
    g_unk0x005210c4 = mode;
}

// FUNCTION: CMR2 0x004b7200
int FUN_004b7200(void)
{
    return g_unk0x005210c4;
}

// FUNCTION: CMR2 0x004b98f0
void FUN_004b98f0(int *p, int value)
{
    if (*p == -1) {
        *p = 0;
        return;
    }
    *p += value;
}

// Damped wobble applied by timer shapes 1 and 4 during their last 16 steps.
// GLOBAL: CMR2 0x00521120
signed char g_timerWobble[24] = { 3, 6, 6, 3, 0, 2, 4, 4, 2, 0, 1, 2, 2, 1, 0, 1, 0 };

// Current value of a timer (32 slots of 0x30 bytes): base + scale * shape(step)
// / 4096, stepping once every 16 ms; once finished the slot goes to shape 5
// and returns its end value.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bc110
int Timer_GetValue(BYTE index)
{
    BYTE *t;
    unsigned int pos;
    unsigned int dur;
    unsigned int v;
    BYTE step;
    int now;

    index &= 0xff;
    t = g_unk0x00521138[index];
    pos = *(unsigned int *)(t + 0x28);
    dur = *(unsigned int *)(t + 4);
    step = (BYTE)pos;
    if (dur <= pos)
        step = (BYTE)dur;
    if (*(int *)(t + 0x24) != 0) {
        (*(int *)(t + 0x24))--;
        return *(int *)(t + 0x10);
    }
    switch (t[0]) {
    case 0:
        v = step;
        break;
    case 1:
        if (dur - 0x11 < step)
            goto wobble;
        v = dur - step - 0x11;
        break;
    case 2:
        v = (int)((dur - step + 1) * (dur - step)) / 2;
        break;
    case 3:
        v = ((step + 1) * (unsigned int)step) / 2;
        break;
    case 4:
        if (dur - 0x11 < step) {
        wobble:
            v = (int)g_timerWobble[16 + step - dur] * (int)(signed char)t[1];
            *(int *)(t + 0x1c) = 0x1000;
        } else {
            v = (int)((dur - step - 0x10) * (dur - step - 0x11)) / 2;
        }
        break;
    case 5:
        v = *(unsigned int *)(t + 0xc);
        break;
    default:
        return *(int *)(t + 0xc);
    }
    if (pos < dur + 3) {
        now = CMain::GetFrameTime();
        if (abs(now - *(int *)(t + 0x2c)) > 16) {
            (*(unsigned int *)(t + 0x28))++;
            *(int *)(t + 0x2c) = CMain::GetFrameTime();
        }
    }
    if (t[0] == 1 || *(unsigned int *)(t + 0x28) < *(unsigned int *)(g_unk0x00521138[index] + 4))
        return (int)(*(int *)(t + 0x1c) * v) / 4096 + *(int *)(t + 8);
    t[0] = 5;
    return *(int *)(t + 0xc);
}

// Whether the timer that p was registered with (its first byte is the slot)
// is still running.
// FUNCTION: CMR2 0x004bc0c0
BYTE FUN_004bc0c0(BYTE *p)
{
    BYTE *timer = g_unk0x00521138[*p];

    if (*(BYTE **)(timer + 0x20) != p)
        return 0;
    if (timer[0x14] != 0)
        return *(unsigned int *)(timer + 0x28) < *(unsigned int *)(timer + 4) + 3;
    return *(unsigned int *)(timer + 0x28) < *(unsigned int *)(timer + 4);
}

// FUNCTION: CMR2 0x004bc3e0
BYTE FUN_004bc3e0(unsigned int index)
{
    BYTE *p = g_unk0x00521138[index & 0xff];
    BYTE r = (BYTE)index;
    int i = 24;

    do {
        r |= *p++;
    } while (--i != 0);
    return r;
}

// FUNCTION: CMR2 0x004bc440
void FUN_004bc440(void)
{
    BYTE *p;

    memset(g_unk0x00521138, 0, sizeof(g_unk0x00521138));
    p = g_unk0x00521138[0];
    do {
        *p = 5;
        p += 0x30;
    } while ((int)p < (int)g_unk0x00521138[32]);
}

// FUNCTION: CMR2 0x004bc470
void FUN_004bc470(BYTE *p)
{
    g_unk0x00521138[*p][0] = 5;
}

// GLOBAL: CMR2 0x008164c8
int g_unk0x008164c8;
// GLOBAL: CMR2 0x00816298
BYTE g_unk0x00816298[0x230];

// GLOBAL: CMR2 0x00816704
int g_unk0x00816704;
// GLOBAL: CMR2 0x00816820
char *g_unk0x00816820[20];
// GLOBAL: CMR2 0x00816974
unsigned int g_unk0x00816974;

// Slow pulsing level (0..0.3) eased near its ends, plus a free-running phase.
// GLOBAL: CMR2 0x0081680c
float g_pulsePhase;
// GLOBAL: CMR2 0x00816810
float g_pulseLevel;
// GLOBAL: CMR2 0x00816814
int g_pulseFrozen;
// GLOBAL: CMR2 0x00816818
float g_pulseMin;
// GLOBAL: CMR2 0x0081681c
float g_pulseSpeed;
// GLOBAL: CMR2 0x00521738
int g_pulseRising = 1;
// GLOBAL: CMR2 0x0052173c
float g_pulseMax = 0.3f;

// Level thresholds and the phase step, taken from the original's constant block.
// GLOBAL: CMR2 0x00511ce8
extern const float g_unk0x00511ce8 = 0.1f;
// GLOBAL: CMR2 0x00511d10
extern const float g_unk0x00511d10 = 0.25f;
// GLOBAL: CMR2 0x00511d14
extern const float g_unk0x00511d14 = 0.15f;
// GLOBAL: CMR2 0x00511d18
extern const float g_unk0x00511d18 = 0.05f;
// GLOBAL: CMR2 0x00511d1c
extern const float g_unk0x00511d1c = 0.004f;
// GLOBAL: CMR2 0x0051136c
extern const float g_unk0x0051136c = 0.2f;

// FUNCTION: CMR2 0x004bcc60
void Pulse_Update(unsigned int dt)
{
    if (g_pulseFrozen != 0)
        return;
    g_pulsePhase = (float)dt * g_unk0x00511d1c + g_pulsePhase;
    if (g_pulseRising == 0) {
        if (g_pulseLevel <= g_unk0x00511d18)
            g_pulseSpeed = 0.0007f;
        else if (g_pulseLevel <= g_unk0x00511ce8)
            g_pulseSpeed = 0.0011f;
        else if (g_pulseLevel <= g_unk0x00511d14)
            g_pulseSpeed = 0.0014f;
        else if (g_pulseLevel <= g_unk0x0051136c)
            g_pulseSpeed = 0.0011f;
        else if (g_pulseLevel <= g_unk0x00511d10)
            g_pulseSpeed = 0.0007f;
        else
            g_pulseSpeed = 0.0004f;
    } else {
        if (g_pulseLevel <= g_unk0x00511d18)
            g_pulseSpeed = 0.0007f;
        else if (g_pulseLevel <= g_unk0x00511ce8)
            g_pulseSpeed = 0.0011f;
        else if (g_pulseLevel <= g_unk0x00511d14)
            g_pulseSpeed = 0.0014f;
        else if (g_pulseLevel <= g_unk0x0051136c)
            g_pulseSpeed = 0.0011f;
        else if (g_pulseLevel <= g_unk0x00511d10)
            g_pulseSpeed = 0.0007f;
        else
            g_pulseSpeed = 0.0004f;
    }
    if (g_pulseLevel >= g_pulseMax)
        g_pulseRising = 0;
    if (g_pulseLevel <= g_pulseMin) {
        if (g_pulseRising == 0) {
            g_pulseRising = 1;
            g_pulseLevel = (float)dt * g_pulseSpeed + g_pulseLevel;
            return;
        }
    } else if (g_pulseRising == 0) {
        g_pulseLevel = g_pulseLevel - (float)dt * g_pulseSpeed;
        return;
    }
    g_pulseLevel = (float)dt * g_pulseSpeed + g_pulseLevel;
}

// FUNCTION: CMR2 0x004bcad0
void FUN_004bcad0(int value)
{
    g_unk0x00816704 = value;
}

// FUNCTION: CMR2 0x004bcae0
int FUN_004bcae0(void)
{
    return g_unk0x00816704;
}

// FUNCTION: CMR2 0x004bcfe0
char *FUN_004bcfe0(unsigned int index)
{
    if (index >= 20)
        return CMain::m_logFileBlankLine;
    if (index >= g_unk0x00816974)
        return CMain::m_logFileBlankLine;
    return g_unk0x00816820[index];
}

// GLOBAL: CMR2 0x00816970
int g_argsParsed;

// Release callback: frees the argument copies.
// FUNCTION: CMR2 0x004bcf80
int Args_Free(void)
{
    unsigned int i;

    for (i = 0; i < g_unk0x00816974; i++) {
        GlobalUnlock(GlobalHandle(g_unk0x00816820[i]));
        GlobalFree(GlobalHandle(g_unk0x00816820[i]));
    }
    return 1;
}

// Splits the command line into space-separated arguments (GlobalAlloc copies);
// returns their count.
// FUNCTION: CMR2 0x004bce80
int Args_Parse(char *pCommandLine)
{
    char word[260];
    char *pOut;
    int last;
    int i;

    last = 0;
    g_unk0x00816974 = 0;
    for (i = 0; i < 20; i++)
        g_unk0x00816820[i] = NULL;
    if (pCommandLine == NULL || *pCommandLine == '\0') {
        g_argsParsed = 1;
        return 0;
    }
    for (;;) {
        memset(word, 0, sizeof(word));
        pOut = word;
        while (*pCommandLine == ' ')
            pCommandLine++;
        while (*pCommandLine != ' ') {
            if (*pCommandLine == '\0') {
                last = 1;
                break;
            }
            *pOut++ = *pCommandLine++;
        }
        g_unk0x00816820[g_unk0x00816974] = (char *)GlobalLock(GlobalAlloc(GHND, lstrlenA(word) + 2));
        lstrcpyA(g_unk0x00816820[g_unk0x00816974], word);
        g_unk0x00816974++;
        if (last) {
            g_argsParsed = 1;
            CGame::RegisterCallback(Args_Free, NULL);
            return g_unk0x00816974;
        }
    }
}

// Case-insensitive search for an argument; returns 1 when present.
// FUNCTION: CMR2 0x004bd010
int Args_Has(char *pArg)
{
    char wanted[260];
    char arg[260];
    unsigned int i;

    strcpy(wanted, pArg);
    _strlwr(wanted);
    for (i = 0; i < g_unk0x00816974; i++) {
        strcpy(arg, FUN_004bcfe0(i));
        _strlwr(arg);
        if (strcmp(wanted, arg) == 0)
            return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x004bcac0
int FUN_004bcac0(void)
{
    g_unk0x008164c8 = 0;
    return 1;
}

// Fog distances (floats written to the render states as raw bits).
// GLOBAL: CMR2 0x008165f4
float g_fogEnd;
// GLOBAL: CMR2 0x008165f8
float g_fogField2;
// GLOBAL: CMR2 0x008165fc
float g_fogField1;
// GLOBAL: CMR2 0x00816600
float g_fogStart;

// Sets the fog range (16.16) and colour (r, g, b in the low bytes) and turns
// on linear vertex fog when fog is enabled.
// FUNCTION: CMR2 0x004bcaf0
void Graphics_SetFog(int start, int end, int a, int b, DWORD colour)
{
    g_fogStart = (float)start * CGraphics::m_oneOver65536;
    g_fogEnd = (float)end * CGraphics::m_oneOver65536;
    g_fogField1 = (float)a * CGraphics::m_oneOver65536;
    g_fogField2 = (float)b * CGraphics::m_oneOver65536;
    CGraphics::m_pTextureManager->pD3D->SetRenderState(
        D3DRENDERSTATE_FOGCOLOR, RGBA_MAKE(((BYTE *)&colour)[0], ((BYTE *)&colour)[1], ((BYTE *)&colour)[2], 0xff));
    if (FUN_004b74c0() != 0) {
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
    }
}

// Re-enables linear vertex fog with the stored range.
// FUNCTION: CMR2 0x004bcbb0
void Graphics_EnableFog(void)
{
    if (FUN_004b74c0() != 0) {
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)&g_fogStart);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGEND, *(DWORD *)&g_fogEnd);
    }
}

// FUNCTION: CMR2 0x004bcc20
void Graphics_DisableFog(void)
{
    if (FUN_004b74d0() != 0 || FUN_004b74c0() != 0) {
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
        CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE);
    }
}

// FUNCTION: CMR2 0x004b2d70
void Graphics_SetRenderTarget(Texture *pTexture)
{
    CGraphics::m_pTextureManager->pD3D->SetRenderTarget(pTexture->pSurface, 0);
}

// Stores a 3-bit value in bits 15..17 of the flags of every mesh in a hierarchy.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b2d90
void SceneNode_SetMeshFlagBits(SceneNode *pNode, unsigned int value)
{
    SceneNode *pChild;
    SceneNode *p;
    Mesh *pMesh;

    if (pNode == NULL)
        return;
    if (pNode->type == SCENE_NODE_MESH && (pMesh = (Mesh *)pNode->pObject) != NULL)
        pMesh->flags = (value & 7) << 15 | pMesh->flags & 0xfffc7fff;
    pChild = pNode->pFirstChild;
    while (pChild != NULL) {
        for (p = pChild; p != NULL; p = p->pFirstChild) {
            if (p->type == SCENE_NODE_MESH && (pMesh = (Mesh *)p->pObject) != NULL)
                pMesh->flags = (value & 7) << 15 | pMesh->flags & 0xfffc7fff;
        }
        pChild = pChild->pNext;
    }
}

// GLOBAL: CMR2 0x0052111c
char g_strSuffixW[4] = "W";

extern char g_fontTgaFormat[12];
int Graphics_HasLocalSuffix(char *pName);
Texture *FUN_004b9b80(char *name);

// Variants of the texture file suffix the load flags depend on.
// GLOBAL: CMR2 0x00521114
char g_str0x00521114[4] = "B3";
// GLOBAL: CMR2 0x00521118
char g_str0x00521118[4] = "B2";

// Loads every texture of a list of texture records. Each record holds a header
// with the number of textures followed by that many index/offset entries into
// the texture directory; the file name is built from the directory of the game
// plus the last path component of the texture entry. Resident textures are
// skipped when the record type is 6.
// match 62%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b9910
void FUN_004b9910(int param1, int param2, unsigned int param3, int param4, int param5)
{
    unsigned int flags = param3;
    unsigned int records;
    unsigned short *pEntry;
    char *pName;
    char fileName[260];
    unsigned int i;

    for (records = param3; records > 0; records--) {
        pEntry = (unsigned short *)((BYTE *)param1 + 0x14);
        for (i = 0; i < *(unsigned short *)((BYTE *)param1 + 0xc); i++) {
            pName = (char *)(param2 + *pEntry * 0x104);
            CGenericFileLoader::StrUpperPolish((BYTE *)pName);
            strcpy(CFrontend::m_stringDest, CInstallInfo::FUN_0040ed50());
            sprintf(fileName, g_fontTgaFormat, CFrontend::m_stringDest,
                    strchr(pName, '\\') + 1);
            if (param4 == 6) {
                if (FUN_004b9b80(fileName) == 0) {
                    if (strncmp(fileName + strlen(fileName) - 6, CGraphics::m_strSuffixBU, 2) != 0 &&
                        strncmp(fileName + strlen(fileName) - 6, g_str0x00521118, 2) != 0 &&
                        strncmp(fileName + strlen(fileName) - 6, g_str0x00521114, 2) != 0)
                        flags = Graphics_HasLocalSuffix(fileName) != 0 ? 0x140 : 0x100;
                    CTexture::FindLoadTexture((GenericFile *)param5, fileName, 0, 0, 0, flags);
                }
            } else {
                CTexture::FindLoadTexture((GenericFile *)param5, fileName, 0, 0, 0,
                                          param4 != 10 ? 0x90 : 0);
            }
            pEntry += 4;
        }
        param1 = (int)((BYTE *)param1 + 0x10 + i * 8);
    }
}

// Returns 1 when a file name ends in a localised suffix: "RU"/"BR" before
// the extension, or 'W' two characters earlier.
// FUNCTION: CMR2 0x004b9af0
int Graphics_HasLocalSuffix(char *pName)
{
    if (strncmp(pName + strlen(pName) - 6, CGraphics::m_strSuffixBR, 2) != 0 &&
        strncmp(pName + strlen(pName) - 6, CGraphics::m_strSuffixRU, 2) != 0 &&
        strncmp(pName + strlen(pName) - 8, g_strSuffixW, 1) != 0)
        return 0;
    return 1;
}

// FUNCTION: CMR2 0x004bca70
void FUN_004bca70(short *param1)
{
    if (g_unk0x008164c8 != 0)
        return;
    CGame::RegisterCallback(FUN_004bcac0, NULL);
    if (param1[2] != 0 && param1[3] != 0) {
        CGraphics::CreateTextureSurface((Texture *)g_unk0x00816298, param1[2], param1[3], 8);
        g_unk0x008164c8 = 1;
    }
}

// Screen rectangle and colour of the sun for the lens flare test.
// GLOBAL: CMR2 0x008164d0
short g_flareRect[4];
// GLOBAL: CMR2 0x008164d8
BYTE g_flareColour[3];
// GLOBAL: CMR2 0x008164dc
BYTE g_flareVisibility;
// GLOBAL: CMR2 0x008164dd
BYTE g_flareTolerance;

// Lowest set bit and number of set bits of a colour mask.
#define MASK_SHIFT(mask, out)             \
    v = (mask);                           \
    for (k = 0; k < 32; k++) {            \
        if (v & 1)                        \
            break;                        \
        v >>= 1;                          \
    }                                     \
    out = (BYTE)k
#define MASK_BITS(mask, out)              \
    v = (mask);                           \
    n = 0;                                \
    for (k = 32; k != 0; k--) {           \
        if (v & 1)                        \
            n++;                          \
        v >>= 1;                          \
    }                                     \
    out = (BYTE)n

// Lens flare occlusion test. With a rectangle and a colour, stores them for
// the next test and returns the last result; without, copies that screen
// rectangle out of the back buffer and returns the percentage of its pixels
// still showing the sun's colour (within the tolerance).
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004bc490
BYTE Flare_SampleVisibility(short *pRect, BYTE *pColour, BYTE tolerance)
{
    DDSURFACEDESC2 desc;
    RECT src;
    RECT dst;
    IDirectDrawSurface7 *pSurface;
    unsigned short resX;
    unsigned short resY;
    short x;
    short y;
    short w;
    short h;
    short rLo;
    short rHi;
    short gLo;
    short gHi;
    short bLo;
    short bHi;
    short c;
    int area;
    int count;
    int rShift;
    int gShift;
    int bShift;
    int rBits;
    int gBits;
    int bBits;
    unsigned int v;
    unsigned int pixel;
    unsigned short row;
    unsigned short col;
    int k;
    int n;

    resX = (unsigned short)g_pGraphics->resX;
    resY = (unsigned short)g_pGraphics->resY;
    count = 0;
    if (g_unk0x008164c8 == 0) {
        if (pRect == NULL)
            goto sample;
        FUN_004bca70(pRect);
        if (g_unk0x008164c8 == 0)
            return 0;
    }
    if (pRect != NULL && pColour != NULL) {
        g_flareRect[0] = pRect[0];
        g_flareRect[1] = pRect[1];
        g_flareRect[2] = pRect[2];
        g_flareRect[3] = pRect[3];
        g_flareColour[0] = pColour[0];
        g_flareColour[1] = pColour[1];
        g_flareColour[2] = pColour[2];
        g_flareTolerance = tolerance;
        return g_flareVisibility;
    }
sample:
    rHi = g_flareColour[0] + g_flareTolerance;
    rLo = g_flareColour[0] - g_flareTolerance;
    gHi = g_flareColour[1] + g_flareTolerance;
    gLo = g_flareColour[1] - g_flareTolerance;
    bHi = g_flareColour[2] + g_flareTolerance;
    bLo = g_flareColour[2] - g_flareTolerance;
    area = g_flareRect[2] * g_flareRect[3];
    if (area == 0)
        return 0;

    // Clip the rectangle to the screen.
    x = g_flareRect[0];
    y = g_flareRect[1];
    w = g_flareRect[2];
    h = g_flareRect[3];
    if (x < 0) {
        if (w > -x) {
            w += x;
            x = 0;
        } else {
            w = 0;
            x = 0;
        }
    }
    if (y < 0) {
        if (h > -y) {
            h += y;
            y = 0;
        } else {
            h = 0;
            y = 0;
        }
    }
    if (x >= (int)resX) {
        w = 0;
        x = 0;
    }
    if (y >= (int)resY) {
        h = 0;
        y = 0;
    }
    if (x + w >= (int)resX)
        w = resX - x;
    if (y + h >= (int)resY)
        h = resY - y;
    if (w == 0 || h == 0)
        return 0;

    src.left = x;
    src.top = y;
    src.right = x + w;
    src.bottom = y + h;
    dst.left = 0;
    dst.top = 0;
    dst.right = w;
    dst.bottom = h;
    desc.dwSize = 0x7c;
    pSurface = ((Texture *)g_unk0x00816298)->pSurface;
    if (pSurface->Blt(&dst, g_pGraphics->pBackBufferSurface, &src, DDBLT_WAIT, NULL) != DD_OK)
        return 0;
    pSurface = ((Texture *)g_unk0x00816298)->pSurface;
    pSurface->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    MASK_SHIFT(desc.ddpfPixelFormat.dwRBitMask, rShift);
    MASK_SHIFT(desc.ddpfPixelFormat.dwGBitMask, gShift);
    MASK_SHIFT(desc.ddpfPixelFormat.dwBBitMask, bShift);
    MASK_BITS(desc.ddpfPixelFormat.dwRBitMask, rBits);
    MASK_BITS(desc.ddpfPixelFormat.dwGBitMask, gBits);
    MASK_BITS(desc.ddpfPixelFormat.dwBBitMask, bBits);
    if (desc.ddpfPixelFormat.dwRGBBitCount == 16) {
        for (row = 0; row < h; row++) {
            for (col = 0; col < w; col++) {
                pixel = ((unsigned short *)desc.lpSurface)[(row * desc.lPitch) / 2 + col];
                c = (BYTE)(((pixel & desc.ddpfPixelFormat.dwRBitMask) >> rShift) << (8 - rBits));
                if (c < rLo || c > rHi)
                    continue;
                c = (BYTE)(((pixel & desc.ddpfPixelFormat.dwGBitMask) >> gShift) << (8 - gBits));
                if (c < gLo || c > gHi)
                    continue;
                c = (BYTE)(((pixel & desc.ddpfPixelFormat.dwBBitMask) >> bShift) << (8 - bBits));
                if (c < bLo || c > bHi)
                    continue;
                count++;
            }
        }
    } else if (desc.ddpfPixelFormat.dwRGBBitCount == 32) {
        for (row = 0; row < h; row++) {
            for (col = 0; col < w; col++) {
                pixel = ((DWORD *)desc.lpSurface)[(row * desc.lPitch) / 4 + col];
                c = (BYTE)((pixel & desc.ddpfPixelFormat.dwRBitMask) >> rShift);
                if (c < rLo || c > rHi)
                    continue;
                c = (BYTE)((pixel & desc.ddpfPixelFormat.dwGBitMask) >> gShift);
                if (c < gLo || c > gHi)
                    continue;
                c = (BYTE)((pixel & desc.ddpfPixelFormat.dwBBitMask) >> bShift);
                if (c < bLo || c > bHi)
                    continue;
                count++;
            }
        }
    }
    pSurface->Unlock(NULL);
    g_flareVisibility = (BYTE)(count * 100 / area);
    return g_flareVisibility;
}

// GLOBAL: CMR2 0x006db200
unsigned short g_unk0x006db200[800 * 6];
// GLOBAL: CMR2 0x006dd784
int g_unk0x006dd784;
// GLOBAL: CMR2 0x006dd788
int g_unk0x006dd788;
void Billboard_Reset(void);

// Builds the 800-entry triangle-strip index table.
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b1150
void FUN_004b1150(void)
{
    int i;
    unsigned short *pIndex;

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
    CGame::RegisterCallback(Billboard_Reset, NULL);
}

// Camera-facing quad requested by the game (0x24 bytes); 16.16 fixed point.
struct BillboardDef {
    FixVector pos;              // 0x0
    int top;                    // 0xc
    int left;                   // 0x10
    int bottom;                 // 0x14
    int right;                  // 0x18
    BYTE r, g, b, a;            // 0x1c
    short field_0x20;           // 0x20 rotation (12-bit angle)
    BYTE shade;                 // 0x22 0 = fully lit ... 256 = scene dark colour
    BYTE flags;                 // 0x23 1 mirrored, 2 lit by the scene light
    int field_0x24;
};

// Queued billboard in render format (0x58 bytes).
struct BillboardQuad {
    D3DVECTOR corner[4];        // 0x0  offsets from pos in camera space (y right, z up)
    unsigned short texture;     // 0x30
    D3DVECTOR pos;              // 0x34
    D3DCOLOR colour;            // 0x40
    BYTE field_0x44[0xc];
    int mirror;                 // 0x50 flip the texture horizontally
    short field_0x54;           // 0x54 rotation about the view axis (12-bit angle)
};

// Runs of consecutive quads sharing a texture: {count, texture}.
// GLOBAL: CMR2 0x006c84f8
int *g_billboardRun;
// GLOBAL: CMR2 0x006c8500
int g_billboardRuns[800][2];
// GLOBAL: CMR2 0x006c9e00
BillboardQuad g_billboards[800];
// GLOBAL: CMR2 0x006dd780
int g_billboardsEnabled;
// GLOBAL: CMR2 0x006dd78c
int g_billboardsRequested;

void Scene_GetLightColour(DWORD *pColour, int level);

// Queues a billboard for this frame (at most 800), grouping it with the
// previous one when both use the same texture.
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b11c0
void Billboard_Add(BillboardDef *pDef, unsigned short *pTexture)
{
    BillboardQuad *pQuad;
    float left;
    float top;
    float right;
    float bottom;
    BYTE dark[4];
    BYTE bright[4];
    int t;

    if (g_billboardsEnabled == 0) {
        if (g_billboardsRequested == 0)
            g_billboardsRequested = 1;
        return;
    }
    if (pTexture == NULL || pDef == NULL || (unsigned int)g_unk0x006dd784 >= 800)
        return;
    pQuad = &g_billboards[g_unk0x006dd784];
    left = (float)pDef->left * CGraphics::m_oneOver65536;
    top = (float)pDef->top * CGraphics::m_oneOver65536;
    right = (float)pDef->right * CGraphics::m_oneOver65536;
    bottom = (float)pDef->bottom * CGraphics::m_oneOver65536;
    pQuad->corner[0].x = 0.0f;
    pQuad->corner[1].x = 0.0f;
    pQuad->corner[2].x = 0.0f;
    pQuad->corner[3].x = 0.0f;
    pQuad->corner[0].y = -left;
    pQuad->corner[0].z = top;
    pQuad->corner[1].y = -left;
    pQuad->corner[1].z = bottom;
    pQuad->corner[2].y = -right;
    pQuad->corner[2].z = top;
    pQuad->corner[3].y = -right;
    pQuad->corner[3].z = bottom;
    pQuad->texture = *pTexture;
    pQuad->pos.x = (float)pDef->pos.x * CGraphics::m_oneOver65536;
    pQuad->pos.y = (float)pDef->pos.y * CGraphics::m_oneOver65536;
    pQuad->pos.z = (float)pDef->pos.z * CGraphics::m_oneOver65536;
    if ((pDef->flags & 2) == 0) {
        pQuad->colour = RGBA_MAKE(pDef->r, pDef->g, pDef->b, pDef->a);
    } else {
        Scene_GetLightColour((DWORD *)dark, 0);
        Scene_GetLightColour((DWORD *)bright, 0x10000);
        dark[0] = dark[0] * pDef->r / 256;
        dark[1] = dark[1] * pDef->g / 256;
        dark[2] = dark[2] * pDef->b / 256;
        bright[0] = bright[0] * pDef->r / 256;
        bright[1] = bright[1] * pDef->g / 256;
        bright[2] = bright[2] * pDef->b / 256;
        t = pDef->shade;
        pQuad->colour = RGBA_MAKE((bright[0] * (256 - t) + dark[0] * t) >> 8,
                                  (bright[1] * (256 - t) + dark[1] * t) >> 8,
                                  (bright[2] * (256 - t) + dark[2] * t) >> 8, pDef->a);
    }
    pQuad->mirror = pDef->flags & 1;
    pQuad->field_0x54 = pDef->field_0x20;
    if (g_unk0x006dd784 == 0) {
        g_billboardRun = g_billboardRuns[0];
        g_billboardRuns[0][0] = 1;
        g_billboardRuns[0][1] = pQuad->texture;
        g_unk0x006dd788 = 1;
        g_unk0x006dd784++;
        return;
    }
    if (pQuad->texture != g_billboardRun[1]) {
        g_billboardRun += 2;
        g_unk0x006dd788++;
        g_billboardRun[0] = 1;
        g_billboardRun[1] = pQuad->texture;
        g_unk0x006dd784++;
        return;
    }
    g_billboardRun[0]++;
    g_unk0x006dd784++;
}

// Release callback: empties the billboard queue and disables it.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b1500
void Billboard_Reset(void)
{
    memset(g_billboards, 0, sizeof(g_billboards));
    g_billboardsEnabled = g_unk0x006dd788 = g_unk0x006dd784 = 0;
}

// Vertex of a billboard (D3DFVF_XYZ | NORMAL | DIFFUSE | SPECULAR | TEX2).
struct BillboardVertex {
    float x, y, z;
    float nx, ny, nz;
    D3DCOLOR diffuse;
    D3DCOLOR specular;
    float u, v;
    float u2, v2;
};

// GLOBAL: CMR2 0x006a2cf8
BillboardVertex g_billboardVerts[800 * 4];

void FUN_004a3dd0(void);
D3DMATRIX *FixMatrix_ToFloat(D3DMATRIX *pOut, FixMatrix *pIn);

// Turns the queued billboards into quads facing pCamera (rotated by their
// angle), draws them one texture run at a time and empties the queue.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b1530
void Billboard_Draw(SceneNode *pCamera)
{
    BillboardVertex *pVert;
    BillboardQuad *pQuad;
    D3DMATRIX axes;
    D3DMATRIX view;
    D3DVECTOR pos;
    D3DVECTOR c0, c1, c2, c3;
    unsigned short angle;
    D3DCOLOR colour;
    float c, s;
    int n;
    unsigned int i;
    int first;

    first = 0;
    if (g_billboardsEnabled == 0 || g_unk0x006dd784 == 0)
        return;
    pVert = g_billboardVerts;
    FixMatrix_ToFloat(&view, &pCamera->world);
    if (g_unk0x006dd784 != 0) {
        n = g_unk0x006dd784;
        pQuad = g_billboards;
        CGame::m_unk0x0059ce20 += g_unk0x006dd784 * 2;
        do {
            pos = pQuad->pos;
            c0 = pQuad->corner[0];
            c1 = pQuad->corner[1];
            c2 = pQuad->corner[2];
            c3 = pQuad->corner[3];
            angle = pQuad->field_0x54;
            colour = pQuad->colour;
            if (angle == 0) {
                axes = view;
            } else {
                c = (float)g_sinTable[(angle + 0x400) & 0xfff] * CGraphics::m_oneOver65536;
                s = (float)g_sinTable[angle & 0xfff] * CGraphics::m_oneOver65536;
                axes._11 = view._11 * c + view._21 * s;
                axes._12 = view._12 * c + view._22 * s;
                axes._13 = view._13 * c + view._23 * s;
                axes._21 = view._21 * c - view._11 * s;
                axes._22 = view._22 * c - view._12 * s;
                axes._23 = view._23 * c - view._13 * s;
            }
            pVert[0].x = c0.z * axes._21 + c0.y * axes._11 + pos.x;
            pVert[0].y = c0.z * axes._22 + c0.y * axes._12 + pos.y;
            pVert[0].z = c0.z * axes._23 + c0.y * axes._13 + pos.z;
            if (pQuad->mirror == 0) {
                pVert[0].u = 0.0f;
                pVert[0].v = 1.0f;
            } else {
                pVert[0].u = 1.0f;
                pVert[0].v = 1.0f;
            }
            pVert[0].diffuse = colour;
            pVert[0].specular = 0xff000000;
            pVert[1].x = c1.z * axes._21 + c1.y * axes._11 + pos.x;
            pVert[1].y = c1.z * axes._22 + c1.y * axes._12 + pos.y;
            pVert[1].z = c1.z * axes._23 + c1.y * axes._13 + pos.z;
            if (pQuad->mirror == 0)
                pVert[1].u = 0.0f;
            else
                pVert[1].u = 1.0f;
            pVert[1].v = 0.0f;
            pVert[1].diffuse = colour;
            pVert[1].specular = 0xff000000;
            pVert[2].x = c2.z * axes._21 + c2.y * axes._11 + pos.x;
            pVert[2].y = c2.z * axes._22 + c2.y * axes._12 + pos.y;
            pVert[2].z = c2.z * axes._23 + c2.y * axes._13 + pos.z;
            if (pQuad->mirror == 0)
                pVert[2].u = 1.0f;
            else
                pVert[2].u = 0.0f;
            pVert[2].v = 1.0f;
            pVert[2].diffuse = colour;
            pVert[2].specular = 0xff000000;
            pVert[3].x = c3.z * axes._21 + c3.y * axes._11 + pos.x;
            pVert[3].y = c3.z * axes._22 + c3.y * axes._12 + pos.y;
            pVert[3].z = c3.z * axes._23 + c3.y * axes._13 + pos.z;
            if (pQuad->mirror == 0)
                pVert[3].u = 1.0f;
            else
                pVert[3].u = 0.0f;
            pVert[3].v = 0.0f;
            pVert[3].diffuse = colour;
            pVert[3].specular = 0xff000000;
            pVert += 4;
            pQuad++;
        } while (--n != 0);
    }
    CGraphics::SetZWriteEnable(0);
    g_billboardRun = g_billboardRuns[0];
    for (i = 0; i < (unsigned int)g_unk0x006dd788; i++) {
        CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[g_billboardRun[1]]);
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0x2d2, &g_billboardVerts[first * 4],
                                                                 g_billboardRun[0] * 4, g_unk0x006db200,
                                                                 g_billboardRun[0] * 6, 0);
        first += g_billboardRun[0];
        g_billboardRun += 2;
    }
    CGraphics::SetZWriteEnable(1);
    g_unk0x006dd784 = 0;
    g_unk0x006dd788 = 0;
    FUN_004a3dd0();
    CGraphics::FUN_004a3de0();
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
// GLOBAL: CMR2 0x006a2bc8
int g_unk0x006a2bc8;
// GLOBAL: CMR2 0x00520f94
BYTE g_unk0x00520f94[4] = { 0, 0, 0, 70 };
// GLOBAL: CMR2 0x00520f98
Quad2DInputVertex g_projectedQuad[4] = {
    { 0, 0, 0, { 0xff, 0xff, 0xff, 0xff }, 0, 0 },
    { 0, 0, 0, { 0xff, 0xff, 0xff, 0xff }, 0xfff9, 0 },
    { 0, 0, 0, { 0xff, 0xff, 0xff, 0xff }, 0xfff9, 0xfff9 },
    { 0, 0, 0, { 0xff, 0xff, 0xff, 0xff }, 0, 0xfff9 },
};
// GLOBAL: CMR2 0x00521004
FixVector g_quadBasisA = {0, 0x10000, 0};
// GLOBAL: CMR2 0x00521010
FixVector g_quadBasisB = {0, 0, 0x10000};

// FUNCTION: CMR2 0x004ae140
void FUN_004ae140(BYTE *pColour)
{
    g_unk0x00520f94[0] = pColour[0];
    g_unk0x00520f94[1] = pColour[1];
    g_unk0x00520f94[2] = pColour[2];
    g_unk0x00520f94[3] = pColour[3];
}

// FUNCTION: CMR2 0x004ae230
void FUN_004ae230(int *p, int x, int y)
{
    p[3] = -x;
    p[4] = y;
    p[5] = x;
    p[6] = -y;
}

// FUNCTION: CMR2 0x004ae260
void FUN_004ae260(void)
{
    int i;

    for (i = 0; i < g_unk0x006a2bcc; i++)
        *(int *)((BYTE *)g_unk0x006a2a98 + i * 0x5c) = 0;
    g_unk0x006a2bc8 = 0;
}

// FUNCTION: CMR2 0x004ae3d0
void FUN_004ae3d0(BYTE *p, BYTE value)
{
    if (p != NULL)
        p[0x50] = value;
}

// FUNCTION: CMR2 0x004ae3f0
void FUN_004ae3f0(BYTE *p, int value)
{
    if (p != NULL)
        *(int *)(p + 0x3c) = value;
}

// FUNCTION: CMR2 0x004ae410
void FUN_004ae410(BYTE a, BYTE b, int c, int d)
{
}

// Draws a fading rectangle around a point projected onto the given plane.
// The corners and colours use the fixed-point triangle queue's shared scratch.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ae950
void Graphics_DrawProjectedQuad(BYTE *pSurface, FixVector *pPoint, FixVector *pTarget, FixVector *pUnused)
{
    FixVector *pPlanePoint = (FixVector *)(pSurface + 0x1c);
    FixVector *pNormal = (FixVector *)(pSurface + 0x28);
    FixVector displacement;
    FixVector projected;
    FixVector offset;
    FixVector axisA;
    FixVector axisB;
    FixVector step;
    int depth;
    int fade;
    int size;
    int length;
    int reciprocal;
    int colour;
    int i;
    BYTE intensity;

    displacement.x = pPoint->x - pPlanePoint->x;
    displacement.y = pPoint->y - pPlanePoint->y;
    displacement.z = pPoint->z - pPlanePoint->z;
    depth = FixVecDot(pNormal, &displacement);
    FixVecScale(&offset, pNormal, depth);
    projected.x = pPoint->x - offset.x;
    projected.y = pPoint->y - offset.y;
    projected.z = pPoint->z - offset.z;
    if (depth < 0)
        depth = -depth;
    fade = FixMul(depth - 0x6666, 0x1aaac);
    if (fade < 0)
        fade = 0;
    else if (fade > 0x10000)
        fade = 0x10000;
    fade = 0x10000 - fade;

    depth = FixMul(pNormal->x, 0x10000);
    axisA.x = 0x10000 - FixMul(pNormal->x, depth);
    axisA.y = -FixMul(pNormal->y, depth);
    axisA.z = -FixMul(pNormal->z, depth);
    length = FixVecLength(&axisA);
    if (length == 0) {
        axisA.x = 0; axisA.y = 0; axisA.z = 0;
    } else {
        FixVecScaleRecip(&axisA, &axisA, length);
    }
    FixVecCross(&axisB, &axisA, pNormal);
    length = FixVecLength(&axisB);
    if (length == 0) {
        axisB.x = 0; axisB.y = 0; axisB.z = 0;
    } else {
        FixVecScaleRecip(&axisB, &axisB, length);
    }
    size = FixMul(*(int *)(pSurface + 0x40), *(int *)(pSurface + 0x34));
    FixVecScale(&axisA, &axisA, size);
    size = FixMul(*(int *)(pSurface + 0x40), *(int *)(pSurface + 0x34));
    FixVecScale(&axisB, &axisB, size);

    displacement.x = pTarget->x - projected.x;
    displacement.y = pTarget->y - projected.y;
    displacement.z = pTarget->z - projected.z;
    length = FixVecLength(&displacement);
    if (length > 0x10000) {
        reciprocal = FixDiv(0x10000, length);
        FixVecScale(&step, &displacement, reciprocal);
        projected.x += step.x;
        projected.y += step.y;
        projected.z += step.z;
        reciprocal = 0x10000 - reciprocal;
        FixVecScale(&axisA, &axisA, reciprocal);
        FixVecScale(&axisB, &axisB, reciprocal);
    }

    g_projectedQuad[0].x = projected.x + axisA.x - axisB.x;
    g_projectedQuad[0].y = projected.y + axisA.y - axisB.y;
    g_projectedQuad[0].z = projected.z + axisA.z - axisB.z;
    g_projectedQuad[1].x = projected.x + axisA.x + axisB.x;
    g_projectedQuad[1].y = projected.y + axisA.y + axisB.y;
    g_projectedQuad[1].z = projected.z + axisA.z + axisB.z;
    g_projectedQuad[2].x = projected.x - axisA.x + axisB.x;
    g_projectedQuad[2].y = projected.y - axisA.y + axisB.y;
    g_projectedQuad[2].z = projected.z - axisA.z + axisB.z;
    g_projectedQuad[3].x = projected.x - axisA.x - axisB.x;
    g_projectedQuad[3].y = projected.y - axisA.y - axisB.y;
    g_projectedQuad[3].z = projected.z - axisA.z - axisB.z;

    intensity = (BYTE)(((unsigned int)pSurface[0x51] * FixMul(fade, *(int *)(pSurface + 0x3c))) >> 16);
    colour = 0xff000000 | ((int)intensity << 16) | ((int)intensity << 8) | intensity;
    for (i = 0; i < 4; i++)
        *(int *)g_projectedQuad[i].colour = colour;
    Quad2D_QueueFixedTriangle(0, &g_projectedQuad[0], &g_projectedQuad[1], &g_projectedQuad[2],
                              *(Texture **)(pSurface + 0x48), (Quad2D *)0xe);
    Quad2D_QueueFixedTriangle(0, &g_projectedQuad[0], &g_projectedQuad[2], &g_projectedQuad[3],
                              *(Texture **)(pSurface + 0x48), (Quad2D *)0xe);
}

// FUNCTION: CMR2 0x004b1970
int CGraphics::FUN_004b1970(void)
{
    return (unsigned int)m_unk0x006dd890 >> 10;
}
// GLOBAL: CMR2 0x006a2a10
Quad2DInputVertex g_layerQuad[4];
// Glow billboard being built; its position anchors the layer quad.
// GLOBAL: CMR2 0x006a2a70
BillboardDef g_glowDef;

// Projects the layer anchor onto a plane, stretches it toward a target and
// draws a four-vertex strip with distance-based greyscale opacity.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ae470
void Graphics_DrawLayerQuad(BYTE *pSurface, FixVector *pTarget)
{
    FixVector *pPlanePoint = (FixVector *)(pSurface + 0x1c);
    FixVector *pNormal = (FixVector *)(pSurface + 0x28);
    FixVector displacement;
    FixVector projected;
    FixVector offset;
    FixVector axisA;
    FixVector axisB;
    int depth;
    int length;
    int extension;
    int residual;
    int opacity;
    int fade;
    int colour;
    int i;
    BYTE intensity;

    displacement.x = g_glowDef.pos.x - pPlanePoint->x;
    displacement.y = g_glowDef.pos.y - pPlanePoint->y;
    displacement.z = g_glowDef.pos.z - pPlanePoint->z;
    depth = FixVecDot(pNormal, &displacement);
    FixVecScale(&offset, pNormal, depth);
    projected.x = g_glowDef.pos.x - offset.x;
    projected.y = g_glowDef.pos.y - offset.y;
    projected.z = g_glowDef.pos.z - offset.z;
    fade = FixMul(depth, 0x20000);
    opacity = FixMul(g_glowDef.top, 0x9999);

    displacement.x = pTarget->x - projected.x;
    displacement.y = pTarget->y - projected.y;
    displacement.z = pTarget->z - projected.z;
    length = FixVecLength(&displacement);
    extension = length - 0x30000;
    if (extension > 0xa0000)
        extension = 0xa0000;
    FixVecScaleRecip(&displacement, &displacement, length);
    FixVecScale(&displacement, &displacement, extension);
    projected.x += displacement.x;
    projected.y += displacement.y;
    projected.z += displacement.z;

    residual = 0x10000 - FixDiv(extension, length);
    FixVecScale(&axisA, &g_quadBasisA, fade);
    FixVecScale(&axisA, &axisA, residual);
    FixVecScale(&axisB, &g_quadBasisB, opacity);
    FixVecScale(&axisB, &axisB, residual);

    g_layerQuad[0].x = projected.x - axisB.x;
    g_layerQuad[0].y = projected.y - axisB.y;
    g_layerQuad[0].z = projected.z - axisB.z;
    g_layerQuad[1].x = projected.x + axisB.x;
    g_layerQuad[1].y = projected.y + axisB.y;
    g_layerQuad[1].z = projected.z + axisB.z;
    g_layerQuad[2].x = projected.x - axisA.x + axisB.x;
    g_layerQuad[2].y = projected.y - axisA.y + axisB.y;
    g_layerQuad[2].z = projected.z - axisA.z + axisB.z;
    g_layerQuad[3].x = projected.x - axisA.x - axisB.x;
    g_layerQuad[3].y = projected.y - axisA.y - axisB.y;
    g_layerQuad[3].z = projected.z - axisA.z - axisB.z;

    fade = depth - 0x20000;
    if (fade < 0)
        fade = 0;
    fade = FixMul(fade, 0x20000);
    if (fade > 0x10000)
        fade = 0x10000;
    intensity = (BYTE)FixMulShift32(FixMul(0x10000 - fade, *(int *)(pSurface + 0x44)),
                                    (int)g_glowDef.r << 16);
    colour = 0xff000000 | ((int)intensity << 16) | ((int)intensity << 8) | intensity;
    for (i = 0; i < 4; i++)
        *(int *)g_layerQuad[i].colour = colour;
    Quad2D_QueueFixedTriangle(0, &g_layerQuad[0], &g_layerQuad[1], &g_layerQuad[2],
                              *(Texture **)(pSurface + 0x4c), (Quad2D *)0x16);
    Quad2D_QueueFixedTriangle(0, &g_layerQuad[0], &g_layerQuad[2], &g_layerQuad[3],
                              *(Texture **)(pSurface + 0x4c), (Quad2D *)0x16);
}
// Light glow source (0x5c bytes), one per entry of g_unk0x006a2a98.
struct GlowLight {
    int type;                   // 0x0  0 free, 2 seen from behind, 3 seen from both sides
    FixVector pos;              // 0x4  local to pNode when set
    FixVector dir;              // 0x10
    FixVector planePoint;       // 0x1c
    FixVector planeNormal;      // 0x28
    int sizeX;                  // 0x34
    int sizeY;                  // 0x38
    int intensity;              // 0x3c
    int field_0x40;
    int layerIntensity;         // 0x44 draw the ground layer quad when non-zero
    unsigned short *pTexture;   // 0x48
    Texture *pLayerTexture;     // 0x4c
    BYTE enabled;               // 0x50
    BYTE projected;             // 0x51 also draw the projected quad
    BYTE field_0x52[2];
    SceneNode *pNode;           // 0x54
    int field_0x58;
};

// Reserves a free glow slot and copies its position, direction, and draw settings.
// FUNCTION: CMR2 0x004ae2f0
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1,
                    int sizeX, int sizeY, int billboardTexture, int layerTexture,
                    int intensity, int node, BYTE projected, int unused2,
                    int field_0x40)
{
    int i;
    GlowLight *light;

    if (g_unk0x006a2bc8 < g_unk0x006a2bcc) {
        for (i = 0; i < g_unk0x006a2bcc; i++) {
            light = &((GlowLight *)g_unk0x006a2a98)[i];
            if (light->type == 0)
                break;
        }
        if (i < g_unk0x006a2bcc) {
            light = &((GlowLight *)g_unk0x006a2a98)[i];
            if (light != NULL) {
                light->type = type;
                light->pos = *pos;
                if (dir == NULL) {
                    light->dir.x = 0;
                    light->dir.y = 0;
                    light->dir.z = 0;
                } else {
                    light->dir = *dir;
                }
                light->sizeX = sizeX;
                light->sizeY = sizeY;
                light->pTexture = (unsigned short *)billboardTexture;
                light->pLayerTexture = (Texture *)layerTexture;
                light->intensity = intensity;
                light->pNode = (SceneNode *)node;
                light->enabled = 1;
                light->projected = projected;
                light->field_0x40 = field_0x40;
                g_unk0x006a2bc8++;
                return light;
            }
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x004ae2a0
void Glow_SetPosition(GlowLight *pLight, FixVector *pPos, FixVector *pDir)
{
    if (pLight != NULL) {
        if (pPos != NULL)
            pLight->pos = *pPos;
        if (pDir != NULL)
            pLight->dir = *pDir;
    }
}

// FUNCTION: CMR2 0x004ae420
void Glow_SetLayerPlane(GlowLight *pLight, FixVector *pPoint, FixVector *pNormal, int layerIntensity)
{
    if (pLight != NULL) {
        pLight->planePoint = *pPoint;
        pLight->planeNormal = *pNormal;
        pLight->layerIntensity = layerIntensity;
    }
}

// GLOBAL: CMR2 0x006a2aa0
BillboardDef g_glowBillboard;
// Horizontal camera forward (normalised) used by the glow quads.
// GLOBAL: CMR2 0x00520ff8
FixVector g_glowForward = { 0x10000, 0, 0 };

int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
void FixMatrix_GetPosition(FixVector *pOut, FixMatrix *pM);
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);

// Draws the glow of every enabled light seen from pCamera in view `view`: a
// billboard pulled one unit toward the camera and faded by the viewing angle,
// plus the optional ground layer and projected quads.
// FUNCTION: CMR2 0x004af120
void Glow_Draw(SceneNode *pCamera, BYTE view)
{
    FixMatrix *pCamMatrix;
    GlowLight *pLight;
    FixVector camPos;
    FixVector dir;
    FixVector pos;
    FixVector nodePos;
    FixVector toLight;
    FixVector toCamera;
    BYTE mask;
    int len;
    int d;
    int m;
    int degrees;
    int fade;
    int intensity;
    int inv;
    int scale;
    int i;
    short a;

    pCamMatrix = &pCamera->world;
    FixMatrix_GetPosition(&camPos, pCamMatrix);
    mask = 1 << view;
    FixMatrix_GetForward(&g_glowForward, pCamMatrix);
    g_glowForward.y = 0;
    FIX_NORMALIZE_INTO(g_glowForward, g_glowForward);
    g_quadBasisB.x = -g_glowForward.z;
    g_quadBasisB.y = 0;
    g_quadBasisB.z = g_glowForward.x;

    for (i = 0; i < g_unk0x006a2bcc; i++) {
        pLight = &((GlowLight *)g_unk0x006a2a98)[i];
        if (pLight->type == 0 || pLight->enabled == 0 || pLight->intensity <= 0)
            continue;
        if (pLight->pNode != NULL && (mask & pLight->pNode->field_0x17c) == 0)
            continue;
        if (pLight->pNode != NULL && pLight->pNode->pParent != NULL &&
            (mask & pLight->pNode->pParent->field_0x17c) == 0)
            continue;
        if (pLight->pNode == NULL) {
            dir = pLight->dir;
            pos = pLight->pos;
        } else {
            FixMatrix_RotateVector(&dir, &pLight->dir, &pLight->pNode->world);
            FixMatrix_RotateVector(&pos, &pLight->pos, &pLight->pNode->world);
            FixMatrix_GetPosition(&nodePos, &pLight->pNode->world);
            pos.x += nodePos.x;
            pos.y += nodePos.y;
            pos.z += nodePos.z;
        }

        if (pLight->type == 2 || pLight->type == 3) {
            FixMatrix_GetPosition(&toLight, pCamMatrix);
            toLight.x = pos.x - toLight.x;
            toLight.y = pos.y - toLight.y;
            toLight.z = pos.z - toLight.z;
            // Pre-scale by the largest component so the length cannot overflow.
            if (FIX_ABS(toLight.x) > FIX_ABS(toLight.y) && FIX_ABS(toLight.x) > FIX_ABS(toLight.z))
                m = FIX_ABS(toLight.x);
            else if (FIX_ABS(toLight.y) > FIX_ABS(toLight.x) && FIX_ABS(toLight.y) > FIX_ABS(toLight.z))
                m = FIX_ABS(toLight.y);
            else
                m = FIX_ABS(toLight.z);
            if (m != 0)
                FixVecScaleRecip(&toLight, &toLight, m);
            if (toLight.x == 0 && toLight.y == 0 && toLight.z == 0)
                toLight.x = 0x10000;
            FIX_NORMALIZE_INTO(toLight, toLight);
            d = FixVecDot(&dir, &toLight);
            if (FIX_ABS(d) >= 0xfd70) {
                if (d > 0)
                    degrees = 0;
                else
                    degrees = 0xb40000;
            } else {
                a = FixAcos(d);
                degrees = (0x400 - a) * 0x1680;
            }
            if (degrees > 0x5a0000) {
                fade = FixMul(degrees - 0x5a0000, 0x3d7);
            } else {
                if (degrees >= 0x5a0000 || pLight->type != 3)
                    goto projected;
                fade = FixMul(0x5a0000 - degrees, 0x3d7);
            }
            if (fade > 0x10000)
                fade = 0x10000;
            else if (fade <= 0)
                goto projected;
        } else {
            fade = 0x10000;
        }

        intensity = FixMul(fade, pLight->intensity);
        FUN_004ae230((int *)&g_glowDef, pLight->sizeX, pLight->sizeY);
        g_glowDef.r = (BYTE)((intensity * 254) >> 16);
        g_glowDef.g = g_glowDef.r;
        g_glowDef.b = g_glowDef.r;
        g_glowDef.a = 0xff;
        g_glowDef.pos = pos;
        g_glowBillboard = g_glowDef;
        toCamera.x = camPos.x - pos.x;
        toCamera.y = camPos.y - pos.y;
        toCamera.z = camPos.z - pos.z;
        len = FixVecLength(&toCamera);
        if (len >= 0x10000) {
            inv = FixDiv(0x10000, len);
            FixVecScale(&toCamera, &toCamera, inv);
            g_glowBillboard.pos.x += toCamera.x;
            g_glowBillboard.pos.y += toCamera.y;
            g_glowBillboard.pos.z += toCamera.z;
            scale = 0x10000 - inv;
            g_glowBillboard.bottom = FixMul(g_glowBillboard.bottom, scale);
            g_glowBillboard.right = FixMul(g_glowBillboard.right, scale);
            g_glowBillboard.top = FixMul(g_glowBillboard.top, scale);
            g_glowBillboard.left = FixMul(g_glowBillboard.left, scale);
        }
        Billboard_Add(&g_glowBillboard, pLight->pTexture);
        if (pLight->layerIntensity != 0)
            Graphics_DrawLayerQuad((BYTE *)pLight, &camPos);
    projected:
        if (pLight->projected != 0)
            Graphics_DrawProjectedQuad((BYTE *)pLight, &pos, &camPos, &dir);
    }
}

// Release callback of FUN_004ae170: frees the glow table.
// FUNCTION: CMR2 0x004ae200
int Glow_FreeTable(void)
{
    if (g_unk0x006a2a98 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x006a2a98);
        g_unk0x006a2a98 = NULL;
    }
    g_unk0x006a2bcc = 0;
    return 1;
}

// FUNCTION: CMR2 0x004ae170
void FUN_004ae170(int param1)
{
    if (g_unk0x006a2a98 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x006a2a98);
        g_unk0x006a2a98 = NULL;
    }
    g_unk0x006a2a98 = CFileBuffer::AllocateLockedBuffer((param1 & 0xff) * 92);
    g_unk0x006a2bcc = (BYTE)param1;
    g_layerQuad[0].u = 0;
    g_layerQuad[0].v = 0;
    g_layerQuad[1].u = 0xfff9;
    g_layerQuad[1].v = 0;
    g_layerQuad[2].u = 0xfff9;
    g_layerQuad[2].v = 0xfff9;
    g_layerQuad[3].u = 0;
    g_layerQuad[3].v = 0xfff9;
    CGame::RegisterCallback(Glow_FreeTable, NULL);
}

// GLOBAL: CMR2 0x006a2a08
LPDIRECT3DVERTEXBUFFER7 g_quadVertexBuffer;

// Release callback of FUN_004ae0a0: releases the quad vertex buffer.
// FUNCTION: CMR2 0x004ae120
int QuadVB_Release(void)
{
    if (g_quadVertexBuffer != NULL) {
        if (g_quadVertexBuffer->Release() == 0)
            g_quadVertexBuffer = NULL;
    }
    return 1;
}


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
        &desc, &g_quadVertexBuffer, 0);
    CGame::RegisterCallback(QuadVB_Release, NULL);
}


// Finds a free entry of the 0x2384 table and initialises its six 0x130-byte
// records, or notifies the failure through FUN_004a76d0(NULL).
// Allocates a free cube-map slot (six 0x130-byte faces) and creates its
// surfaces; returns the render texture (NULL when out of memory).
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a4b10
RenderTexture *FUN_004a4b10(void)
{
    BYTE *p;
    int i;
    int j;

    for (i = 0; i < 0x14; i++) {
        if (CGraphics::m_pTextureManager->textureBuffer2[i] == NULL) {
            p = (BYTE *)CFileBuffer::AllocateLockedBuffer(0x738);
            CGraphics::m_pTextureManager->textureBuffer2[i] = (Texture *)p;
            for (j = 0; j < 6; j++) {
                BYTE *q = p + j * 0x130;

                *(unsigned short *)q = (unsigned short)i;
                *(unsigned short *)(q + 0x11c) = 0;
                *(unsigned short *)(q + 0x11e) = 0;
                *(unsigned short *)(q + 0x120) = CGraphics::m_cubeMapSize;
                *(unsigned short *)(q + 0x122) = CGraphics::m_cubeMapSize;
            }
            if (p == NULL)
                return NULL;
            CGraphics::m_unk0x0065fa28++;
            return CGraphics::CreateCubeMapSurfaces((RenderTexture *)p);
        }
    }
    return CGraphics::CreateCubeMapSurfaces(NULL);
}

// GLOBAL: CMR2 0x0067f228
int g_unk0x0067f228;



// Detail level letter ('A' best .. 'D') of each stage/texture group.
// GLOBAL: CMR2 0x0051a3d0
char g_stageQualityCodes[24] = {
    0x43, 0x43, 0x43, 0x43, 0x44, 0x44, 0x44, 0x44, 0x44, 0x43, 0x44, 0x43, 0x44, 0x44, 0x44, 0x44,
    0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
};
extern BYTE g_unk0x00542630[];

// Raises the detail levels when the hardware allows it (texture memory, caps).
// FUNCTION: CMR2 0x00457c50
void FUN_00457c50(void)
{
    if (CGameInfo::FUN_00406410(0x10) != 0 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0) {
        g_stageQualityCodes[0] = 'A';
        g_stageQualityCodes[1] = 'A';
        g_stageQualityCodes[3] = 'A';
        g_stageQualityCodes[2] = 'A';
        g_stageQualityCodes[4] = 'C';
        g_stageQualityCodes[5] = 'C';
        g_stageQualityCodes[6] = 'C';
        g_stageQualityCodes[7] = 'A';
        g_stageQualityCodes[8] = 'A';
        g_stageQualityCodes[13] = 'A';
        g_stageQualityCodes[14] = 'A';
        g_stageQualityCodes[15] = 'A';
        g_stageQualityCodes[16] = 'A';
        g_stageQualityCodes[17] = 'A';
        g_stageQualityCodes[18] = 'A';
        g_stageQualityCodes[19] = 'A';
        g_unk0x00542630[0x394] = 'A';
        g_unk0x00542630[0x395] = 'A';
        g_stageQualityCodes[11] = 'A';
        g_stageQualityCodes[12] = 'A';
        return;
    }
    switch (CGameInfo::FUN_00405d10()) {
    case 2:
        g_stageQualityCodes[0] = 'C';
        g_stageQualityCodes[1] = 'C';
        g_stageQualityCodes[3] = 'D';
        g_stageQualityCodes[2] = 'D';
        g_stageQualityCodes[4] = 'D';
        g_stageQualityCodes[5] = 'E';
        g_stageQualityCodes[6] = 'F';
        g_stageQualityCodes[7] = 'D';
        g_stageQualityCodes[8] = 'D';
        g_stageQualityCodes[13] = 'D';
        g_stageQualityCodes[14] = 'D';
        g_stageQualityCodes[15] = 'D';
        g_stageQualityCodes[16] = 'D';
        g_stageQualityCodes[17] = 'D';
        g_stageQualityCodes[18] = 'E';
        g_stageQualityCodes[19] = 'F';
        g_unk0x00542630[0x394] = 'D';
        g_unk0x00542630[0x395] = 'D';
        g_stageQualityCodes[11] = 'C';
        g_stageQualityCodes[12] = 'D';
        break;
    case 0:
        g_stageQualityCodes[0] = 'A';
        g_stageQualityCodes[1] = 'A';
        g_stageQualityCodes[4] = 'A';
        g_stageQualityCodes[5] = 'C';
        g_stageQualityCodes[6] = 'D';
        g_stageQualityCodes[7] = 'A';
        g_stageQualityCodes[8] = 'C';
        g_stageQualityCodes[13] = 'D';
        g_stageQualityCodes[14] = 'D';
        g_stageQualityCodes[3] = 'C';
        g_stageQualityCodes[2] = 'A';
        g_stageQualityCodes[17] = 'D';
        g_stageQualityCodes[18] = 'E';
        g_stageQualityCodes[19] = 'F';
        g_unk0x00542630[0x394] = 'A';
        g_unk0x00542630[0x395] = 'C';
        g_stageQualityCodes[11] = 'A';
        g_stageQualityCodes[12] = 'D';
        break;
    }
}


// GLOBAL: CMR2 0x0052109c
float g_unk0x0052109c = 1.0f;
// GLOBAL: CMR2 0x005210a0
float g_frameScale = 25.0f;
// GLOBAL: CMR2 0x005210a4
float g_averageFps = 25.0f;
// GLOBAL: CMR2 0x005210a8
float g_fps = 25.0f;
extern int g_unk0x005210ac;
#define g_fpsWarmup g_unk0x005210ac
// GLOBAL: CMR2 0x005210b0
float g_frameScale2 = 25.0f;
// GLOBAL: CMR2 0x006dd9a8
int g_averageFpsDone;
// GLOBAL: CMR2 0x006dd9b0
int g_frameCount;
// GLOBAL: CMR2 0x006dd9b4
int g_framesThisSecond;
// GLOBAL: CMR2 0x006dd9b8
unsigned int g_lastSecond;
// GLOBAL: CMR2 0x006dd9bc
unsigned short g_unk0x006dd9bc[2000];

// Frame timing: average fps after a 3 s warm-up, fps of the last second and
// the time scale of the current frame (1000 / frame time in ms).
// FUNCTION: CMR2 0x004b21e0
void FUN_004b21e0(void)
{
    static unsigned int s_start = CMain::GetFrameTime();
    static unsigned int s_now = CMain::GetFrameTime();
    static unsigned int s_prev = CMain::GetFrameTime() - 40;
    static unsigned int s_secondStart = CMain::GetFrameTime();
    unsigned int elapsed;

    s_prev = s_now;
    s_now = CMain::GetFrameTime();
    g_frameCount++;
    g_framesThisSecond++;
    if (g_fpsWarmup != 0) {
        if (s_now - s_start > 3000) {
            g_fpsWarmup = 0;
            g_averageFpsDone = 0;
            s_start = s_now;
            g_frameCount = 0;
            g_averageFps = 0.0f;
        }
    } else {
        if (g_averageFpsDone == 0 && s_now - s_start > 3000) {
            g_averageFpsDone = 1;
            g_averageFps = (float)g_frameCount * 1000.0f / (float)(s_now - s_start);
        }
        if (g_lastSecond != s_now / 1000) {
            g_lastSecond = s_now / 1000;
            elapsed = s_now - s_secondStart;
            s_secondStart = s_now;
            g_fps = (float)g_framesThisSecond * 1000.0f / (float)elapsed;
            g_framesThisSecond = 0;
        }
    }
    g_frameScale = g_frameScale2 = 1000.0f / (float)(s_now - s_prev);
}

// FUNCTION: CMR2 0x004b23a0
float FUN_004b23a0(void)
{
    return g_frameScale;
}

// FUNCTION: CMR2 0x004b23b0
void FUN_004b23b0(float value)
{
    g_unk0x0052109c = value;
}

#include "Particle.h"

extern double g_minus65536;

// GLOBAL: CMR2 0x005112f0
float g_oneOverRandMax = 1.0f / RAND_MAX;
// GLOBAL: CMR2 0x006a2cd0
int g_particleTypeCount;
// GLOBAL: CMR2 0x006a2cd4
volatile int g_particleCount;
// GLOBAL: CMR2 0x006a2cd8
ParticleType *g_particleTypes;
// GLOBAL: CMR2 0x006a2cdc
Particle *g_particles;
// GLOBAL: CMR2 0x006a2ce0
ParticleType *g_pEditParticleType;
// GLOBAL: CMR2 0x006a2ce4
int g_nextParticle;

// Selects the particle type to edit and resets it to defaults.
// FUNCTION: CMR2 0x004af940
void ParticleEdit_Select(int index)
{
    if (index >= 0 && index < g_particleTypeCount) {
        g_pEditParticleType = &g_particleTypes[index];
        g_pEditParticleType->lifetime = 0;
        g_pEditParticleType->gravity = 0;
        g_pEditParticleType->drag = 0;
        g_pEditParticleType->bounce = 0;
        g_pEditParticleType->friction = 0;
        g_pEditParticleType->type = 0x80;
        g_pEditParticleType->alphaEnd = 0;
        g_pEditParticleType->alphaStep = 0;
        g_pEditParticleType->colour[0] = 0xff;
        g_pEditParticleType->colour[1] = 0xff;
        g_pEditParticleType->colour[2] = 0xff;
        g_pEditParticleType->field0x38 = 0;
        g_pEditParticleType->field0x4c = 0;
        g_pEditParticleType->field0x50 = 0;
        g_pEditParticleType->field0x54 = 0;
        g_pEditParticleType->field0x58 = 0;
        g_pEditParticleType->postUpdate = NULL;
        g_pEditParticleType->callback = NULL;
        g_pEditParticleType->field0x6c = 0;
        g_pEditParticleType->update = NULL;
        g_pEditParticleType->field0x5c = 0;
        g_pEditParticleType->flags &= 0xfe;
        g_pEditParticleType->flags &= 0xfd;
        g_pEditParticleType->flags &= 0xfb;
        g_pEditParticleType->flags &= 0xf7;
        g_pEditParticleType->flags &= 0xef;
        g_pEditParticleType->flags &= 0xdf;
        g_pEditParticleType->flags &= 0xbf;
        g_pEditParticleType->flags &= 0x7f;
        g_pEditParticleType->directionFlags &= 0xfe;
        g_pEditParticleType->directionFlags &= 0xfd;
        g_pEditParticleType->directionFlags &= 0xfb;
        g_pEditParticleType->directionFlags &= 0xf7;
        g_pEditParticleType->spread.x = 0;
        g_pEditParticleType->spread.y = 0;
        g_pEditParticleType->spread.z = 0;
        g_pEditParticleType->field0x38 = 0;
        return;
    }
    g_pEditParticleType = NULL;
}

// Particle type editor: setters for g_pEditParticleType.

// Copies a template particle type (flag 1) into the type being edited.
// FUNCTION: CMR2 0x004afb20
void ParticleEdit_CopyTemplate(int index)
{
    ParticleType *pSrc;

    if (g_pEditParticleType != NULL && index >= 0 && index < g_particleTypeCount) {
        pSrc = &g_particleTypes[index];
        if (pSrc->flags & 1)
            *g_pEditParticleType = *pSrc;
    }
}

// FUNCTION: CMR2 0x004afb70
void ParticleEdit_SetTextureParams(int a, int b, int c, int d, int e)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->field0x38 = a;
        g_pEditParticleType->field0x3c = b;
        g_pEditParticleType->field0x40 = c;
        g_pEditParticleType->field0x44 = d;
        g_pEditParticleType->field0x48 = e;
    }
}

// FUNCTION: CMR2 0x004afbc0
void ParticleEdit_SetExtendedParams(int a, int b, int c, int d, BYTE flag4, BYTE flag8, int e, int f, int g, int h)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->field0x4c = a;
        g_pEditParticleType->field0x50 = b;
        g_pEditParticleType->field0x54 = c;
        g_pEditParticleType->field0x58 = d;
        g_pEditParticleType->directionFlags = (flag4 & 1) << 2 | g_pEditParticleType->directionFlags & 0xfb;
        g_pEditParticleType->directionFlags = (flag8 & 1) << 3 | g_pEditParticleType->directionFlags & 0xf7;
        g_pEditParticleType->field0x3c = e;
        g_pEditParticleType->field0x40 = f;
        g_pEditParticleType->field0x44 = g;
        g_pEditParticleType->field0x48 = h;
    }
}

// FUNCTION: CMR2 0x004afc70
void ParticleEdit_SetMotion(int lifetime, int gravity, int drag, int bounce, int friction, char bounces,
                            BYTE killBelowFloor)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->lifetime = lifetime;
        g_pEditParticleType->gravity = gravity;
        g_pEditParticleType->drag = drag;
        g_pEditParticleType->bounce = bounce;
        g_pEditParticleType->friction = friction;
        g_pEditParticleType->flags = g_pEditParticleType->flags & 0x7f | bounces << 7;
        g_pEditParticleType->flags = (killBelowFloor & 1) << 1 | g_pEditParticleType->flags & 0xfd;
    }
}

// FUNCTION: CMR2 0x004afcf0
void ParticleEdit_SetAlphaRamp(BYTE start, BYTE end, char step, BYTE killAtEnd)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->type = start;
        g_pEditParticleType->alphaEnd = end;
        g_pEditParticleType->alphaStep = step;
        g_pEditParticleType->flags = ((step != 0) & 1) << 6 | g_pEditParticleType->flags & 0xbf;
        g_pEditParticleType->flags = (killAtEnd & 1) << 2 | g_pEditParticleType->flags & 0xfb;
    }
}

// FUNCTION: CMR2 0x004afd60
void ParticleEdit_SetColour(BYTE r, BYTE g, BYTE b, BYTE flag)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->directionFlags = (flag & 1) << 1 | g_pEditParticleType->directionFlags & 0xfd;
        g_pEditParticleType->colour[0] = r;
        g_pEditParticleType->colour[1] = g;
        g_pEditParticleType->colour[2] = b;
    }
}

// FUNCTION: CMR2 0x004afdb0
void ParticleEdit_SetSizeRamp(int size, int target, int step)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->flags &= 0xef;
        g_pEditParticleType->size = size;
        g_pEditParticleType->sizeVariation = target;
        g_pEditParticleType->sizeStep = step;
        g_pEditParticleType->flags = ((step != 0) & 1) << 3 | g_pEditParticleType->flags & 0xf7;
    }
}

// FUNCTION: CMR2 0x004afe10
void ParticleEdit_SetSizeRange(int min, int max)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->flags &= 0xf7;
        g_pEditParticleType->sizeStep = 0;
        if (min < max) {
            g_pEditParticleType->size = min;
            g_pEditParticleType->sizeVariation = max - min;
            g_pEditParticleType->flags |= 0x10;
            return;
        }
        g_pEditParticleType->size = min;
        g_pEditParticleType->sizeVariation = 0;
        g_pEditParticleType->flags &= 0xef;
    }
}

// FUNCTION: CMR2 0x004afec0
void ParticleEdit_SetSpread(int x, int y, int z, BYTE flag)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->spread.x = x;
        g_pEditParticleType->spread.y = y;
        g_pEditParticleType->spread.z = z;
        g_pEditParticleType->directionFlags = (g_pEditParticleType->directionFlags ^ flag) & 1 ^
                                              g_pEditParticleType->directionFlags;
    }
}

// FUNCTION: CMR2 0x004aff10
void ParticleEdit_SetCallbacks(int field0x5c, void (*update)(void *, ParticleType *, int),
                               void (*postUpdate)(void *, ParticleType *, int),
                               void (*callback)(void *, ParticleType *, int), int field0x6c)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->postUpdate = postUpdate;
        g_pEditParticleType->callback = callback;
        g_pEditParticleType->field0x6c = field0x6c;
        g_pEditParticleType->update = update;
        g_pEditParticleType->field0x5c = field0x5c;
    }
}

// FUNCTION: CMR2 0x004afe80
void FUN_004afe80(short param1)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->field0x2c = param1;
        g_pEditParticleType->flags = ((param1 != 0) & 1) << 5 | g_pEditParticleType->flags & 0xdf;
    }
}

// FUNCTION: CMR2 0x004aff60
void FUN_004aff60(void)
{
    if (g_pEditParticleType != NULL) {
        g_pEditParticleType->flags |= 1;
        g_pEditParticleType = NULL;
    }
}

// FUNCTION: CMR2 0x004b0100
void Particle_Kill(Particle *p)
{
    p->active = 0;
}

// FUNCTION: CMR2 0x004aff80
void Particle_KillAll(void)
{
    int i;
    Particle *p = g_particles;

    for (i = 0; i < g_particleCount; i++, p++) {
        if (p->active)
            Particle_Kill(p);
    }
}

// FUNCTION: CMR2 0x004affb0
void FUN_004affb0(void)
{
    int i;
    BYTE *p;

    i = 0;
    if (g_particleCount > 0) {
        p = &g_particles->active;
        do {
            *p = 0;
            i++;
            p += sizeof(Particle);
        } while (i < g_particleCount);
    }
}

// FUNCTION: CMR2 0x004affe0
void FUN_004affe0(void)
{
    int i;
    BYTE *p;

    i = 0;
    if (g_particleTypeCount > 0) {
        p = &g_particleTypes->flags;
        do {
            i++;
            *p &= ~1;
            p += sizeof(ParticleType);
        } while (i < g_particleTypeCount);
    }
}

void FUN_004affb0(void);

// Release callback of the particle system.
// FUNCTION: CMR2 0x004b0010
BYTE Particle_Shutdown(void)
{
    FUN_004affe0();
    FUN_004affb0();
    if (g_particleTypes != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_particleTypes);
        g_particleTypes = NULL;
    }
    if (g_particles != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_particles);
        g_particles = NULL;
    }
    g_particleTypeCount = 0;
    g_particleCount = 0;
    return 1;
}

// Allocates the particle type table and the particle pool.
// FUNCTION: CMR2 0x004b0060
void Particle_Init(int typeCount, int particleCount)
{
    g_particleTypes = (ParticleType *)CFileBuffer::AllocateLockedBuffer(typeCount * sizeof(ParticleType));
    g_particles = (Particle *)CFileBuffer::AllocateLockedBuffer(particleCount * sizeof(Particle));
    if (g_particleTypes != NULL) {
        if (g_particles != NULL) {
            g_particleTypeCount = typeCount;
            g_particleCount = particleCount;
            FUN_004affe0();
            FUN_004affb0();
            CGame::RegisterCallback(Particle_Shutdown, NULL);
            return;
        }
        CFileBuffer::FreeGenericFileBuffer(g_particleTypes);
        g_particleTypes = NULL;
    }
    if (g_particles != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_particles);
        g_particles = NULL;
    }
    g_particleTypeCount = 0;
    g_particleCount = 0;
}

// FUNCTION: CMR2 0x004b1140
void FUN_004b1140(int count)
{
    g_particleCount = count;
}

// Wind the particle drag pulls toward.
// GLOBAL: CMR2 0x006a2ce8
FixVector g_particleWind;

// Advances every active particle by one step: age, drag, gravity, alpha, size
// and angle ramps, floor kill or bounce (unless the type has its own update).
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b0110
void Particle_UpdateAll(int param)
{
    Particle *p;
    ParticleType *pType;
    FixVector wind;
    FixVector rel;
    int keep;
    int a;
    int size;
    short angle;
    int i;

    p = g_particles;
    for (i = 0; i < g_particleCount; i++, p++) {
        if (p->active == 0)
            continue;
        pType = p->pType;
        p->vector0x10 = p->vector0x28;
        p->age -= 0x10000;
        if (p->age <= 0) {
            Particle_Kill(p);
            continue;
        }
        if (pType->update == NULL) {
            if (pType->drag > 0) {
                keep = 0x10000 - FixMul(pType->drag, 0x290);
                FixVecScale(&wind, &g_particleWind, 0x1999);
                rel.x = p->position.x - wind.x;
                rel.y = p->position.y - wind.y;
                rel.z = p->position.z - wind.z;
                FixVecScale(&rel, &rel, keep);
                p->position.x = rel.x + wind.x;
                p->position.y = rel.y + wind.y;
                p->position.z = rel.z + wind.z;
            }
            p->position.y -= pType->gravity;
            p->vector0x1c.x += p->position.x;
            p->vector0x1c.y += p->position.y + pType->gravity / 2;
            p->vector0x1c.z += p->position.z;
            if (pType->flags & 0x40) {
                p->type0x52 = p->type0x56;
                a = p->type0x56;
                if (pType->type < pType->alphaEnd) {
                    a += pType->alphaStep;
                    if (a > pType->alphaEnd)
                        a = pType->alphaEnd;
                } else if (pType->alphaEnd < pType->type) {
                    a -= pType->alphaStep;
                    if (a < pType->alphaEnd)
                        a = pType->alphaEnd;
                }
                p->type0x56 = (BYTE)a;
                if ((pType->flags & 4) && a == pType->alphaEnd) {
                    Particle_Kill(p);
                    continue;
                }
            }
            if (pType->flags & 8) {
                size = p->size;
                if (pType->size < pType->sizeVariation) {
                    size += pType->sizeStep;
                    if (size > pType->sizeVariation)
                        size = pType->sizeVariation;
                } else if (pType->sizeVariation < pType->size) {
                    size -= pType->sizeStep;
                    if (size < pType->sizeVariation)
                        size = pType->sizeVariation;
                }
                p->size = size;
            }
            if (pType->flags & 0x20) {
                angle = p->field0x50 + pType->field0x2c;
                p->field0x50 = angle;
                if (angle > 0x1000)
                    p->field0x50 = angle - 0x1000;
                else if (angle < 0)
                    p->field0x50 = angle + 0x1000;
            }
            if ((pType->flags & 2) && p->vector0x1c.y < p->field0x48) {
                Particle_Kill(p);
                continue;
            }
            if (pType->flags & 0x80) {
                keep = 0x10000 - pType->friction;
                if (p->field0x58 == 0) {
                    if (p->vector0x1c.y < p->field0x48) {
                        p->position.y = -FixMul(p->position.y, pType->bounce);
                        p->vector0x1c.y = p->field0x48;
                        p->position.x = FixMul(p->position.x, keep);
                        p->position.z = FixMul(p->position.z, keep);
                        if (p->position.y < 0x1999) {
                            p->position.y = 0;
                            p->field0x58 = 1;
                        }
                    }
                } else {
                    p->vector0x1c.y = p->field0x48;
                    p->position.x = FixMul(p->position.x, keep);
                    p->position.z = FixMul(p->position.z, keep);
                }
            }
            p->vector0x28 = p->vector0x1c;
        } else {
            pType->update(p, pType, param);
        }
        if (pType->postUpdate != NULL)
            pType->postUpdate(p, pType, param);
    }
}

// Interpolates every active particle between its last two positions (and
// alpha values) by t (0..1) for drawing.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b06a0
void Particle_Interpolate(int t)
{
    Particle *p;
    FixVector d;
    int i;

    p = g_particles;
    for (i = 0; i < g_particleCount; i++, p++) {
        if (p->active != 0) {
            d.x = p->vector0x28.x - p->vector0x10.x;
            d.y = p->vector0x28.y - p->vector0x10.y;
            d.z = p->vector0x28.z - p->vector0x10.z;
            FixVecScale(&d, &d, t);
            p->vector0x1c.x = p->vector0x10.x + d.x;
            p->vector0x1c.y = p->vector0x10.y + d.y;
            p->vector0x1c.z = p->vector0x10.z + d.z;
            p->type0x53 = (BYTE)((p->type0x52 * 0x10000 + FixMul((p->type0x56 - p->type0x52) * 0x10000, t)) >> 16);
        }
    }
}

// Queues a billboard for every active particle visible in view `view`
// (animated texture frames, size scaling, spin, lighting), or calls the
// type's own draw callback.
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b0480
void Particle_DrawAll(int param, BYTE view)
{
    Particle *p;
    ParticleType *pType;
    BillboardDef def;
    int frame;
    int elapsed;
    int texture;
    int *pFrames;
    int i;

    def.flags &= 0xfe;
    p = g_particles;
    for (i = 0; i < g_particleCount; i++, p++) {
        if (p->active == 0 || (p->field0x55 & (1 << view)) == 0)
            continue;
        pType = p->pType;
        if (pType->field0x5c != 0) {
            ((void (*)(void *, ParticleType *, int))pType->field0x5c)(p, pType, param);
            continue;
        }
        pFrames = (int *)pType->field0x4c;
        if (pFrames == NULL) {
            texture = p->field0x60;
        } else {
            frame = 0;
            elapsed = pType->lifetime - p->age;
            if (pType->field0x54 < elapsed && pType->field0x58 > 0) {
                frame = FixDiv(elapsed - pType->field0x54, pType->field0x58) >> 16;
                if (pType->directionFlags & 8)
                    frame += ((unsigned int)p & 0xffff) % (unsigned int)pType->field0x50;
                if (frame >= pType->field0x50 && (pType->directionFlags & 4) == 0) {
                    texture = pFrames[pType->field0x50 - 1];
                    goto draw;
                }
                frame %= pType->field0x50;
            }
            texture = pFrames[frame];
        }
    draw:
        if (texture == 0)
            continue;
        if (p->size == 0x10000) {
            def.top = pType->field0x3c;
            def.left = pType->field0x40;
            def.bottom = pType->field0x44;
            def.right = pType->field0x48;
        } else {
            def.top = FixMul(pType->field0x3c, p->size);
            def.left = FixMul(pType->field0x40, p->size);
            def.bottom = FixMul(pType->field0x44, p->size);
            def.right = FixMul(pType->field0x48, p->size);
        }
        if ((pType->flags & 0x20) == 0)
            def.field_0x20 = 0;
        else
            def.field_0x20 = p->field0x50;
        def.pos = p->vector0x1c;
        if (p->field0x40 != 0) {
            def.pos.x += *(int *)(p->field0x40 + 0x30);
            def.pos.y += *(int *)(p->field0x40 + 0x34);
            def.pos.z += *(int *)(p->field0x40 + 0x38);
        }
        def.flags ^= (pType->directionFlags ^ def.flags) & 2;
        def.r = p->colour[0];
        def.g = p->colour[1];
        def.b = p->colour[2];
        def.a = p->type0x53;
        def.shade = p->field0x54;
        Billboard_Add(&def, (unsigned short *)texture);
    }
}

// Allocates and initialises a particle, optionally orienting its random spread
// around the supplied position vector.
// FUNCTION: CMR2 0x004b07a0
void Particle_Spawn(int typeIndex, FixVector *pSource, FixVector *pPosition,
                    int field0x48, int field0x40, BYTE *pColour,
                    BYTE field0x54, int callbackParam, BYTE field0x55)
{
    ParticleType *pType = NULL;
    bool hasBasis;
    FixVector right;
    FixVector direction;
    FixVector up;
    int length;
    int dot;
    int i;
    int best;
    int selected;
    Particle *pParticle;
    int randomX;
    int randomY;
    int randomZ;
    BYTE directionMask = 1;

    if (typeIndex >= 0 && typeIndex < g_particleTypeCount)
        pType = &g_particleTypes[typeIndex];
    if ((pType->flags & directionMask) == 0)
        return;
    hasBasis = false;

    if ((pType->directionFlags & directionMask) != 0) {
        int x = pPosition->x;
        if (x < 0)
            x = -x;
        if (x > 0xa0) {
            int z = pPosition->z;
            if (z < 0)
                z = -z;
            if (z > 0xa0) {
                length = FixVecLength(pPosition);
                if (length == 0) {
                    direction.x = 0;
                    direction.y = 0;
                    direction.z = 0;
                } else {
                    FixVecScaleRecip(&direction, pPosition, length);
                }

                if (direction.y > 0x8000) {
                    up.x = direction.x;
                    up.y = 0;
                    up.z = direction.z;
                    length = FixVecLength(&up);
                    if (length == 0) {
                        up.x = 0;
                        up.y = 0;
                        up.z = 0;
                    } else {
                        FixVecScaleRecip(&up, &up, length);
                    }
                } else {
                    up.x = 0;
                    up.y = 0x10000;
                    up.z = 0;
                }

                dot = FixVecDot(&direction, &up);
                up.x -= FixMul(direction.x, dot);
                up.y -= FixMul(direction.y, dot);
                up.z -= FixMul(direction.z, dot);
                length = FixVecLength(&up);
                if (length == 0) {
                    up.x = 0;
                    up.y = 0;
                    up.z = 0;
                } else {
                    FixVecScaleRecip(&up, &up, length);
                }

                FixVecCross(&right, &direction, &up);
                length = FixVecLength(&right);
                if (length == 0) {
                    right.x = 0;
                    right.y = 0;
                    right.z = 0;
                } else {
                    FixVecScaleRecip(&right, &right, length);
                }
                hasBasis = true;
            }
        }
    }

    best = 0;
    selected = g_nextParticle;
    pParticle = &g_particles[g_nextParticle];
    i = 0;
    if (g_particleCount > 0) {
        do {
            if (g_nextParticle >= g_particleCount) {
                g_nextParticle = 0;
                pParticle = g_particles;
            }
            if (pParticle->active == 0) {
                if (pParticle != NULL)
                    goto particleFound;
                break;
            }
            if (best < pParticle->pType->lifetime - pParticle->age) {
                best = pParticle->pType->lifetime - pParticle->age;
                selected = g_nextParticle;
            }
            i++;
            pParticle++;
            g_nextParticle++;
        } while (i < g_particleCount);
    }
    pParticle = &g_particles[selected];

particleFound:

    randomX = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);
    randomY = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);
    randomZ = -0x8000 - (int)(__int64)((float)rand() * g_oneOverRandMax * g_minus65536);

    pParticle->position.x = pPosition->x;
    pParticle->position.y = pPosition->y;
    pParticle->position.z = pPosition->z;
    {
        FixVector *pVector = &pParticle->sourceVector;
        *pVector = *pSource;
        pParticle->vector0x10 = *pVector;
        pParticle->vector0x1c = *pVector;
        pParticle->vector0x28 = *pVector;
    }
    pParticle->field0x48 = field0x48;
    pParticle->field0x40 = field0x40;

    if ((pType->directionFlags & 1) != 0) {
        if (hasBasis) {
            if (pType->spread.x != 0) {
                FixVecScale(&right, &right, FixMul(randomX, pType->spread.x));
                pParticle->position.x += right.x;
                pParticle->position.y += right.y;
                pParticle->position.z += right.z;
            }
            if (pType->spread.y != 0) {
                FixVecScale(&up, &up, FixMul(randomY, pType->spread.y));
                pParticle->position.x += up.x;
                pParticle->position.y += up.y;
                pParticle->position.z += up.z;
            }
            if (pType->spread.z != 0) {
                FixVecScale(&direction, &direction, FixMul(randomZ, pType->spread.z));
                pParticle->position.x += direction.x;
                pParticle->position.y += direction.y;
                pParticle->position.z += direction.z;
            }
        } else {
            pParticle->position.x += FixMul(randomX, pType->spread.x);
            pParticle->position.y += FixMul(randomZ, pType->spread.z);
            pParticle->position.z += FixMul(randomZ, pType->spread.y);
        }
    } else {
        pParticle->position.x += FixMul(randomX, pType->spread.x);
        pParticle->position.y += FixMul(randomY, pType->spread.y);
        pParticle->position.z += FixMul(randomZ, pType->spread.z);
    }

    pParticle->pType = pType;
    pParticle->age = pType->lifetime;
    {
        BYTE type = pType->type;
        pParticle->type0x56 = type;
        pParticle->type0x53 = type;
        pParticle->type0x52 = type;
    }
    if ((pType->flags & 0x10) != 0) {
        int sizeVariation = pType->sizeVariation;
        int variation = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        pParticle->size = pType->size + FixMul(sizeVariation, variation);
    } else {
        pParticle->size = pType->size;
    }
    {
        int zero = 0;
        if (pColour != (BYTE *)zero) {
            pParticle->colour[0] = pColour[0];
            pParticle->colour[1] = pColour[1];
            pParticle->colour[2] = pColour[2];
        } else {
            pParticle->colour[0] = pType->colour[0];
            pParticle->colour[1] = pType->colour[1];
            pParticle->colour[2] = pType->colour[2];
        }
        if ((pType->directionFlags & 2) != 0)
            pParticle->field0x54 = field0x54;
        pParticle->field0x60 = pType->field0x38;
        pParticle->field0x55 = field0x55;
        pParticle->active = 1;
        pParticle->field0x58 = (BYTE)zero;
        pParticle->field0x50 = (short)zero;
        if (pType->callback != (void (*)(void *, ParticleType *, int))zero)
            pType->callback(pParticle, pType, callbackParam);
    }
}

// GLOBAL: CMR2 0x006dfe10
int *g_triangleVertexHeights;

// Interpolates the height of a point on a triangle whose vertex heights are stored separately.
// FUNCTION: CMR2 0x004b6340
int Graphics_GetTriangleHeight(unsigned short *pHeightIndices, FixVector *pVertices, FixVector *pPosition)
{
    if (g_triangleVertexHeights != 0) {
        FixVector vertices[3];
        FixVector edge1;
        FixVector edge2;
        FixVector normal;

        vertices[0] = pVertices[0];
        vertices[1] = pVertices[1];
        vertices[2] = pVertices[2];
        vertices[0].y = g_triangleVertexHeights[pHeightIndices[0]];
        vertices[1].y = g_triangleVertexHeights[pHeightIndices[1]];
        vertices[2].y = g_triangleVertexHeights[pHeightIndices[2]];

        edge1.x = vertices[1].x - vertices[0].x;
        edge1.y = vertices[1].y - vertices[0].y;
        edge1.z = vertices[1].z - vertices[0].z;
        edge2.x = vertices[2].x - vertices[0].x;
        edge2.y = vertices[2].y - vertices[0].y;
        edge2.z = vertices[2].z - vertices[0].z;

        int length = FixVecLength(&edge1);
        if (length == 0) {
            edge1.x = 0;
            edge1.y = 0;
            edge1.z = 0;
        } else {
            FixVecScaleRecip(&edge1, &edge1, length);
        }

        length = FixVecLength(&edge2);
        if (length == 0) {
            edge2.x = 0;
            edge2.y = 0;
            edge2.z = 0;
        } else {
            FixVecScaleRecip(&edge2, &edge2, length);
        }

        FixVecCross(&normal, &edge1, &edge2);
        length = FixVecLength(&normal);
        if (length == 0) {
            normal.x = 0;
            normal.y = 0;
            normal.z = 0;
        } else {
            FixVecScaleRecip(&normal, &normal, length);
        }

        int planeDistance = FixVecDot(&vertices[0], &normal);
        if (normal.y != 0) {
            return FixDiv(planeDistance - FixMul(pPosition->x, normal.x) -
                              FixMul(pPosition->z, normal.z),
                          normal.y);
        }
    }

    return -65470464;
}

int CGraphics::m_unk0x00520b1c = 1;
int CGraphics::m_unk0x00520b20 = 1;

// GLOBAL: CMR2 0x00520b18
int g_unk0x00520b18 = -1;

// FUNCTION: CMR2 0x004a3dc0
void FUN_004a3dc0(int param1)
{
    CGraphics::m_unk0x00520b14 = param1;
}

// FUNCTION: CMR2 0x004a3dd0
void FUN_004a3dd0(void)
{
    g_unk0x00520b18 = -1;
}

// FUNCTION: CMR2 0x004a3de0
void CGraphics::FUN_004a3de0(void)
{
    m_unk0x00520b1c = 1;
    m_unk0x00520b20 = 1;
}
int CGraphics::m_unk0x00520b28 = -1;
unsigned int CGraphics::m_unk0x0065fa24;
int CGraphics::m_unk0x0065fa38;

// Sets render states 0x13/0x14 when the pair changes.
// FUNCTION: CMR2 0x004a3e40
void CGraphics::FUN_004a3e40(int param1, int param2)
{
    if (m_unk0x00520b1c == param1 && m_unk0x00520b20 == param2)
        return;
    m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x13, param1);
    m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x14, param2);
    m_unk0x00520b1c = param1;
    m_unk0x00520b20 = param2;
}

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Creates a texture from file data already in memory (DDS or TGA).
// FUNCTION: CMR2 0x004a48c0
Texture *CGraphics::FUN_004a48c0(char *name, void *pData, unsigned int flags)
{
    Texture *pTexture;
    BOOL isDDS;
    int i;

    pTexture = NULL;
    isDDS = FALSE;
    // The original passes the count as memset's fill value and 0 as the count:
    // the call is a no-op (kept so the code matches the original).
    if (CGraphics::m_textureCount == 0)
        memset(m_pTextureManager->textureBuffer, 0x98000, 0);
    if (*(DWORD *)pData == 0x20534444)
        isDDS = TRUE;
    for (i = 0; i < 0x800; i++) {
        if (m_pTextureManager->textureBuffer[i] == NULL) {
            m_pTextureManager->textureBuffer[i] = (Texture *)CFileBuffer::AllocateLockedBuffer(0x130);
            pTexture = m_pTextureManager->textureBuffer[i];
            pTexture->textureId = i;
            FUN_004a3e20((Unk0x004a3e20 *)pTexture, 0);
            if (pTexture == NULL)
                return NULL;
            m_textureCount++;
            break;
        }
    }
    if (isDDS)
        strncpy(name + strlen(name) - 4, m_ddsExtension, 4);
    sprintf(pTexture->name, name);
    pTexture->flags |= flags | 0x1000;
    if (isDDS)
        return LoadDDSTexture((DDSFile *)pData, pTexture);
    return LoadTGATexture((BYTE *)pData, pTexture);
}

// Loads a texture by file name, trying the .DDS file first and the .TGA
// file next.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a49c0
Texture *CGraphics::FUN_004a49c0(char *name, unsigned int flags)
{
    Texture *pTexture;
    void *pData;
    BOOL isDDS;
    char *pExtension;
    int i;

    pTexture = NULL;
    isDDS = FALSE;
    pExtension = name + strlen(name) - 4;
    strncpy(pExtension, m_ddsExtension, 4);
    pData = CFileBuffer::GetGenericFileBuffer(name, FALSE);
    if (pData != NULL) {
        isDDS = TRUE;
    } else {
        strncpy(pExtension, m_tgaExtension, 4);
        pData = CFileBuffer::GetGenericFileBuffer(name, FALSE);
    }
    for (i = 0; i < 0x800; i++) {
        if (m_pTextureManager->textureBuffer[i] == NULL) {
            m_pTextureManager->textureBuffer[i] = (Texture *)CFileBuffer::AllocateLockedBuffer(0x130);
            pTexture = m_pTextureManager->textureBuffer[i];
            pTexture->textureId = i;
            pTexture->pArchive = NULL;
            FUN_004a3e20((Unk0x004a3e20 *)pTexture, 0);
            if (pTexture == NULL)
                return NULL;
            m_textureCount++;
            break;
        }
    }
    sprintf(pTexture->name, name);
    pTexture->flags |= flags;
    if ((pTexture->flags & 0x20) && (g_pGraphics->field913_0x3bc & 0x10))
        return LoadTGABumpMap((BYTE *)pData, pTexture);
    if (isDDS)
        return LoadDDSTexture((DDSFile *)pData, pTexture);
    return LoadTGATexture((BYTE *)pData, pTexture);
}

// Converts a TGA image into a texture: the pixels (read through
// FUN_004a60d0) are packed into a system memory surface of the texture
// format, which is then copied (or turned into a bump map) into the texture.
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a6710
Texture *CGraphics::LoadTGATexture(BYTE *pTGA, Texture *pTexture)
{
    TGAImageInfo *pInfo;
    DDSURFACEDESC2 lockDesc;
    DDSURFACEDESC2 createDesc;
    Texture tmpTexture;
    TextureFormat *pFormat;
    unsigned int width;
    unsigned int height;
    unsigned int x;
    unsigned int y;
    unsigned int rMask;
    unsigned int gMask;
    unsigned int bMask;
    unsigned int aMask;
    unsigned int mask;
    unsigned short rShift;
    unsigned short gShift;
    unsigned short bShift;
    unsigned short aShift;
    short rDepth;
    short gDepth;
    short bDepth;
    short aDepth;
    unsigned short bitCount;
    int padding;
    int i;
    BYTE *p;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
    WORD *pDst16;
    DWORD *pDst32;

    pInfo = ParseTGAHeader(pTGA);
    if (pInfo == NULL)
        return NULL;
    if (pInfo->bytesPerPixel == 4)
        pTexture->flags |= 1;
    width = pInfo->width;
    height = pInfo->height;
    memset(&createDesc, 0, sizeof(createDesc));
    createDesc.dwSize = sizeof(createDesc);
    createDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    createDesc.dwWidth = width;
    createDesc.dwHeight = height;
    createDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    if ((pTexture->flags & 1) && m_pTextureManager->textureInfo2 != NULL)
        pFormat = m_pTextureManager->textureInfo2;
    else
        pFormat = m_pTextureManager->textureInfo1;
    createDesc.ddpfPixelFormat = pFormat->desc.ddpfPixelFormat;
    g_pGraphics->pDD7->CreateSurface(&createDesc, &tmpTexture.pSurface, NULL);
    memset(&lockDesc, 0, sizeof(lockDesc));
    lockDesc.dwSize = sizeof(lockDesc);
    tmpTexture.pSurface->Lock(NULL, &lockDesc, DDLOCK_WAIT, NULL);

    rMask = lockDesc.ddpfPixelFormat.dwRBitMask;
    gMask = lockDesc.ddpfPixelFormat.dwGBitMask;
    bMask = lockDesc.ddpfPixelFormat.dwBBitMask;
    aMask = lockDesc.ddpfPixelFormat.dwRGBAlphaBitMask;
    for (i = 0, mask = rMask; i < 32 && !(mask & 1); i++)
        mask >>= 1;
    rShift = (BYTE)i;
    for (i = 0, mask = gMask; i < 32 && !(mask & 1); i++)
        mask >>= 1;
    gShift = (BYTE)i;
    for (i = 0, mask = bMask; i < 32 && !(mask & 1); i++)
        mask >>= 1;
    bShift = (BYTE)i;
    for (i = 0, mask = aMask; i < 32 && !(mask & 1); i++)
        mask >>= 1;
    aShift = (BYTE)i;
    for (i = 0, mask = rMask; i < 32 && mask != 0; i++)
        mask >>= 1;
    rDepth = (BYTE)i - 8;
    for (i = 0, mask = gMask; i < 32 && mask != 0; i++)
        mask >>= 1;
    gDepth = (BYTE)i - 8;
    for (i = 0, mask = bMask; i < 32 && mask != 0; i++)
        mask >>= 1;
    bDepth = (BYTE)i - 8;
    for (i = 0, mask = aMask; i < 32 && mask != 0; i++)
        mask >>= 1;
    aDepth = (BYTE)i - 8;

    bitCount = (unsigned short)lockDesc.ddpfPixelFormat.dwRGBBitCount;
    padding = lockDesc.lPitch - (bitCount * width >> 3);
    if (bitCount == 16) {
        pDst16 = (WORD *)lockDesc.lpSurface;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                p = SampleTGAPixel(x, y, pInfo, pTexture->flags);
                r = p[0];
                g = p[1];
                b = p[2];
                if (aShift < pInfo->bytesPerPixel * 8)
                    a = p[3];
                else
                    a = 0xff;
                if (aDepth >= 0)
                    a = ((aMask >> abs(aDepth)) & a) << abs(aDepth);
                else
                    a = ((aMask << abs(aDepth)) & a) >> abs(aDepth);
                if (rDepth >= 0)
                    r = ((rMask >> abs(rDepth)) & r) << abs(rDepth);
                else
                    r = ((rMask << abs(rDepth)) & r) >> abs(rDepth);
                if (gDepth >= 0)
                    g = ((gMask >> abs(gDepth)) & g) << abs(gDepth);
                else
                    g = ((gMask << abs(gDepth)) & g) >> abs(gDepth);
                if (bDepth >= 0)
                    b = ((bMask >> abs(bDepth)) & b) << abs(bDepth);
                else
                    b = ((bMask << abs(bDepth)) & b) >> abs(bDepth);
                *pDst16++ = (WORD)(r | g | b | a);
            }
            if (y != height - 1)
                pDst16 += (unsigned short)padding;
        }
    } else if (bitCount == 32) {
        pDst32 = (DWORD *)lockDesc.lpSurface;
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                p = SampleTGAPixel(x, y, pInfo, pTexture->flags);
                if (aShift < pInfo->bytesPerPixel * 8)
                    a = p[3];
                else
                    a = 0;
                *pDst32++ = (p[0] << rShift) | (p[1] << gShift) | (p[2] << bShift) | (a << aShift);
            }
            if (y != height - 1)
                pDst32 += padding;
        }
    } else {
        if (pTexture->pSurface != NULL && pTexture->pSurface->Release() == 0)
            pTexture->pSurface = NULL;
        return NULL;
    }

    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        m_unk0x0065fa2c += GetMipMapDataSize(pTexture);
    if (bitCount == 16)
        m_unk0x0065fa2c += height * width * 2;
    else
        m_unk0x0065fa2c += height * width * 4;
    tmpTexture.pSurface->Unlock(NULL);
    if (g_pGraphics->field913_0x3bc & 0x10) {
        if (strncmp(pTexture->name + strlen(pTexture->name) - 6, m_strSuffixBU, 2) == 0)
            pTexture->flags = (pTexture->flags & ~1) | 0x20;
    }
    pTexture->flags &= ~0x6000;
    if ((pTexture->flags & 0x20) && (g_pGraphics->field913_0x3bc & 0x10)) {
        CreateTextureSurface(pTexture, width, height, pTexture->flags);
        GenerateBumpMap(&tmpTexture, pTexture);
    } else {
        CreateTextureSurface(pTexture, width, height, pTexture->flags);
        pTexture->pSurface->Blt(NULL, tmpTexture.pSurface, NULL, DDBLT_WAIT, NULL);
    }
    if (tmpTexture.pSurface != NULL && tmpTexture.pSurface->Release() == 0)
        tmpTexture.pSurface = NULL;
    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        GetMipMapSurfaces(pTexture);
    if ((g_pGraphics->field913_0x3bc & 3) && (pTexture->flags & 0x10))
        BltMipMaps(pTexture);
    if (!(pTexture->flags & 0x1000))
        CFileBuffer::FreeGenericFileBuffer(pTGA);
    return pTexture;
}

// 1/255 and the double 1.0 the original keeps in its constant block.
// GLOBAL: CMR2 0x00511364
extern const float g_unk0x00511364 = 1.0f / 255.0f;
// GLOBAL: CMR2 0x00511428
extern const double g_unk0x00511428 = 1.0;
extern const float g_netZero;
extern const float g_netByteScale;
extern const float g_netOne;

// The D3DX colour helper the original links statically (ours is the import).
// LIBRARY: CMR2 0x004c674c
// _D3DXColorAdjustContrast@12

// Reads pixel (x, y) of a bottom-up TGA image as R, G, B, A in m_tgaPixel,
// applying the brightness and contrast of the car (flag 0x80) or track
// (flag 0x100) textures.
// FUNCTION: CMR2 0x004a60d0
BYTE *CGraphics::SampleTGAPixel(unsigned int x, unsigned int y, TGAImageInfo *pInfo, unsigned int flags)
{
    BYTE *p;
    float contrast;
    int brightness;
    int r;
    int g;
    int b;
    D3DXCOLOR in;
    D3DXCOLOR out;

    if (x >= pInfo->width || y >= pInfo->height) {
        m_tgaPixel[3] = 0;
        m_tgaPixel[2] = 0;
        m_tgaPixel[1] = 0;
        m_tgaPixel[0] = 0;
        return m_tgaPixel;
    }
    p = ((pInfo->height - y - 1) * pInfo->width + x) * pInfo->bytesPerPixel + pInfo->pixels;
    m_tgaPixel[2] = p[0];
    p++;
    m_tgaPixel[1] = p[0];
    p++;
    m_tgaPixel[0] = p[0];
    if (pInfo->bytesPerPixel == 4)
        m_tgaPixel[3] = p[1];
    if (flags & 0x80) {
        contrast = m_unk0x00520b34;
        brightness = m_unk0x0065fa44;
    } else if (flags & 0x100) {
        contrast = m_unk0x00520b38;
        brightness = m_unk0x0065fa48;
    }
    if (!(flags & 0x80) && !(flags & 0x100))
        return m_tgaPixel;
    if (brightness != 0) {
        r = m_tgaPixel[0] + brightness;
        g = m_tgaPixel[1] + brightness;
        b = m_tgaPixel[2] + brightness;
        if (r < 0)
            r = 0;
        else if (r > 0xff)
            r = 0xff;
        if (g < 0)
            g = 0;
        else if (g > 0xff)
            g = 0xff;
        if (b < 0)
            b = 0;
        else if (b > 0xff)
            b = 0xff;
        m_tgaPixel[0] = r;
        m_tgaPixel[1] = g;
        m_tgaPixel[2] = b;
    }
    if (contrast != g_unk0x00511428) {
        in.r = m_tgaPixel[0] * g_unk0x00511364;
        in.g = m_tgaPixel[1] * g_unk0x00511364;
        in.b = m_tgaPixel[2] * g_unk0x00511364;
        D3DXColorAdjustContrast(&out, &in, contrast);
        if (out.r > g_netOne)
            out.r = 1.0f;
        if (out.g > g_netOne)
            out.g = 1.0f;
        if (out.b > g_netOne)
            out.b = 1.0f;
        if (out.r < g_netZero)
            out.r = 0.0f;
        if (out.g < g_netZero)
            out.g = 0.0f;
        if (out.b < g_netZero)
            out.b = 0.0f;
        m_tgaPixel[0] = (BYTE)(int)(out.r * g_netByteScale);
        m_tgaPixel[1] = (BYTE)(int)(out.g * g_netByteScale);
        m_tgaPixel[2] = (BYTE)(int)(out.b * g_netByteScale);
    }
    return m_tgaPixel;
}

// Builds a bump map texture from a height map TGA: the height differences
// to the right (dU) and lower (dV) neighbours plus the alpha channel as
// luminance, packed into the 16 or 24 bit bump map format.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a6370
Texture *CGraphics::LoadTGABumpMap(BYTE *pTGA, Texture *pTexture)
{
    TGAImageInfo *pInfo;
    DDSURFACEDESC2 desc;
    unsigned int mask;
    BYTE uBits;
    BYTE vBits;
    BYTE lBits;
    int uShift;
    int vShift;
    int lShift;
    int i;
    unsigned int x;
    unsigned int y;
    unsigned int height;
    unsigned int right;
    unsigned int down;
    int du;
    int dv;
    BYTE l;
    WORD *pDst16;
    BYTE *pDst24;

    if (m_pTextureManager->textureInfo5 == NULL)
        return NULL;
    pInfo = ParseTGAHeader(pTGA);
    if (pInfo == NULL)
        return NULL;
    if (pInfo->bytesPerPixel == 3)
        return NULL;
    CreateTextureSurface(pTexture, pInfo->width, pInfo->height, pTexture->flags);
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    pTexture->pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);
    if (desc.ddpfPixelFormat.dwRGBBitCount != 16 && desc.ddpfPixelFormat.dwRGBBitCount != 24) {
        pTexture->pSurface->Unlock(NULL);
        return NULL;
    }
    for (uBits = 0, mask = desc.ddpfPixelFormat.dwBumpDuBitMask, i = 32; i != 0; i--, mask >>= 1)
        if (mask & 1)
            uBits++;
    uBits = 8 - uBits;
    for (vBits = 0, mask = desc.ddpfPixelFormat.dwBumpDvBitMask, i = 32; i != 0; i--, mask >>= 1)
        if (mask & 1)
            vBits++;
    vBits = 8 - vBits;
    for (lBits = 0, mask = desc.ddpfPixelFormat.dwBumpLuminanceBitMask, i = 32; i != 0; i--, mask >>= 1)
        if (mask & 1)
            lBits++;
    lBits = 8 - lBits;
    for (uShift = 0, mask = desc.ddpfPixelFormat.dwBumpDuBitMask; uShift < 32 && !(mask & 1); uShift++)
        mask >>= 1;
    for (vShift = 0, mask = desc.ddpfPixelFormat.dwBumpDvBitMask; vShift < 32 && !(mask & 1); vShift++)
        mask >>= 1;
    for (lShift = 0, mask = desc.ddpfPixelFormat.dwBumpLuminanceBitMask; lShift < 32 && !(mask & 1); lShift++)
        mask >>= 1;

    pDst16 = (WORD *)desc.lpSurface;
    pDst24 = (BYTE *)desc.lpSurface;
    for (y = 0; y < pInfo->height; y++) {
        for (x = 0; x < pInfo->width; x++) {
            height = *SampleTGAPixel(x, y, pInfo, 0);
            if (x < pInfo->width - 1)
                right = *SampleTGAPixel(x + 1, y, pInfo, 0);
            else
                right = height + 0x80;
            if (y < pInfo->height - 1)
                down = *SampleTGAPixel(x, y + 1, pInfo, 0);
            else
                down = height + 0x80;
            du = abs((int)(height - right));
            dv = abs((int)(height - down));
            l = SampleTGAPixel(x, y, pInfo, 0)[3];
            if (desc.ddpfPixelFormat.dwRGBBitCount == 16) {
                *pDst16++ = (WORD)(((l >> lBits) << lShift) | ((dv >> vBits) << vShift) | ((du >> uBits) << uShift));
            } else {
                pDst24[0] = (BYTE)du;
                pDst24[1] = (BYTE)dv;
                pDst24[2] = l;
                pDst24 += 3;
            }
        }
    }
    pTexture->pSurface->Unlock(NULL);
    CFileBuffer::FreeGenericFileBuffer(pTGA);
    return pTexture;
}

// Sets up the texture stage for the texture's blend mode (stage alone with
// no texture): colour/alpha operations, filters, blend render states and
// texture coordinate set.
// FUNCTION: CMR2 0x004a3e90
void CGraphics::FUN_004a3e90(int param1, int param2)
{
    Texture *pTexture;
    float value;

    pTexture = (Texture *)param2;
    if (pTexture == NULL) {
        g_unk0x00520b18 = -1;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, (DWORD)pTexture);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, (DWORD)pTexture);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        return;
    }
    switch (pTexture->blendMode) {
    case 0:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        if (g_pGraphics->field913_0x3bc & 1)
            m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MIPFILTER, D3DTFP_POINT);
        else if (g_pGraphics->field913_0x3bc & 2)
            m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
        else
            m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MIPFILTER, D3DTFP_NONE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MAXMIPLEVEL, 0);
        FUN_004a3e40(5, 6);
        SetTexCoordIndex(param1, 0);
        break;
    case 1:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        FUN_004a3e40(2, 2);
        SetTexCoordIndex(param1, 0);
        break;
    case 2:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        FUN_004a3e40(5, 2);
        SetTexCoordIndex(param1, 0);
        break;
    case 3:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        FUN_004a3e40(1, 4);
        SetTexCoordIndex(param1, 0);
        break;
    case 4:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        SetTexCoordIndex(param1, 1);
        break;
    case 5:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        FUN_004a3e40(5, 2);
        SetTexCoordIndex(param1, 0);
        break;
    case 6:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        SetTexCoordIndex(param1, 0);
        break;
    case 7:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        FUN_004a3e40(9, 3);
        SetTexCoordIndex(param1, 1);
        break;
    case 8:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        value = m_bumpScale;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVMAT00, *(DWORD *)&value);
        value = 0.0f;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVMAT01, *(DWORD *)&value);
        value = 0.0f;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVMAT10, *(DWORD *)&value);
        value = m_bumpScale;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVMAT11, *(DWORD *)&value);
        value = 1.0f;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVLSCALE, *(DWORD *)&value);
        value = 0.0f;
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_BUMPENVLOFFSET, *(DWORD *)&value);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        SetTexCoordIndex(param1, 0);
        break;
    case 9:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        FUN_004a3e40(2, 2);
        SetTexCoordIndex(param1, 1);
        break;
    case 10:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        FUN_004a3e40(5, 6);
        SetTexCoordIndex(param1, 0);
        break;
    case 11:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_TEXCOORDINDEX,
                                                      D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | 1);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
        break;
    case 0xff:
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        m_pTextureManager->pD3D->SetTextureStageState(param1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        FUN_004a3e40(5, 6);
        SetTexCoordIndex(param1, 0);
        break;
    }
    g_unk0x00520b18 = pTexture->blendMode;
}

// Applies a texture stage change and forwards it to the render states.
// FUNCTION: CMR2 0x004a4850
void CGraphics::FUN_004a4850(int param1, int param2)
{
    if (m_unk0x0065fa38 == param2 && m_unk0x00520b28 == param1)
        return;
    m_unk0x0065fa38 = param2;
    m_unk0x00520b28 = param1;
    if (param2 != 0) {
        m_pTextureManager->pD3D->SetTexture((DWORD)param1,
            (IDirectDrawSurface7 *)*(int *)(param2 + 0x114));
        FUN_004a3e90(param1, param2);
    } else {
        m_pTextureManager->pD3D->SetTexture((DWORD)param1, NULL);
        FUN_004a3e90(param1, 0);
    }
    m_unk0x0065fa24++;
}

// GLOBAL: CMR2 0x0065ad18
DWORD g_textureFactor;

// Sets the alpha of the texture factor render state from pColour[3].
// FUNCTION: CMR2 0x004a3df0
void Graphics_SetTextureFactorAlpha(BYTE *pColour)
{
    g_textureFactor = pColour[3] << 24;
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, g_textureFactor);
}

// GLOBAL: CMR2 0x00520b4c
char g_strTextureNotFound[28] = "couldn't find texture =  %s";

// Reloads the image of a loaded texture from its archive, e.g. after its
// name was changed to another variant.
// FUNCTION: CMR2 0x004a5da0
void Graphics_ReloadTexture(Texture *pTexture)
{
    int i;
    Texture *p;
    DWORD *pData;

    for (i = 0; i < 2048; i++) {
        p = CGraphics::m_pTextureManager->textureBuffer[i];
        if (p == pTexture) {
            if (p->pSurface != NULL && p->pSurface->Release() == 0)
                p->pSurface = NULL;
            pData = (DWORD *)CGenericFileLoader::FindFile((GenericFile *)p->pArchive, p->name, NULL, NULL, 0);
            if (pData == NULL) {
                sprintf(CFrontend::m_stringDest, g_strTextureNotFound, p->name);
                return;
            }
            if (*pData == 0x20534444)
                CGraphics::LoadDDSTexture((DDSFile *)pData, p);
            else
                CGraphics::LoadTGATexture((BYTE *)pData, p);
        }
    }
}

// Looks for a free timer slot (shape 5); the result is not used.
// FUNCTION: CMR2 0x004bc410
void Timer_FindFree(void)
{
    int slot;
    int i;

    slot = 0;
    for (i = 0; i < 32; i++) {
        if (g_unk0x00521138[slot][0] == 5)
            return;
        slot = (slot + 1) % 32;
    }
}

// Cube-map texture ids reserved for the car reflections (-1 when unused).
// GLOBAL: CMR2 0x006de95c
unsigned int g_unk0x006de95c[20];

// Reserves count cube maps of the given size, then loads the environment
// texture; returns 0 when it is missing.
// FUNCTION: CMR2 0x004b23c0
int FUN_004b23c0(char *name, int count, GenericFile *pFile, DWORD size)
{
    unsigned int *pId;
    unsigned short *pTexture;

    CGraphics::m_cubeMapSize = size;
    memset(g_unk0x006de95c, 0xff, sizeof(g_unk0x006de95c));
    if ((*(BYTE *)&g_pGraphics->field913_0x3bc & 0x80) != 0 && count > 0) {
        pId = g_unk0x006de95c;
        do {
            *pId++ = *(unsigned short *)FUN_004a4b10();
        } while (--count != 0);
    }
    pTexture = (unsigned short *)CTexture::FindLoadTexture(pFile, name, NULL, 0, 0, 0x8000);
    if (pTexture != NULL) {
        g_unk0x005210b8 = *pTexture;
        FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8], 4);
        return 1;
    }
    return 0;
}

void FloatMatrix_Multiply(D3DMATRIX *pOut, D3DMATRIX *pA, D3DMATRIX *pB);
extern const float g_netOne;

// Scale applied to the shadow vertex positions (0.5).
// GLOBAL: CMR2 0x00511424
extern const float g_unk0x00511424 = 0.5f;

// Builds the view * world matrix, uses it to project every mesh vertex (taking
// the midpoint between the vertex and the next one, 0x30 bytes apart) into the
// x/y stored at offsets 0x28/0x2c of the vertex, and re-uploads the vertices
// to the mesh's slot of the shared vertex buffer.
// FUNCTION: CMR2 0x004b2460
void FUN_004b2460(Mesh *pMesh)
{
    D3DMATRIX transform;
    D3DMATRIX world;
    D3DMATRIX view;
    float m11, m21, m31, m12, m22, m32;
    float *pVertexData = (float *)pMesh->pVertexData;
    void *pVertices;
    int i;
    int j;

    CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_VIEW, &view);
    CGraphics::m_pTextureManager->pD3D->GetTransform(D3DTRANSFORMSTATE_WORLD, &world);
    FloatMatrix_Multiply(&transform, &view, &world);
    m11 = transform._11;
    m21 = transform._21;
    m31 = transform._31;
    m12 = transform._12;
    m22 = transform._22;
    m32 = transform._32;
    for (i = 0; i < pMesh->triangleCount; i++) {
        for (j = 0; j < 3; j++) {
            int index = pMesh->pTriangles[i].vertexIndex[j];
            float *pVertex = (float *)((BYTE *)pVertexData + index * 0x30);
            float x = (pVertex[3] + pVertex[0]) * g_unk0x00511424;
            float y = (pVertex[4] + pVertex[1]) * g_unk0x00511424;
            float z = (pVertex[5] + pVertex[2]) * g_unk0x00511424;

            pVertex[10] = (z * m31 + y * m21 + x * m11 + g_netOne) * g_unk0x00511424;
            pVertex[11] = (g_netOne - (z * m32 + y * m22 + x * m12)) * g_unk0x00511424;
        }
    }
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Lock(0x821, &pVertices, NULL);
    memcpy((BYTE *)pVertices + pMesh->vertexOffset * 0x30, pMesh->pVertexData, pMesh->field_0x10 * 0x30);
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Unlock();
}

// Draws the triangles of a mesh in contiguous texture runs, setting the
// reserved cube map as texture and clamping the texture address while the
// shadow geometry is drawn.
// FUNCTION: CMR2 0x004b2610
void FUN_004b2610(Mesh *pMesh)
{
    int triangleCount;
    int count;
    int currentTexture;
    int i;
    int textureIndex;
    int texture;

    FUN_004b2460(pMesh);
    triangleCount = pMesh->triangleCount;
    count = 0;
    currentTexture = -1;
    FUN_0049dcc0(1);
    CGraphics::SetTextureAddressClamp(0);
    if ((g_unk0x005210b8 < 0 && (int)g_unk0x006de95c[(pMesh->flags >> 15) & 7] < 0) || triangleCount <= 0)
        goto done;
    for (i = 0; i < triangleCount; i++) {
        textureIndex = pMesh->pTriangles[i].field_0x30;
        if (currentTexture != *(int *)((BYTE *)&pMesh->pTriangles[i] + 4 + textureIndex * 4)) {
            if (count > 0) {
                CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
                    D3DPT_TRIANGLELIST,
                    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
                    pMesh->vertexOffset, pMesh->field_0x10, g_unk0x006dd9bc, count, 0);
            }
            count = 0;
            currentTexture = *(int *)((BYTE *)&pMesh->pTriangles[i] + 4 + textureIndex * 4);
            if (currentTexture > -1) {
                if ((g_pGraphics->field913_0x3bc & 0x10) != 0) {
                    FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[currentTexture], 8);
                    CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[currentTexture]);
                    FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8], 9);
                    if (g_unk0x005210bc != 0)
                        CGraphics::FUN_004a4850(1, (int)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8]);
                } else {
                    FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8], 4);
                    CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8]);
                    FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[currentTexture], 5);
                    if (g_unk0x005210bc != 0)
                        CGraphics::FUN_004a4850(1, (int)CGraphics::m_pTextureManager->textureBuffer[currentTexture]);
                }
            }
        }
        if (currentTexture > -1) {
            g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[0];
            g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[1];
            g_unk0x006dd9bc[count++] = pMesh->pTriangles[i].vertexIndex[2];
            CGame::m_unk0x0059ce18++;
        }
    }
    if (count == 0)
        goto done;
    texture = *(int *)((BYTE *)&pMesh->pTriangles[triangleCount - 1] + 4 + textureIndex * 4);
    if (texture > -1) {
        if ((g_pGraphics->field913_0x3bc & 0x10) != 0) {
            FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[texture], 8);
            CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
            FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8], 9);
            if (g_unk0x005210bc != 0)
                CGraphics::FUN_004a4850(1, (int)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8]);
            } else {
            FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8], 4);
            CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[g_unk0x005210b8]);
            FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[texture], 5);
            if (g_unk0x005210bc != 0)
                CGraphics::FUN_004a4850(1, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
        }
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
            D3DPT_TRIANGLELIST,
            CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
            pMesh->vertexOffset, pMesh->field_0x10, g_unk0x006dd9bc, count, 0);
    }
done:
    CGraphics::SetTextureAddressClamp(1);
    CGraphics::FUN_004a4850(1, 0);
    CGraphics::FUN_004a4850(2, 0);
}

// Finds a loaded texture by name (not for "local" textures) and makes sure
// it is resident.
// FUNCTION: CMR2 0x004b9b80
Texture *FUN_004b9b80(char *name)
{
    int i;

    if (Graphics_HasLocalSuffix(name) == 0) {
        for (i = 0; i < (int)CGraphics::m_textureCount; i++) {
            if (CGraphics::m_pTextureManager->textureBuffer[i] != NULL &&
                strcmp(CGraphics::m_pTextureManager->textureBuffer[i]->name, name) == 0) {
                i = FUN_004a4bd0(CGraphics::m_pTextureManager->textureBuffer[i], i);
                return CGraphics::m_pTextureManager->textureBuffer[i];
            }
        }
    }
    return NULL;
}

// Non-zero while the draw lists are depth sorted before being drawn
// (0x49cbc0/0x49cc50/0x49cd20).
// GLOBAL: CMR2 0x005207b4
int g_unk0x005207b4 = 1;

// Draws the LOD record a mesh is using: takes the whole-triangle-list or the
// part-list path its flags ask for and binds cull mode, alpha blending and
// lighting for it, restoring the lighting mode afterwards.
// FUNCTION: CMR2 0x0049c940
void Graphics_DrawMeshLOD(Mesh *pMesh, int useParts, int clampTexture, int markTextures)
{
    Mesh *pLod = (Mesh *)((BYTE *)pMesh + ((BYTE *)pMesh)[0x112] * 0x108);
    unsigned int lightingMode;

    if (pLod->pTriangles == NULL)
        return;

    // Triangle runs are only stored for meshes flagged with the run list.
    if (useParts == 0 && (pLod->flags & 0x2000) == 0)
        useParts = 1;

    if ((pLod->flags & 1) != 0)
        CGraphics::SetCullMode(1);
    else
        CGraphics::SetCullMode(CGame::FUN_0049dcb0());

    // Meshes with flag 0x20 keep their own lighting mode and the restore at
    // the end skips them, so the saved mode stays uninitialised for them
    // (the original reads it anyway).
    if ((pLod->flags & 0x20) == 0) {
        lightingMode = (unsigned int)FUN_004b7200();
        if ((pLod->flags & 0x40000) != 0)
            Graphics_SetLightingMode(3);
        else
            Graphics_SetLightingMode(2);
    }

    if (g_unk0x005207b4 != 0)
        FUN_0049dcc0((int)((pLod->flags >> 3) & 1));
    else
        FUN_0049dcc0(0);

    if (useParts != 0) {
        if (markTextures != 0)
            FUN_0049c7b0(pLod);
        else
            FUN_0049c880(pLod);
    } else if (clampTexture != 0) {
        FUN_0049c510(pLod);
    } else {
        FUN_0049c680(pLod);
    }

    if ((pLod->flags & 0x200) != 0 && (pLod->flags & 0x40000) == 0 &&
        (g_pGraphics->field913_0x3bc & 8) != 0) {
        if ((g_pGraphics->field913_0x3bc & 0x80) != 0)
            Mesh_DrawEnvMapped(pLod);
        else
            FUN_004b2610(pLod);
    }

    if ((pLod->flags & 0x20) == 0)
        Graphics_SetLightingMode(lightingMode);
}

// Draws every part of a mesh from its vertex buffer, one texture at a time,
// and counts the triangles drawn.
// FUNCTION: CMR2 0x0049c880
void FUN_0049c880(Mesh *pMesh)
{
    int i;
    MeshPart **ppPart;
    MeshPart *pPart;

    for (i = 0, ppPart = pMesh->pParts; i < pMesh->partCount; i++, ppPart++) {
        pPart = *ppPart;
        CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[pPart->texture]);
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
            D3DPT_TRIANGLELIST, CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
            pMesh->vertexOffset + pPart->minIndex, pMesh->field_0x10 - pPart->minIndex, pPart->pData,
            pPart->indexCount, 0);
        CGame::m_unk0x0059ce18 += pPart->indexCount / 3;
    }
}

// Same as FUN_0049c880, first marking each part's texture as used (10).
// FUNCTION: CMR2 0x0049c7b0
void FUN_0049c7b0(Mesh *pMesh)
{
    int i;
    MeshPart **ppPart;
    MeshPart *pPart;

    for (i = 0, ppPart = pMesh->pParts; i < pMesh->partCount; i++, ppPart++) {
        pPart = *ppPart;
        FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[pPart->texture], 10);
        CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[pPart->texture]);
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
            D3DPT_TRIANGLELIST, CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
            pMesh->vertexOffset + pPart->minIndex, pMesh->field_0x10 - pPart->minIndex, pPart->pData,
            pPart->indexCount, 0);
        CGame::m_unk0x0059ce18 += pPart->indexCount / 3;
    }
}

// GLOBAL: CMR2 0x0059be74
unsigned short g_unk0x0059be74[2000];

// Draws a mesh's triangles in contiguous texture runs.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049c680
void FUN_0049c680(Mesh *pMesh)
{
    int currentTexture = -1;
    int count = 0;
    int n;
    MeshTriangle *pTri = pMesh->pTriangles;

    for (n = pMesh->triangleCount; n != 0; n--) {
        int texture = *(int *)((BYTE *)pTri + 4 + pTri->field_0x2c * 4);
        if (texture != currentTexture) {
            if (count > 0) {
                CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
                    D3DPT_TRIANGLELIST,
                    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
                    pMesh->vertexOffset, pMesh->field_0x10, g_unk0x0059be74, count, 0);
            }
            count = 0;
            CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
            currentTexture = texture;
        }
        g_unk0x0059be74[count++] = pTri->vertexIndex[0];
        g_unk0x0059be74[count++] = pTri->vertexIndex[1];
        g_unk0x0059be74[count++] = pTri->vertexIndex[2];
        CGame::m_unk0x0059ce18++;
        pTri++;
    }
    if (count != 0) {
        MeshTriangle *pLast = &pMesh->pTriangles[pMesh->triangleCount - 1];
        int texture = *(int *)((BYTE *)pLast + 4 + pLast->field_0x2c * 4);
        CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
            D3DPT_TRIANGLELIST,
            CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
            pMesh->vertexOffset, pMesh->field_0x10, g_unk0x0059be74, count, 0);
    }
}
