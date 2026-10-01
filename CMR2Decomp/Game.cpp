#include "StageBlock.h"
#include "Game.h"
#include "Menu.h"
#include "main.h"
#include "RegKey.h"
#include "GameInfo.h"
#include "InstallInfo.h"
#include "NetworkLeaderboards.h"
#include "Graphics.h"
#include "Input.h"
#include "FileBuffer.h"
#include "Mesh.h"
#include "Frontend.h"
#include "Texture.h"
#include "Sprite.h"
#include "Sound.h"
#include "Font.h"
#include "Sprite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

BOOL CGame::m_shouldExit = FALSE;
BOOL CGame::m_isActive = FALSE;
int CGame::m_unk0x0059ce14;
int CGame::m_unk0x0059ce18;
// GLOBAL: CMR2 0x0059ce1c
int CGame::m_unk0x0059ce1c;
int CGame::m_unk0x0059ce20;
int CGame::m_unk0x0059ce28;
int CGame::m_unk0x0059ce2c;
void *CGame::m_unk0x00593cb0[4096];
void *CGame::m_unk0x00597d04[4096];
int CGame::m_unk0x005207f8 = 3;
int CGame::m_unk0x00663dc4;
DPlayConnection CGame::m_connections[10];
BYTE CGame::m_maxConnections;
BYTE CGame::m_connectionCount;
int CGame::m_unk0x00523c58 = -1;
int CGame::m_unk0x00523c5c = -1;
Unk0049c2c0 CGame::m_unk0x00817da0;
int CGame::m_unk0x0052ea4c;
BYTE CGame::m_unk0x0052ea51;
bool CGame::m_unk0x00817eb0 = false;
Unk00817d98 CGame::m_unk0x00817d98;
unsigned int CGame::m_unk0x00523c18[16] = {
    0x0101ff00, 0x0600ff00, 0x0706ff00, 0x0200ff01, 0x0300ff02, 0x0400ff03,
    0x0500ff04, 0x0600ff05, 0x0700ff06, 0x0902ff06, 0x0707ff06, 0x0807ff06,
    0x0900ff08, 0x0900ff07, 0x0000ff09, 0xffffffff,
};
BYTE CGame::m_unk0x00593cac;
CallbackIndex CGame::m_unk0x00593ba8;
Unk00817d98 *CGame::m_unk0x00593ba4;

BYTE CGame::m_unk0x00523d68 = 1;
BYTE CGame::m_unk0x008180f9;
BYTE CGame::m_unk0x008180fc;
BYTE CGame::m_unk0x00516120 = 1;
BYTE CGame::m_unk0x00531768;
BYTE CGame::m_unk0x0052ea58;
BYTE CGame::m_unk0x0052ea59;

// GLOBAL: CMR2 0x00593ba0;
int CGame::m_unk0x00593ba0;

// GLOBAL: CMR2 0x005939a0
void *CGame::m_callbacks[64];

BYTE CGame::m_unk0x005a1818;
BYTE CGame::m_unk0x005a1819;
Unk0x005a1820 CGame::m_unk0x005a1820[7];
int CGame::m_unk0x005a1e34;
bool CGame::m_unk0x005a1fc0;
IDirectPlay4A *CGame::m_pDirectPlay4A = NULL;
DPID CGame::m_unk0x005a1ea0 = NULL;

IDirectPlayLobby3A *CGame::m_pDirectPlayLobby3A;

BOOL CGame::m_unk0x005a1fbc;
void *CGame::m_unk0x005a1fb8;



// Boot states of the grouped callback machine (functions next to
// CGame::InitializeGame; the render halves are not written yet).
void FUN_004d0ea0(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1a90(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1b40(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1c90(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d0a80(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d0ba0(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1cc0(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1ba0(Unk0049c2c0 *p1, BYTE state);
void FUN_004d1c30(Unk0049c2c0 *p1, BYTE state);
void FUN_004d1e10(Unk0049c2c0 *p1, BYTE state);
void FUN_005012e0(Unk0049c2c0 *p1, BYTE state);
void FUN_005015d0(Unk0049c2c0 *p1, BYTE state);
void FUN_005010a0(Unk0049c2c0 *p1, BYTE state);
void FUN_00501130(Unk0049c2c0 *p1, BYTE state);
void FUN_004d1e90(Unk0049c2c0 *p1, BYTE p2);
void FUN_004d1080(Unk0049c2c0 *p1, BYTE p2);
void FUN_00501350(int param1, int unused);
void FUN_004d1370(Unk0049c2c0 *p1, BYTE p2);

FuncTableGroup CGame::m_initializeGameGroupedFuncTable[10] = {
    {InitializeGame,
     FUN_00501680},
    {FUN_004d1b40, FUN_004d1080},
    {FUN_004d1b40, FUN_004d1370},
    {FUN_004d1ba0, FUN_00501680},
    {FUN_004d1c30, FUN_00501680},
    {FUN_004d1a90, FUN_004d0ea0},
    {FUN_004d1c90, FUN_004d0a80},
    {FUN_004d1cc0, FUN_004d0ba0},
    {FUN_004d1e10, FUN_00501680},
    {FUN_004d1e90, FUN_00501680},
};

// FUNCTION: CMR2 0x004a15a0
BOOL FUN_004a15a0(void)
{
    return CGameInfo::m_unk0x005a0060;
}

// FUNCTION: CMR2 0x004a9a40
void CGame::SetShouldExit(void)
{
    m_shouldExit = TRUE;
}

// FUNCTION: CMR2 0x004b7a40
void CGame::FUN_004b7a40(void)
{
    SoundSlot **ppSlot;

    if (CSound::m_unk0x006e0eec != 0) {
        ppSlot = CSound::m_soundSlots;
        do {
            if (*ppSlot != NULL)
                CSound::FUN_004a27c0(*ppSlot);
            ppSlot++;
        } while ((int)ppSlot < (int)&CSound::m_soundSlotsEnd);
    }
}

// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d0780
BOOL CGame::FUN_004d0780(void)
{
    m_unk0x00523c5c = FUN_004057d0();
    if (m_unk0x00523c58 != m_unk0x00523c5c)
        m_unk0x00523c58 = m_unk0x00523c5c;

    switch (m_unk0x00523c5c)
    {
    case 3:
        return FUN_0041b060();

    case 2:
        return FUN_004ff450();

    case 0:
        if (m_unk0x00817eb0)
        {
            FUN_0049c2c0(&m_unk0x00817da0);
            FUN_0049c310(&m_unk0x00817da0);
            FUN_0049c370(&m_unk0x00817da0);
            return FALSE;
        }

        FUN_0049c150(&m_unk0x00817d98, 0, 0xFF);
        FUN_0049c190(&m_unk0x00817da0, 1, &m_unk0x00817d98, m_initializeGameGroupedFuncTable, &m_unk0x00523c18);
        m_unk0x00817eb0 = true;
        return FALSE;

    default:
        return FALSE;
    }
}

// Splash screen scene, created while the frontend resources load.
// GLOBAL: CMR2 0x00817fc4
SceneNode *g_unk0x00817fc4;
// GLOBAL: CMR2 0x00817fc8
SceneNode *g_unk0x00817fc8;

// Destroys the splash screen scene (registered as a callback by FUN_004d0840).
// FUNCTION: CMR2 0x004d0820
BYTE FUN_004d0820(void)
{
    if (g_unk0x00817fc8 != NULL)
        SceneNode_Destroy(g_unk0x00817fc8);

    return TRUE;
}

// Creates the empty scene the frontend draws while it loads its textures.
// FUNCTION: CMR2 0x004d0840
void FUN_004d0840(void)
{
    FixVector translation;
    FixAngles angles;
    SceneNode *pNode;

    translation.x = 0;
    translation.y = 0;
    translation.z = 0xffec0000;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0;
    angles.pad = 0;

    g_unk0x00817fc8 = SceneNode_CreateRoot();
    g_unk0x00817fc4 = SceneType2_Create(&translation, &angles, NULL, g_unk0x00817fc8);

    for (pNode = g_unk0x00817fc8; pNode != NULL; pNode = pNode->pParent)
        pNode->dirty = 1;

    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGame::RegisterCallback(FUN_004d0820, NULL);
}

// Set once the boot sequence has finished loading; the render states clear it.
// GLOBAL: CMR2 0x00817fcc
BYTE g_unk0x00817fcc;
// Format of the debug FPS counter ("FPS: %.2f").
// GLOBAL: CMR2 0x00516e14
char g_strFpsFormat0x00516e14[10] = "FPS: %.2f";
// Colour table of the debug FPS text; only the entry 0 is read.
// GLOBAL: CMR2 0x00523c64
int g_unk0x00523c64[5] = { -1, 120, 120, 120, 120 };

void FUN_004ea5b0(void);
float FUN_004b23a0(void);
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);
int FUN_0049d3f0(int, int, void *, int, BYTE);
void FUN_0049de40(void);

// Boot render state: places the splash-scene camera, clears the target and
// prints the FPS counter while the graphics debug flag (bit 2) is set.
// FUNCTION: CMR2 0x004d0a80
void FUN_004d0a80(Unk0049c2c0 *p1, BYTE p2)
{
    FixVector translation;
    FixAngles angles;
    short rect[4];

    translation.x = 0;
    translation.y = 0;
    translation.z = 0xffec0000;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0;
    angles.pad = 0;
    rect[0] = 0;
    rect[1] = 0;
    rect[2] = (short)g_pGraphics->resX;
    rect[3] = (short)g_pGraphics->resY;

    SceneNode_SetPosition(g_unk0x00817fc4, &translation);
    SceneNode_SetRotation(g_unk0x00817fc4, &angles);
    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    FUN_004ea5b0();
    Game_PrepareScene(g_unk0x00817fc8, g_unk0x00817fc4, (int)rect, 0);
    if ((g_pGraphics->field913_0x3bc & 4) != 0) {
        sprintf(CFrontend::m_stringDest, g_strFpsFormat0x00516e14, FUN_004b23a0());
        Font_DrawText(0, CFrontend::m_stringDest, 0, 0, g_unk0x00523c64, 9);
    }
    FUN_0049d3f0((int)g_unk0x00817fc8, (int)g_unk0x00817fc4, rect, 0, 1);
    if (g_unk0x00817fcc == 0)
        FUN_0049de40();
}

// Shared with the other boot renders of Game.cpp (0x4d0a80, 0x4d1370).

// Render half of boot state 1: draws the CMR2 logo and a row of up to four
// loading sprites, prepares the splash scene and shows the FPS counter when the
// debug flag of the graphics device is on.
// FUNCTION: CMR2 0x004d1080
void FUN_004d1080(Unk0049c2c0 *p1, BYTE p2)
{
    BYTE colour[4];
    SpriteRect dst;
    SpriteRect src;
    SpriteRect screen;
    int centre[3];
    Texture *pTexture;
    int count;
    int i;

    screen.x = 0;
    screen.y = 0;
    screen.w = g_pGraphics->resX;
    screen.h = g_pGraphics->resY;
    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    centre[0] = 0;
    centre[1] = 0;
    centre[2] = 0;

    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();

    pTexture = CFrontend::m_unk0x00817fd0;
    if (pTexture != NULL) {
        // the texture size is reused for the source, the destination and the centre
        src.x = pTexture->field_0x11c;
        src.y = pTexture->field_0x11e;
        short w = pTexture->width;
        src.w = w;
        short h = pTexture->height;
        src.h = h;
        dst.x = (int)(g_pGraphics->resX * 43) / 640;
        dst.y = (int)(g_pGraphics->resY * 202) / 480;
        dst.w = w;
        dst.h = h;
        centre[0] = w / 2;
        centre[1] = h / 2;
        Sprite_Queue(&src, &dst, pTexture, 1, 0, centre, NULL, colour, 8);
    }

    if (CGameInfo::GetScreenWidth() >= 0x400 &&
        CFrontend::FUN_004b7560(0x400) &&
        CFrontend::FUN_004b7590(0x400))
        count = 4;
    else
        count = 3;

    dst.x = (int)(g_pGraphics->resX * 43) / 640;
    dst.y = (int)(g_pGraphics->resY * 260) / 480;

    for (i = 0; i < count; i++) {
        pTexture = CFrontend::m_unk0x00817fd4[i];
        if (pTexture != NULL) {
            src.x = pTexture->field_0x11c;
            src.y = pTexture->field_0x11e;
            short w = pTexture->width;
            src.w = w;
            short h = pTexture->height;
            src.h = h;
            dst.w = w;
            dst.h = h;
            centre[0] = w / 2;
            centre[1] = h / 2;
            Sprite_Queue(&src, &dst, pTexture, 1, 0, centre, NULL, colour, 8);
            dst.x += dst.w;
        }
    }

    Game_PrepareScene(g_unk0x00817fc8, g_unk0x00817fc4, (int)&screen, 0);
    if (g_pGraphics->field913_0x3bc & 4) {
        sprintf(CFrontend::m_stringDest, g_strFpsFormat0x00516e14, FUN_004b23a0());
        Font_DrawText(0, CFrontend::m_stringDest, 0, 0, (int *)colour, 9);
    }
    FUN_0049d3f0((int)g_unk0x00817fc8, (int)g_unk0x00817fc4, &screen, 0, 1);
    if (g_unk0x00817fcc == 0)
        FUN_0049de40();
}

int FUN_004d0d30(int, int, char *, char);
unsigned char RallyDataCountryIndex(void);

// Boot render state shown while the country data loads: draws the country name
// with the current championship position, centred on the screen.
// FUNCTION: CMR2 0x004d0ba0
void FUN_004d0ba0(Unk0049c2c0 *p1, BYTE p2)
{
    FixVector translation;
    FixAngles angles;
    short rect[4];
    int x;
    int y;

    translation.x = 0;
    translation.y = 0;
    translation.z = 0xffec0000;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0;
    angles.pad = 0;

    if (CFrontend::FUN_004d20f0() == 1) {
        rect[0] = 0;
        rect[1] = 0;
        rect[2] = (short)g_pGraphics->resX;
        rect[3] = (short)g_pGraphics->resY;
        SceneNode_SetPosition(g_unk0x00817fc4, &translation);
        SceneNode_SetRotation(g_unk0x00817fc4, &angles);
        CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
        CGraphics::SetClearColour(1, 0, 0, 0);
        CGraphics::ClearTarget();
        CGraphics::ClearZBuffer();
        x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
        y = FUN_004d0d30(x, (int)g_pGraphics->resY / 2, CFrontend::GetTextString(0x1bd), 0);
        // the original copies the country text with sprintf("%s").
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                CFrontend::GetTextString((RallyDataCountryIndex() & 0xff) + 0x27));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        FUN_004d0d30(y, (int)g_pGraphics->resY / 2, CFrontend::m_stringDest, 1);
        Game_PrepareScene(g_unk0x00817fc8, g_unk0x00817fc4, (int)rect, 0);
        FUN_0049d3f0((int)g_unk0x00817fc8, (int)g_unk0x00817fc4, rect, 0, 1);
        if (g_unk0x00817fcc == 0)
            FUN_0049de40();
    }
}

// Sets the FPU control word to 53-bit precision (the CRT's default for the
// x87 unit before the game changes it). This is CRT startup code, which the
// original built with /Os: the size optimisation is what turns the cdecl
// cleanup into `pop ecx / pop ecx`.
#pragma optimize("s", on)
// FUNCTION: CMR2 0x00405796
void FUN_00405796(void)
{
    _controlfp(_PC_53, _MCW_PC);
}
#pragma optimize("s", off)

// FUNCTION: CMR2 0x004057a8
int FUN_004057a8(void)
{
    return 0;
}

// FUNCTION: CMR2 0x004057ab
void FUN_004057ab(void)
{
}

// FUNCTION: CMR2 0x004057d0
int CGame::FUN_004057d0(void)
{
    return m_unk0x0052ea4c;
}

// FUNCTION: CMR2 0x004057c0
void CGame::FUN_004057c0(void)
{
    m_unk0x0052ea51 = 1;
}

// FUNCTION: CMR2 0x004057e0
void CGame::FUN_004057e0(int param1)
{
    m_unk0x0052ea4c = param1;
}

// --- Boot pieces called by CGame::InitializeGame that are not decompiled
// yet. They are empty STUBs so the calls are in place; they do nothing yet.
BYTE FUN_004067b0(void);
void FUN_004067c0(BYTE param1);
void RallyData_FUN_00408290(void);
void RallyData_FUN_0040df30(void);
void FUN_004ea9f0(void);
DWORD FUN_004eaa00(void);
void FUN_0040bad0(void);
void FUN_0040bd60(unsigned short slot, DeviceInfo *pOut);
extern int g_unk0x00817fe4;
extern unsigned int g_unk0x00817ff4;

void RallyData_ResetSelection(void);
BOOL FUN_004a15a0(void);
void Session_SetOpen(char open);
void FUN_004ea890(BYTE param1);
bool FUN_004f4e80(void);

// Globals the return-to-frontend path of InitializeGame resets.
// GLOBAL: CMR2 0x00819744
int g_unk0x00819744;

// GLOBAL: CMR2 0x00818ce4
BYTE g_unk0x00818ce4;

// Signatures follow the original's `ret N` (stdcall: N/4 arguments).
void FUN_004e9f70(BYTE param1, BYTE param2);
void FUN_004f4ef0(void);
BYTE FUN_004f3b50(void);
BYTE FUN_004f4d20(void);
BYTE FUN_004f4b90(void);
void FUN_004f4910(char registerRelease);
void FUN_004eadb0(void);
void FUN_004ef150(void);
void FUN_004ec2b0(void);
void FUN_004ead10(void);
void FUN_004ea840(void);
void FUN_004f3f60(void);
void FUN_004f3bb0(void);
void FUN_004eb470(void);
void FUN_004ebec0(void);
HRESULT FUN_004a2bd0(int param1);
void FUN_004d5ca0(void);

// Frontend music track ("%s\\select1.adp").
// GLOBAL: CMR2 0x00523d70
char g_strMusicSelect1Adp[16] = "%s\\select1.adp";

void FUN_004ea510(void);

int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);
float FUN_004b23a0(void);
int FUN_0049d3f0(int, int, void *, int, BYTE);
void FUN_0049de40(void);

// FPS overlay format string ("FPS: %.2f").

// Renders the state-5 boot screen: clears both targets, queues the loaded
// frontend texture as a sprite centred over the screen and, when the debug
// flag is on, draws the frame rate.
// FUNCTION: CMR2 0x004d0ea0
void FUN_004d0ea0(Unk0049c2c0 *p1, BYTE p2)
{
    SpriteRect screenRect;
    BYTE colour[4];
    int centre[3];
    SpriteRect src;
    SpriteRect dst;

    screenRect.x = 0;
    screenRect.y = 0;
    screenRect.w = g_pGraphics->resX;
    screenRect.h = g_pGraphics->resY;
    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    centre[0] = 0;
    centre[1] = 0;
    centre[2] = 0;

    CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();

    if (CFrontend::m_unk0x00817fd0 != NULL) {
        src.x = CFrontend::m_unk0x00817fd0->field_0x11c;
        src.y = CFrontend::m_unk0x00817fd0->field_0x11e;
        src.w = CFrontend::m_unk0x00817fd0->width;
        src.h = CFrontend::m_unk0x00817fd0->height;
        dst.x = (int)(g_pGraphics->resX * 43) / 640;
        dst.y = (int)(g_pGraphics->resY * 202) / 480;
        dst.w = src.w;
        dst.h = src.h;
        centre[0] = src.w / 2;
        centre[1] = src.h / 2;
        Sprite_Queue(&src, &dst, CFrontend::m_unk0x00817fd0, 1, 0, centre, NULL, colour, 8);
    }

    Game_PrepareScene(g_unk0x00817fc8, g_unk0x00817fc4, (int)&screenRect, 0);

    if ((g_pGraphics->field913_0x3bc & 4) != 0) {
        sprintf(CFrontend::m_stringDest, g_strFpsFormat0x00516e14, FUN_004b23a0());
        Font_DrawText(0, CFrontend::m_stringDest, 0, 0, (int *)colour, 9);
    }

    FUN_0049d3f0((int)g_unk0x00817fc8, (int)g_unk0x00817fc4, &screenRect, 0, 1);

    if (g_unk0x00817fcc == 0)
        FUN_0049de40();
}

// Waits for the pad button (or 5 s), then restarts the music and asks for state 0.
// FUNCTION: CMR2 0x004d1a90
void FUN_004d1a90(Unk0049c2c0 *p1, BYTE p2)
{
    char path[MAX_PATH];
    DeviceInfo *pDevice;

    g_unk0x00817fe4 = timeGetTime();
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    pDevice = CInput::FUN_0049ead0(0);
    FUN_0040bd60(0, pDevice);
    if ((pDevice->field_0x8 & 0x10) != 0 || (unsigned int)(g_unk0x00817fe4 - FUN_004eaa00()) > 0x1388) {
        FUN_004ea9f0();
        sprintf(path, g_strMusicSelect1Adp, CInstallInfo::GetMusicDir());
        CSound::FUN_004a28d0(path);
        CSound::FUN_004a31f0(CGameInfo::FUN_00405e40());
        FUN_004a2bd0(1);
        CGame::FUN_0049c1c0(p1, p2, 0, 2);
    }
}

// Waits ~5 s, then asks for state 0.
// FUNCTION: CMR2 0x004d1b40
void FUN_004d1b40(Unk0049c2c0 *p1, BYTE p2)
{
    g_unk0x00817fe4 = timeGetTime();
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    FUN_0040bd60(0, CInput::FUN_0049ead0(0));
    if ((unsigned int)(g_unk0x00817fe4 - FUN_004eaa00()) > 0x1388) {
        FUN_004ea9f0();
        CGame::FUN_0049c1c0(p1, p2, 0, 2);
    }
}

// Stores the frame time and runs the frontend.
// FUNCTION: CMR2 0x004d1c90
void FUN_004d1c90(Unk0049c2c0 *p1, BYTE p2)
{
    g_unk0x00817fe4 = timeGetTime();
    g_unk0x00817ff4 = g_unk0x00817fe4 - FUN_004eaa00();
    FUN_004ea510();
}

// Fade step of the boot/HUD colour: 1/1500 per elapsed millisecond.
// GLOBAL: CMR2 0x00513ec8
float g_unk0x00513ec8 = 1.0f / 1500.0f;
extern const float g_netByteScale;
int Sprite_FillRect(int unused, short *pRect, BYTE *pColour, int layer);

// Draws a boot/HUD label at (x, y) in a colour that fades out 2.5 s after the
// frame timer was last reset; unless flag is set it also fills the 2 pixel wide
// bar that follows the text. Returns the x after the bar.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d0d30
int FUN_004d0d30(int x, int y, char *pText, char flag)
{
    int elapsed;
    int alpha;
    BYTE colour[8];
    short rect[4];

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    elapsed = timeGetTime();
    elapsed = elapsed - FUN_004eaa00();
    if (elapsed > 0x9c4) {
        if (elapsed > 0xfa0)
            alpha = 0;
        else
            alpha = 0xff -
                    (int)(__int64)((float)(elapsed - 0x9c4) * g_netByteScale * g_unk0x00513ec8);
    } else {
        alpha = 0xff;
    }
    colour[0] = alpha;
    colour[1] = alpha;
    colour[2] = alpha;
    Font_DrawText(2, pText, x, y, (int *)colour, 0x11);
    if (flag == 0) {
        x += Font_GetTextWidth(2, (BYTE *)pText);
        x += (int)(g_pGraphics->resX * 10) / 0x280;
        rect[0] = (short)x;
        rect[2] = 2;
        rect[1] = (int)(g_pGraphics->resY * 200) / 0x1e0;
        rect[3] = (int)(g_pGraphics->resY * 60) / 0x1e0;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, colour + 4, 1);
        x += (int)(g_pGraphics->resX * 10) / 0x280;
    }
    return x;
}

// Prototypes for this boot state (defined in other modules or still stubs).
unsigned char RallyDataCountryIndex(void);
unsigned char RallyDataStageIndex(void);
int FUN_0050fdf0(char *path, Texture *pTexture, short *pRect, unsigned int flags, unsigned int track);

// Cleared on entry and set while the intro video of the boot sequence is
// being shown.
extern BYTE g_unk0x00817fec;

// Uppercase country codes of the intro videos. They are four bytes apart,
// 0x00519264..0x00519280; the last one is CFrontend::m_strUK (already
// declared in Frontend.h).
extern char g_str0x00519268[4];
extern char g_str0x0051926c[4];
extern char g_str0x00519270[4];
extern char g_str0x00519274[4];
extern char g_str0x00519278[4];
extern char g_str0x0051927c[4];
extern char g_str0x00519280[4];

// Country intro video path template ("%s\\%s\\%s.bik").
// GLOBAL: CMR2 0x00523dc0
char g_str0x00523dc0[14] = "%s\\%s\\%s.bik";

// Boot state 7: shows the frontend for four seconds and then plays the
// country intro video before asking for state 0.
// FUNCTION: CMR2 0x004d1cc0
void FUN_004d1cc0(Unk0049c2c0 *p1, BYTE p2)
{
    char *codes[8] = {
        g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274,
        g_str0x00519270, g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK,
    };
    char path[MAX_PATH];
    DeviceInfo *pDevice;
    bool queuedVideo;

    g_unk0x00817fec = 0;
    queuedVideo = false;
    if (CGame::m_unk0x00523d68 != 0 &&
        (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1) &&
        RallyDataStageIndex() == 0) {
        g_unk0x00817fec = 1;
        CSound::FUN_004a2b50(0);
        g_unk0x00817fe4 = timeGetTime();
        CInput::FUN_0049eab0();
        FUN_0040bad0();
        pDevice = CInput::FUN_0049ead0(0);
        FUN_0040bd60(0, pDevice);
        if (g_unk0x00817fe4 - (int)FUN_004eaa00() > 4000) {
            int language = CGameInfo::GetGameLanguage();
            sprintf(path, g_str0x00523dc0, CInstallInfo::GetCountrySpecificOtherDir(),
                    CGameInfo::GetGameRegionDirectory(), codes[RallyDataCountryIndex() & 0xff]);
            FUN_0050fdf0(path, NULL, NULL, 2, language);
            queuedVideo = true;
        }
        if ((pDevice->field_0x8 & 0x10) == 0 && !queuedVideo)
            return;
    }
    CGame::FUN_0049c1c0(p1, p2, 0, 2);
}

// Boot pieces called by the exit path of the game state that live in other
// modules.
void FUN_004eabc0(void);
void FUN_004ea9c0(unsigned int param1, unsigned int param2);
void FUN_00406820(void);
void RallyData_FUN_004068d0(char param1);
void FUN_0040dc30(void);
bool FUN_004eb3e0(void);
void RallyData_FUN_004ec1a0(void);
unsigned char RallyDataCountryIndex(void);
unsigned char RallyDataStageIndex(void);
extern int g_unk0x00817fe8;

// Byte flag the exit path of the boot states tests to pick the next game
// state (also read by the function at 0x4d2070).
// GLOBAL: CMR2 0x008180fa
BYTE g_unk0x008180fa;

// Unwinds the group of callbacks of the current state, saves the data of the
// ending race and asks for the next state (level 2, state index 0).
// FUNCTION: CMR2 0x004d1e90
void FUN_004d1e90(Unk0049c2c0 *p1, BYTE unused)
{
    CGame::FUN_00406810(0);
    if (CGameInfo::FUN_00406410(0x11))
        CGameInfo::FUN_004d0590(1);
    else
        CGameInfo::FUN_004d0590(0);
    if (CGameInfo::FUN_00405d80() != 4)
        CFrontend::FUN_004cf060();

    CSound::FUN_004a2b50(0);
    FUN_004eabc0();

    if (CGame::m_unk0x008180f9 == 0) {
        if (CGameInfo::FUN_00405d80() != 8 && CGameInfo::FUN_00405d80() != 9 &&
            CGameInfo::FUN_00405d80() != 10 && CGameInfo::FUN_00405d80() != 11 &&
            CGameInfo::FUN_00405d80() != 12)
            FUN_0040dc30();

        FUN_00406820();
        RallyData_FUN_004068d0(-1);
        CGameInfo::FUN_00406580();

        if (CGameInfo::FUN_00406320() == 0) {
            if (CGame::m_unk0x00523d68 != 0 || CGame::m_unk0x008180f9 != 0 || g_unk0x008180fa != 0)
                FUN_004ea9c0(RallyDataCountryIndex() & 0xff, RallyDataStageIndex() & 0xff);

            FUN_004ea840();
            CNetworkLeaderboards::SaveLeaderboards();
            CInput::SaveControllerInfo();
            FUN_004eb3e0();
        }
    }

    CGame::UnwindCallbacks(g_unk0x00817fe8);
    CGraphics::FUN_004a5be0();
    CGraphics::FreeTextureBuffers();
    if (CGameInfo::FUN_00405d80() != 4)
        RallyData_FUN_004ec1a0();

    if (CGame::m_unk0x00523d68 != 0) {
        CGame::FUN_0049c1c0(p1, 0, 0, 2);
        if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1) {
            FUN_004ea9f0();
            CGame::FUN_004057e0(2);
            return;
        }
        if (CGameInfo::FUN_00405d80() != 2 && CGameInfo::FUN_00405d80() != 3 &&
            CGameInfo::FUN_00405d80() != 8 && CGameInfo::FUN_00405d80() != 9 &&
            CGameInfo::FUN_00405d80() != 10) {
            CGame::FUN_004057e0(3);
            CInput::FUN_0040af20();
            return;
        }
        CGame::FUN_004057e0(2);
        CInput::FUN_0040af20();
        return;
    }

    if (CGame::m_unk0x008180f9 != 0) {
        CGame::FUN_0049c1c0(p1, 0, 0, 2);
        CGame::FUN_004057e0(2);
        return;
    }
    if (g_unk0x008180fa != 0) {
        CGame::FUN_0049c1c0(p1, 0, 0, 2);
        CGame::FUN_004057e0(3);
        return;
    }
    CGame::FUN_0049c1c0(p1, 0, 0, 2);
    CGame::SetShouldExit();
}

float FUN_004b23a0(void);
int FUN_0049d3f0(int, int, void *, int, BYTE);
void FUN_0049de40(void);
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param);

// Needed by every boot-screen render of this file (0x4d0a80 / 0x4d0ea0 /
// 0x4d1080 / 0x4d1370); the duplicated definition is deduplicated on integration.

// Bink Video credit line drawn at the bottom of the boot screens.
// GLOBAL: CMR2 0x00523cd8
char g_strBinkCredit0x00523cd8[65] = "Uses Bink Video. Copyright (C) 1997-1999 by RAD Game Tools, Inc.";
// Bink Video credit line of the Polish build.
// GLOBAL: CMR2 0x00523d1c
char g_strBinkCreditPl0x00523d1c[73] = "Wykorzystuje Bink Video. Copyright (C) 1997-1999 by RAD Game Tools, Inc.";

// Renders the boot screens: clears the target white, draws the frontend movie
// frame centred, then the FPS counter and the Bink Video credit line.
// FUNCTION: CMR2 0x004d1370
void FUN_004d1370(Unk0049c2c0 *p1, BYTE p2)
{
    SpriteRect dst;
    SpriteRect viewport;
    SpriteRect src;

    viewport.x = 0;
    viewport.y = 0;
    viewport.w = (short)g_pGraphics->resX;
    viewport.h = (short)g_pGraphics->resY;
    BYTE white[4] = { 0xff, 0xff, 0xff, 0xff };
    BYTE black[4] = { 0x00, 0x00, 0x00, 0xff };
    int centre[3] = { 0, 0, 0 };

    CGraphics::SetClearColour(1, 0xff, 0xff, 0xff);
    CGraphics::SetProjection(0x25645, 0x4326e, 0xfa0000, 0x10000);
    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    if (CFrontend::m_unk0x00817ebc != NULL) {
        short w = CFrontend::m_unk0x00817ebc->width;
        short h = CFrontend::m_unk0x00817ebc->height;

        src.x = CFrontend::m_unk0x00817ebc->field_0x11c;
        src.y = CFrontend::m_unk0x00817ebc->field_0x11e;
        src.w = w;
        src.h = h;
        dst.w = w;
        dst.h = h;
        dst.x = (short)((int)(g_pGraphics->resX * 0x140) / 0x280);
        dst.y = (short)((int)(g_pGraphics->resY * 0xf0) / 0x1e0);
        dst.x -= w / 2;
        dst.y -= h / 2;
        centre[0] = w / 2;
        centre[1] = h / 2;
        Sprite_Queue(&src, &dst, CFrontend::m_unk0x00817ebc, 1, 0, centre, NULL, white, 8);
    }
    Game_PrepareScene(g_unk0x00817fc8, g_unk0x00817fc4, (int)&viewport, 0);
    if ((g_pGraphics->field913_0x3bc & 4) != 0) {
        sprintf(CFrontend::m_stringDest, g_strFpsFormat0x00516e14, FUN_004b23a0());
        Font_DrawText(0, CFrontend::m_stringDest, 0, 0, (int *)white, 9);
    }
    if (CGameInfo::GetGameRegion() == 3)
        sprintf(CFrontend::m_stringDest, g_strBinkCreditPl0x00523d1c);
    else
        sprintf(CFrontend::m_stringDest, g_strBinkCredit0x00523cd8);
    Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x140) / 0x280,
                  (int)(g_pGraphics->resY * 0x15e) / 0x1e0, (int *)black, 10);
    FUN_0049d3f0((int)g_unk0x00817fc8, (int)g_unk0x00817fc4, &viewport, 0, 1);
    if (g_unk0x00817fcc == 0)
        FUN_0049de40();
}

// Boot states 0x4d1ba0/0x4d1c30/0x4d1e10 (cascade seed batch 1, verified 96-98%).
// GLOBAL: CMR2 0x00523da4
char g_strCmBik[] = "%s\\cm.bik";
// GLOBAL: CMR2 0x00523db0
char g_strIntroBik[] = "%s\\Intro.bik";
// GLOBAL: CMR2 0x00523d6c
int g_unk0x00523d6c = -1;
// GLOBAL: CMR2 0x00817ff0
unsigned int g_unk0x00817ff0;

// FUNCTION: CMR2 0x004d1ba0
void FUN_004d1ba0(Unk0049c2c0 *p1, BYTE state)
{
    char path[260];

    CGraphics::SetClearColour(1, 0, 0, 0);
    CGraphics::ClearTarget();
    g_pGraphics->pPrimarySurface->Blt(NULL, g_pGraphics->pBackBufferSurface, NULL, DDBLT_WAIT, NULL);
    sprintf(path, g_strCmBik, CInstallInfo::GetVideosDir());
    FUN_0050fdf0(path, NULL, NULL, 2, 0);
    FUN_004ea9f0();
    CGame::FUN_0049c1c0(p1, state, 0, 2);
}

// FUNCTION: CMR2 0x004d1c30
void FUN_004d1c30(Unk0049c2c0 *p1, BYTE state)
{
    char path[260];

    sprintf(path, g_strIntroBik, CInstallInfo::GetVideosDir());
    FUN_0050fdf0(path, NULL, NULL, 2, 0);
    FUN_004ea9f0();
    CGame::FUN_0049c1c0(p1, state, 0, 2);
}

// FUNCTION: CMR2 0x004d1e10
void FUN_004d1e10(Unk0049c2c0 *p1, BYTE state)
{
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    CInput::FUN_0049ead0(0);
    if (g_unk0x00523d6c == -1) {
        g_unk0x00817ff0 = CMain::GetFrameDelta();
        g_unk0x00523d6c = 0;
    }
    if (500 < (unsigned int)(CMain::GetFrameDelta() - g_unk0x00817ff0)) {
        if (g_unk0x00523d6c < 2) {
            g_unk0x00817ff0 = CMain::GetFrameDelta();
            g_unk0x00523d6c = g_unk0x00523d6c + 1;
            return;
        }
        CGame::FUN_0049c1c0(p1, state, 0, 2);
    }
}

// FUNCTION: CMR2 0x004d15e0
void CGame::InitializeGame(Unk0049c2c0 *p1, BYTE p2)
{
    time_t srandSeed;
    char *skuValue, *skuRegion;
    bool didLoadGameInfo = false;

    srandSeed = time(NULL);
    srand(srandSeed);

    // europe sku check
    skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
    skuRegion = CRegKey::m_skuEurope;
    if (strcmp(skuRegion, skuValue) == 0)
       CGameInfo::SetGameRegion(0);
    else
    {
       // america sku check
       skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
       skuRegion = CRegKey::m_skuAmerica;
       if (strcmp(skuRegion, skuValue) == 0)
           CGameInfo::SetGameRegion(1);
       else
       {
           // japan sku check
           skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
           skuRegion = CRegKey::m_skuJapan;
           if (strcmp(skuRegion, skuValue) == 0)
               CGameInfo::SetGameRegion(2);
           else
           {
               // poland sku check
               skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
               skuRegion = CRegKey::m_skuPoland;
               if (strcmp(skuRegion, skuValue) == 0)
                   CGameInfo::SetGameRegion(3);
               else // otherwise die
                   CGame::SetShouldExit();
           }
       }
    }

    CGameInfo::FUN_004f4b40();
    m_unk0x00523d68 = 1;
    m_unk0x008180f9 = 0;
    m_unk0x008180fc = FUN_004ea880();
    if (FUN_004ea880() != 0)
    {
        // First boot: load the configuration, bring up DirectX, sound and the frontend.
        FUN_004083e0(0);
        FUN_00406810(0);
        FUN_004067e0();
        CGameInfo::FUN_00405de0(0);
        if (CInstallInfo::FUN_0040e8d0() == 0)
            goto exit;

        CGameInfo::FUN_00510410();
        CGameInfo::FUN_00406560();
        CGameInfo::FUN_00406580();
        CGameInfo::SetupInputs(2);
        didLoadGameInfo = CGameInfo::LoadGameInfo();
        CNetworkLeaderboards::Reset();
        CNetworkLeaderboards::LoadLeaderboards();
        CGameInfo::FUN_00405de0(0);
        if (CGraphics::InitializeDirectX())
            CGraphics::SetDefaults();
        CInput::LoadControllerInfo();
        CGameInfo::FUN_0049ea90(1);
        CGameInfo::FUN_004d05d0();
        if (FUN_004aaa40() == false)
            goto exit;
        if (CGame::FUN_004d0a50(true) == false)
            goto exit;

        CGraphics::SetClearColour(1, 0, 0, 0);
        FUN_004f4ef0();
        CGame::RegisterCallback(FUN_004f4e80, NULL);
        Sound_Init(0x5622, 2, 0x10, 0);
        CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
        FUN_004f3b50();
        if (FUN_004f4d20() == 0)
            goto exit;
        if (FUN_004f4b90() == 0)
            goto exit;
        FUN_004f4910(1);
        RallyData_ResetSelection();
        FUN_004eadb0();
        FUN_004ef150();
        FUN_004e9f70(didLoadGameInfo == false, 1);
        FUN_004f3f60();
        FUN_004f3bb0();
        FUN_004ea890(0);
        RallyData_FUN_00408290();
        RallyData_FUN_0040df30();
        CGame::FUN_0049c1c0(p1, p2, 1, 2);
        FUN_004ea9f0();
        FUN_004d5ca0();
        return;
    }

    // Back from a race: save state, reload the frontend and restart its music.
    if (CGameInfo::FUN_00405d80() == 4)
        g_unk0x00819744 = 0;
    FUN_00406810(0);
    if (FUN_004067e0())
        CGameInfo::FUN_00405de0(0);
    else
    {
        if (CGameInfo::FUN_00405e00() != 0 &&
            (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12))
            FUN_00406810(1);
        if (FUN_004a15a0())
            Session_SetOpen(1);
        g_unk0x00818ce4 = 0;
        FUN_004ec2b0();
    }
    if (CGameInfo::FUN_00406320())
    {
        FUN_004ead10();
        CGameInfo::FUN_00406330(0);
    }
    CGameInfo::FUN_00406580();
    FUN_004ea840();
    CNetworkLeaderboards::SaveLeaderboards();
    CInput::SaveControllerInfo();
    CGameInfo::FUN_0049ea90(1);
    if (CGame::FUN_004d0a50(false) == false)
        goto exit;

    if (FUN_004067b0() == 0)
        CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
    else
        CGraphics::SetClearColour(1, 0, 0, 0);
    FUN_004f4ef0();
    CGame::RegisterCallback(FUN_004f4e80, NULL);
    Sound_Init(0x5622, 2, 0x10, 0);
    FUN_004f3b50();
    if (FUN_004f4d20() == 0)
        goto exit;
    if (FUN_004f4b90() == 0)
        goto exit;
    FUN_004f4910(1);
    FUN_004ef150();
    FUN_004e9f70(0, 0);
    FUN_004f3f60();
    FUN_004f3bb0();
    sprintf(CFrontend::m_stringDest, g_strMusicSelect1Adp, CInstallInfo::GetMusicDir());
    CSound::FUN_004a28d0(CFrontend::m_stringDest);
    CSound::FUN_004a31f0(CGameInfo::FUN_00405e40());
    FUN_004a2bd0(1);
    FUN_004eb470();
    if (CGameInfo::FUN_00405e00() == 0 && FUN_004067b0() == 0)
        FUN_004ebec0();
    RallyData_FUN_00408290();
    RallyData_FUN_0040df30();
    FUN_004ea9f0();
    if (FUN_004067b0() == 1) {
        FUN_004067c0(0);
        CGame::FUN_0049c1c0(p1, p2, 6, 2);
    } else {
        CGame::FUN_0049c1c0(p1, p2, 0, 2);
    }
    FUN_004d5ca0();
    return;

exit:
    CGame::SetShouldExit();
}

// FUNCTION: CMR2 0x0049c2c0
void CGame::FUN_0049c2c0(Unk0049c2c0 *param1)
{
    FuncTableEntry func;
    BYTE counter;
    BYTE unkIx;

    counter = 0;
    unkIx = 0;
    if (param1->count > 0)
    {
        do
        {
            func = param1->funcLookupTable[param1->unk[unkIx].field0x1 & 0xFF].func1;
            if (func != NULL)
                (func)(param1, unkIx);

            counter++;
            unkIx = counter;
        } while (counter < param1->count);
    }
}

// FUNCTION: CMR2 0x0049c310
void CGame::FUN_0049c310(Unk0049c2c0 *param1)
{
    OtherFuncTableEntry func;
    BYTE counter;
    BYTE unkIx;

    if (m_unk0x00593cac == 0)
    {

        counter = 0;
        unkIx = 0;
        if (param1->count > 0)
        {
            do
            {
                func = param1->funcLookupTable[param1->unk[unkIx].field0x1 & 0xFF].func2;
                if (func != NULL)
                    (func)(param1, unkIx);

                counter++;
                unkIx = counter;
            } while (counter < param1->count);
        }
    }
    else
        m_unk0x00593cac = 0;
}

// FUNCTION: CMR2 0x0049c370
void CGame::FUN_0049c370(Unk0049c2c0 *param1)
{
    unsigned int tVar1;

    m_unk0x00593ba8.index = 0;
    if (param1->count > 0)
    {
        do
        {
            m_unk0x00593ba4 = &param1->unk[m_unk0x00593ba8.packed & 0xff];
            tVar1 = m_unk0x00593ba4->field0x1;
            if (tVar1 & 0x3000000)
            {
                m_unk0x00593ba4->field0x1 = ((tVar1 >> 0x10) & 0xff) | (tVar1 & 0xffffff00);
                m_unk0x00593ba4->field0x1 = m_unk0x00593ba4->field0x1 & 0xff00ffff;
                m_unk0x00593ba4->field0x1 = m_unk0x00593ba4->field0x1 & 0xfcffffff;
                m_unk0x00593ba4->field0x2 = 0;
            }
            else
                m_unk0x00593ba4->field0x2 = m_unk0x00593ba4->field0x2 + 1;

            m_unk0x00593ba8.index++;
        } while (m_unk0x00593ba8.index < param1->count);
    }
}

// FUNCTION: CMR2 0x0049c150
void CGame::FUN_0049c150(Unk00817d98 *param1, int param2, int param3)
{
    param1->field0x2 = 0;
    param1->bits.state = (BYTE)param2;
    param1->bits.value = (BYTE)param3;
    param1->bits.rule = 0;
    param1->bits.level = 0;
}

// FUNCTION: CMR2 0x0049c190
void CGame::FUN_0049c190(Unk0049c2c0 *p1, BYTE count, Unk00817d98 *unk, FuncTableGroup *funcLookupTable, void *unk2)
{
    p1->count = count;
    p1->unk = unk;
    p1->funcLookupTable = funcLookupTable;
    p1->unk2 = unk2;
}

// Promotes entry index of the table to the given level when a rule of
// p->unk2 (terminated by 0xffffffff, 0xff bytes are wildcards) matches it
// with the given value; the rule's top byte becomes the entry's third byte.
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049c1c0
int CGame::FUN_0049c1c0(Unk0049c2c0 *p, BYTE index, BYTE value, int level)
{
    unsigned int *pEntry;
    unsigned int *pRule;
    unsigned int entry;
    unsigned int rule;

    pEntry = (unsigned int *)&p->unk[index];
    entry = *pEntry;
    if ((entry & 0x3000000) == 0x3000000 || (int)((entry >> 24) & 3) < level) {
        for (pRule = (unsigned int *)p->unk2;; pRule++) {
            rule = *pRule;
            if ((rule & 0xff) == 0xff && (rule & 0xff00) == 0xff00 && (rule & 0xff0000) == 0xff0000 &&
                (rule & 0xff000000) == 0xff000000)
                break;
            if (((BYTE)(rule ^ entry) == 0 || (rule & 0xff) == 0xff) &&
                ((BYTE)((rule ^ entry) >> 8) == 0 || (rule & 0xff00) == 0xff00) && ((rule >> 16) & 0xff) == value) {
                *pEntry = ((level & 3) << 24) | (entry & 0xfcffffff);
                *pEntry = ((*pRule >> 8) & 0xff0000) | ((level & 3) << 24) | (entry & 0xfc00ffff);
                return 1;
            }
        }
    }
    return 0;
}
// GLOBAL: CMR2 0x0082a7f0
Unk0049c2c0 g_unk0x0082a7f0;

// FUNCTION: CMR2 0x004ff440
Unk0049c2c0 *FUN_004ff440(void)
{
    return &g_unk0x0082a7f0;
}
// GLOBAL: CMR2 0x0082a800
Unk00817d98 g_unk0x0082a800;
// GLOBAL: CMR2 0x0082a908
BYTE g_unk0x0082a908;
// State 0x500df0 (country menus) is defined in GameInfo.cpp.
void FUN_00500df0(Unk0049c2c0 *p1, BYTE p2);

// State of the option menu machine (GameInfo.cpp).
void FUN_00500c80(Unk0049c2c0 *p1, BYTE state);

void FUN_00500f80(Unk0049c2c0 *p1, BYTE state);
void FUN_00501780(int param1, int unused);
void FUN_00501920(int param1, int unused);

// GLOBAL: CMR2 0x00526ee0
FuncTableGroup g_unk0x00526ee0[7] = {
    {FUN_00500c80, NULL},
    {FUN_005012e0, (OtherFuncTableEntry)FUN_00501780},
    {FUN_00500df0, NULL},
    {FUN_00500f80, FUN_005015d0},
    {(FuncTableEntry)FUN_00501350, (OtherFuncTableEntry)FUN_00501920},
    {FUN_005010a0, CGame::FUN_00501680},
    {FUN_00501130, CGame::FUN_00501680},
};
// Magic value handed to the callback machine as opaque data (0x0100ff00), not
// an address, so it must not be typed as a pointer.
// GLOBAL: CMR2 0x00526f18
unsigned int g_unk0x00526f18 = 0x0100ff00;

// FUNCTION: CMR2 0x004ff450
BOOL CGame::FUN_004ff450()
{
    if (g_unk0x0082a908 != 0) {
        FUN_0049c2c0(&g_unk0x0082a7f0);
        FUN_0049c310(&g_unk0x0082a7f0);
        FUN_0049c370(&g_unk0x0082a7f0);
        return FALSE;
    }
    FUN_0049c150(&g_unk0x0082a800, 0, 0xFF);
    FUN_0049c190(&g_unk0x0082a7f0, 1, &g_unk0x0082a800, g_unk0x00526ee0, &g_unk0x00526f18);
    g_unk0x0082a908 = 1;
    return FALSE;
}
// In-race callback group of the game state machine (0x0041b060), filled by
// FUN_0049c190; its slots live at 0x537de0.
extern BYTE g_unk0x00537dd0[0x20];
extern BYTE g_unk0x00537ef4;
extern BYTE g_unk0x00537ef5;
extern int g_unk0x00537ef8;
extern int g_unk0x00537efc;
extern int g_unk0x00537df0;

void FUN_00404f40(Unk0049c2c0 *param1);
void FUN_00405470(BYTE *param1);
void FUN_0041b360(void);
unsigned int *RallyData_GetChampionshipState(void);
char RallyData_FUN_00408500(BYTE param1);
int FUN_00407270(void);
void RallyData_FUN_00407500(BYTE param1);
void FUN_0042b660(int count);
void Dash_Reset(void);
void FUN_0041bf80(int param1, int param2);
void FUN_0041f420(Unk0049c2c0 *p, BYTE index);
void FUN_00411070(int param1, int param2);
void FUN_0041c0e0(int param1, int param2);
void FUN_00410c80(int param1, char param2);
void FUN_00401000(BYTE *param1, int param2);
void FUN_004010a0(BYTE *param1, int param2);
void FUN_00401150(int param1, int param2);
void FUN_00410de0(BYTE *param1, unsigned int param2);
void FUN_00472e00(BYTE *param_1, unsigned int param_2);
void FUN_004759d0(int unused1, int unused2);
void FUN_0041d2b0(BYTE *param1, unsigned int param2);
void FUN_0041d7a0(int param1, unsigned int param2);
void FUN_0041db10(BYTE *param1, unsigned int param2);
void FUN_0041e8d0(BYTE *param1, unsigned int param2);
void FUN_0041e350(int param1, unsigned int param2);
void FUN_0041f4e0(Unk0049c2c0 *p, BYTE index);
void FUN_0041e5c0(int param1, char param2);
void FUN_00410e20(BYTE *param1, unsigned int param2);
void FUN_00410e70(BYTE *param1, unsigned int param2);
void FUN_00410ee0(BYTE *param1, unsigned int param2);
void FUN_00411020(BYTE *param1, unsigned int param2);
void FUN_00410fa0(BYTE *param1, unsigned int param2);

// In-race state table: pairs of (update, render) callbacks indexed by the state
// of each slot of the callback group (0x537dd0).
// GLOBAL: CMR2 0x005190b0
FuncTableGroup g_unk0x005190b0[14] = {
    {(FuncTableEntry)FUN_0041bf80, NULL},
    {(FuncTableEntry)FUN_0041f420, (OtherFuncTableEntry)FUN_00411070},
    {(FuncTableEntry)FUN_0041c0e0, (OtherFuncTableEntry)FUN_00410c80},
    {(FuncTableEntry)FUN_00401000, NULL},
    {(FuncTableEntry)FUN_004010a0, NULL},
    {(FuncTableEntry)FUN_00401150, (OtherFuncTableEntry)FUN_00410de0},
    {(FuncTableEntry)FUN_00472e00, (OtherFuncTableEntry)FUN_004759d0},
    {(FuncTableEntry)FUN_0041d2b0, (OtherFuncTableEntry)FUN_00410e20},
    {(FuncTableEntry)FUN_0041d7a0, (OtherFuncTableEntry)FUN_00410e70},
    {(FuncTableEntry)FUN_0041db10, (OtherFuncTableEntry)FUN_00410ee0},
    {(FuncTableEntry)FUN_0041e350, (OtherFuncTableEntry)FUN_00411020},
    {(FuncTableEntry)FUN_0041e8d0, (OtherFuncTableEntry)CGame::FUN_00501680},
    {(FuncTableEntry)FUN_0041f4e0, NULL},
    {(FuncTableEntry)FUN_0041e5c0, (OtherFuncTableEntry)FUN_00410fa0},
};

// State transition rules of the in-race machine (byte 0 = current state, 0xff
// any; byte 1 = slot level, 0xff any; byte 2 = value; byte 3 = next state),
// terminated by 0xffffffff.
// GLOBAL: CMR2 0x00519120
unsigned int g_unk0x00519120[25] = {
    0x0100ff00, 0x0200ff01, 0x0c01ff01, 0x0300ff02, 0x0400ff03, 0x0500ff04,
    0x0700ff05, 0x0605ff05, 0x0d06ff05, 0x0a01ff05, 0x0700ff06, 0x0800ff07,
    0x0900ff08, 0x0a00ff09, 0x0b00000a, 0x0b00010a, 0x0b00040a, 0x0b00020a,
    0x0b00030a, 0x0000ff0b, 0x0407ff0b, 0x0403ff0b, 0x0000ff0c, 0x0504ffff,
    0x0b01ffff
};

// Entry point of the in-race state machine: on the first frame it builds the
// callback group of the current in-race mode, on later frames it only
// dispatches the transition of the current state.
//
// The logic is faithful; the remaining gap is codegen shape. Exact differences
// against the original (643 bytes):
//  1. switch block layout (the bulk). The original puts the switch epilogue
//     (`mov [esp+0x14],2; jmp state1`) right after case 1 (0x41b190), so every
//     `break` reaches it with a short jump and case 2's second FUN_00408500 call
//     is tail-merged into case 1's (`jmp 0x41b187`). MSVC6 12.00.8804 from any
//     source shape probed instead keeps an epilogue copy after every case and
//     duplicates case 2's call. Case 0 keeps its own copy of the epilogue in the
//     original (0x41b14e), reproduced here with the explicit `goto state1`.
//  2. stack slots are swapped: original has `state` at [esp+0x10] and `level` at
//     [esp+0x14]; ours is the reverse (9 operand mismatches). Declaration order
//     does not move them.
//  3. the loop's else branch has a dead `mov al,[g_unk0x00537ef4]; test al,al`
//     before `push 0` that no source form probed reproduces (2 instructions).
//  4. the loop counter is a dword in the original (`mov ebx,ebp; and ebx,0xff;
//     dec ebx`); the byte `i` here compiles to `dec bl`.
//  5. `level` is masked into a copy in the original (`mov ecx,edi; and ecx,0xff`)
//     keeping the unmasked value in edi for the call argument; ours masks edi.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0041b060
BOOL CGame::FUN_0041b060(void)
{
    unsigned int *pState;
    Unk00817d98 *pSlot;
    BYTE level;
    BYTE state;
    BYTE i;
    char c;

    if (g_unk0x00537ef4 != 0 && g_unk0x00537ef5 == 0 && g_unk0x00537ef8 == 0) {
        if (CGameInfo::FUN_00404f20() != 0) {
            FUN_00404f40((Unk0049c2c0 *)g_unk0x00537dd0);
            FUN_00405470(g_unk0x00537dd0);
            FUN_0049c370((Unk0049c2c0 *)g_unk0x00537dd0);
            return FALSE;
        }
        FUN_0049c2c0((Unk0049c2c0 *)g_unk0x00537dd0);
        FUN_0049c310((Unk0049c2c0 *)g_unk0x00537dd0);
        FUN_0049c370((Unk0049c2c0 *)g_unk0x00537dd0);
        return FALSE;
    }
    pState = RallyData_GetChampionshipState();
    if (g_unk0x00537ef4 == 0)
        FUN_0041b360();
    state = CGameInfo::FUN_00405d70();
    c = (char)CGameInfo::FUN_00405d80();
    if (c == 4) {
        switch ((int)((*pState >> 3) & 7) - 1) {
        case 0:
            if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] & 0x1f) == -1) {
                if (RallyData_FUN_00408500((pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] >> 5) & 0x1f) == -1)
                    goto fail;
                level = 2;
                goto state1;
            }
            break;
        case 1:
            if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 10] & 0x1f) == -1) {
                if (RallyData_FUN_00408500((pState[((*pState >> 0xc) & 0xf) * 3 + 10] >> 5) & 0x1f) == -1)
                    goto fail;
            }
            break;
        case 2:
            if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 4] & 0x1f) == -1) {
                if (RallyData_FUN_00408500((pState[((*pState >> 0xc) & 0xf) * 3 + 4] >> 5) & 0x1f) == -1)
                    goto fail;
            }
            break;
        case 3:
            if (RallyData_FUN_00408500((BYTE)pState[1] & 0x1f) == -1) {
                if (RallyData_FUN_00408500((pState[1] >> 5) & 0x1f) == -1)
                    goto fail;
            }
            break;
        default:
fail:
            level = 3;
            state = 2;
            goto done;
        }
        level = 2;
    } else {
        if ((BYTE)FUN_00407270() == 0 && state > 1) {
            if (CGameInfo::FUN_00405d80() != 3 && 2 >= state && CGameInfo::FUN_00405da0() == 0) {
                level = 1;
                state = 2;
                goto done;
            }
            level = 4;
        } else {
            level = 0;
        }
    }
state1:
    state = 1;
done:
    RallyData_FUN_00407500(state);
    g_unk0x00537df0 = level;
    if (state > 0) {
        pSlot = (Unk00817d98 *)&g_unk0x00537dd0[0x10];
        i = state;
        do {
            if (g_unk0x00537ef8 != 0) {
                g_unk0x00537efc = 1;
                FUN_0049c150(pSlot, 5, level);
            } else {
                FUN_0049c150(pSlot, 0, level);
            }
            pSlot++;
            i--;
        } while (i != 0);
    }
    FUN_0049c190((Unk0049c2c0 *)g_unk0x00537dd0, state, (Unk00817d98 *)&g_unk0x00537dd0[0x10],
                 g_unk0x005190b0, g_unk0x00519120);
    if (g_unk0x00537ef8 != 0) {
        FUN_0042b660(2);
        Dash_Reset();
    }
    g_unk0x00537ef4 = 1;
    g_unk0x00537ef5 = 0;
    g_unk0x00537ef8 = 0;
    return FALSE;
}


// FUNCTION: CMR2 0x00501680
void CGame::FUN_00501680(struct Unk0049c2c0 *, BYTE) { return; }

extern int g_unk0x0082b1b0;

void FUN_004bad40(int *pOut, FixVector *pPoint, BYTE *pView);

// Projects a 16.16 point through the option menu's background camera and scales
// the result by 2/3.
// FUNCTION: CMR2 0x00501690
void FUN_00501690(int *param1, FixVector *param2)
{
    FUN_004bad40(param1, param2, (BYTE *)g_unk0x0082b1b0);
    param1[0] = FixMul(param1[0], FixDiv(0x20000, 0x30000));
    param1[1] = FixMul(param1[1], FixDiv(0x20000, 0x30000));
}

// Saves the game configuration (Configuration\GameInfo.rcf), keeping the
// current fullscreen setting.
// FUNCTION: CMR2 0x004ea840
void FUN_004ea840(void)
{
    CGameInfo::SetFullscreen((BYTE)g_pGraphics->isFullscreen);
    sprintf(CFrontend::m_stringDest, CGameInfo::m_stringGameInfoRCF, CInstallInfo::GetGameHDPath());
    CInstallInfo::WriteFileToDisk(CFrontend::m_stringDest, 0, &CGameInfo::m_gameInfo, sizeof(GameInfo));
}

// FUNCTION: CMR2 0x004ea880
BYTE CGame::FUN_004ea880(void) { return m_unk0x00516120; }

// FUNCTION: CMR2 0x004ea890
void FUN_004ea890(BYTE param1)
{
    CGame::m_unk0x00516120 = param1;
}

// FUNCTION: CMR2 0x004083e0
void CGame::FUN_004083e0(BYTE param1)
{
    m_unk0x00531768 = param1;
}

// GLOBAL: CMR2 0x0052ea50
BYTE g_unk0x0052ea50;

// FUNCTION: CMR2 0x004067b0
BYTE FUN_004067b0(void)
{
    return g_unk0x0052ea50;
}

// FUNCTION: CMR2 0x004067c0
void FUN_004067c0(BYTE param1)
{
    g_unk0x0052ea50 = param1;
}

// FUNCTION: CMR2 0x004067d0
void FUN_004067d0(void)
{
    CGame::m_unk0x0052ea58 = 1;
}

// FUNCTION: CMR2 0x00406800
BYTE FUN_00406800(void)
{
    return CGame::m_unk0x0052ea59;
}

// FUNCTION: CMR2 0x00406810
void CGame::FUN_00406810(BYTE param1)

{
    m_unk0x0052ea59 = param1;
    return;
}

// FUNCTION: CMR2 0x004067e0
bool CGame::FUN_004067e0(void)
{
    if (m_unk0x0052ea58 != 0)
    {
        m_unk0x0052ea58 = 0;
        return true;
    }
    return false;
}

// FUNCTION: CMR2 0x0049c0a0
int CGame::RegisterCallback(void *param1, void *param2) {
    int iVar2;
    void **piVar3;

    if (param1 == NULL)
        return -1;

    iVar2 = 0;
    if (m_unk0x00593ba0 > 0) {
        piVar3 = m_callbacks;
        while (1) {
            if (param1 == *piVar3)
                return iVar2;
    
            iVar2++;
            piVar3++;
            
            if (iVar2 >= m_unk0x00593ba0) break;
        }
    }

    if (m_unk0x00593ba0 >= 0x40)
        return -1;

    m_callbacks[m_unk0x00593ba0] = param1;
    return m_unk0x00593ba0++;
}

// FUNCTION: CMR2 0x004a17b0
void CGame::FUN_004a17b0(void) {
    m_unk0x005a1fbc = FALSE;
    if (m_unk0x005a1fb8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_unk0x005a1fb8);
        m_unk0x005a1fb8 = NULL;
    }    
}

// The original calls this wrapper instead of FUN_004a17b0 from CGame::Cleanup;
// it compiles to a five byte tail jump.
// FUNCTION: CMR2 0x004a17e0
void CGame::FUN_004a17e0(void) {
    FUN_004a17b0();
}

// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a17f0
void CGame::FUN_004a17f0(bool param1) {
    if (param1)
        m_unk0x005a1fc0 = false;

    // The original walks a pointer to field_0x64 and stops when it passes
    // 0x5a1e34, which clears exactly the 7 entries (0x5a1820..0x5a1dd0).
    char *pLongName = m_unk0x005a1820[0].field_0x64;
    do {
        Unk0x005a1820 *dest = (Unk0x005a1820 *)(pLongName - 0x64);
        sprintf(dest->field_0x64, CMain::m_logFileBlankLine);
        sprintf(dest->field_0x0, CMain::m_logFileBlankLine);
        dest->field_0xc8 = 0;
        dest->field_0xcc = 0;
        pLongName += sizeof(Unk0x005a1820);
    } while ((int)pLongName < (int)&m_unk0x005a1e34);

    m_unk0x005a1818 = 0;
}

// FUNCTION: CMR2 0x004a1a90
BOOL CGame::FUN_004a1a90(void) {
    IDirectPlay4A *pVar1;
    HRESULT hr;
    if (m_unk0x005a1fc0) {
        pVar1 = GetDirectPlay();
        if (pVar1 != NULL) {
            hr = pVar1->DestroyPlayer(m_unk0x005a1ea0);
            if (hr > DPERR_UNAVAILABLE && hr != DPERR_CONNECTIONLOST && hr == DP_OK)
                return TRUE;
        }
    }

    return FALSE;
}

// FUNCTION: CMR2 0x004aaa10
void CGame::FUN_004aaa10(void) {
    int i;

    for (i = 0; i < 10; i++) {
        if (m_connections[i].pConnection != NULL) {
            CFileBuffer::FreeGenericFileBuffer(m_connections[i].pConnection);
            m_connections[i].pConnection = NULL;
        }
    }
}

// FUNCTION: CMR2 0x004aaac0
bool CGame::Cleanup(void)
{
  FUN_004a1a90();
  DestroyDirectPlayLobby();
  DestroyDirectPlay();
  FUN_004aaa10();
  FUN_004a17e0();
  CoUninitialize();
  return 1;
}

// FUNCTION: CMR2 0x004aab40
void CGame::DestroyDirectPlay(void) {
    if (m_pDirectPlay4A != NULL) {
        m_pDirectPlay4A->Release();
        m_pDirectPlay4A = NULL;
    }
}

// FUNCTION: CMR2 0x004aabb0
void CGame::DestroyDirectPlayLobby(void) {
    if (m_pDirectPlayLobby3A != NULL) {
        m_pDirectPlayLobby3A->Release();
        m_pDirectPlayLobby3A = NULL;
    }
}

// FUNCTION: CMR2 0x004aad40
IDirectPlay4A* CGame::GetDirectPlay(void) {
    return m_pDirectPlay4A;
}

// FUNCTION: CMR2 0x004aaa40
bool CGame::FUN_004aaa40(void) {
    int i;

    m_pDirectPlay4A = NULL;
    m_pDirectPlayLobby3A = NULL;

    CGameInfo::FUN_004a0c60();
    FUN_004a17f0(true);
    FUN_004a17b0();
    
    for (i = 0; i < 10; i++) {
        sprintf(m_connections[i].name, CMain::m_logFileBlankLine);
        m_connections[i].pConnection = NULL;
    }

    m_connectionCount = 0;
    CoInitialize(NULL);

    RegisterCallback(Cleanup, NULL);

    return true;
}

// FUNCTION: CMR2 0x004d0a50
bool CGame::FUN_004d0a50(bool param1) {
    BOOL didLoadSplashScreens;
    BOOL bVar2;

    didLoadSplashScreens = CFrontend::LoadSplashScreens(param1);
    if (didLoadSplashScreens != FALSE) {
        Font_SetBlendMode(TRUE);
        FUN_004e2e50();
        
        return true;
    }

    return false;
}

// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004e2e50
void CGame::FUN_004e2e50(void) {
    char countryCodes[8][10];
    char countryNames[8][10];
    BOOL bZero = false;

    memcpy(countryCodes[0], CFrontend::m_strFin, sizeof(countryCodes[0]));
    memcpy(countryCodes[1], CFrontend::m_strGre, sizeof(countryCodes[1]));
    memcpy(countryCodes[2], CFrontend::m_strFra, sizeof(countryCodes[2]));
    memcpy(countryCodes[3], CFrontend::m_strSwe, sizeof(countryCodes[3]));
    memcpy(countryCodes[4], CFrontend::m_strAus, sizeof(countryCodes[4]));
    memcpy(countryCodes[5], CFrontend::m_strKen, sizeof(countryCodes[5]));
    memcpy(countryCodes[6], CFrontend::m_strIta, sizeof(countryCodes[6]));
    memcpy(countryCodes[7], CFrontend::m_strUK, sizeof(countryCodes[7]));

    memcpy(countryNames[0], CFrontend::m_strFinland, sizeof(countryNames[0]));
    memcpy(countryNames[1], CFrontend::m_strGreece, sizeof(countryNames[1]));
    memcpy(countryNames[2], CFrontend::m_strFrance, sizeof(countryNames[2]));
    memcpy(countryNames[3], CFrontend::m_strSweden, sizeof(countryNames[3]));
    memcpy(countryNames[4], CFrontend::m_strAus, sizeof(countryNames[4]));
    memcpy(countryNames[5], CFrontend::m_strKenya, sizeof(countryNames[5]));
    memcpy(countryNames[6], CFrontend::m_strItaly, sizeof(countryNames[6]));
    memcpy(countryNames[7], CFrontend::m_strUK, sizeof(countryNames[7]));

    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesAr640ATGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pAr640ATexture = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);

    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesAr640DTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pAr640DTexture = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);

    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesLgMatrixTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pLgMatrixTexture = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);

    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesSmMatrixTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pSmMatrixTexture = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);

    // Load country banners and flags
    for (int i = 0; i < 8; i++) {
        sprintf(CFrontend::m_stringDest, CFrontend::m_strSetupRepTexturesBanners, CInstallInfo::GetGameCDPath(), countryCodes[i]);
        CFrontend::m_pSetupRepBanners[i] = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);
        
        sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTinyFlags, CInstallInfo::GetGameCDPath(), countryNames[i]);
        CFrontend::m_pTinyFlags[i] = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);
    }

    // Load medal textures
    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesTGoldTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pTGold = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);
    
    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesTSilverTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pTSilver = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);
    
    sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesTBronzeTGA, CInstallInfo::GetGameCDPath());
    CFrontend::m_pTBronze = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, false, NULL, bZero, bZero);
    
    CFrontend::FUN_004d2590();
}

// FUNCTION: CMR2 0x004a9b00
BOOL CGame::IsActive(void)
{
    return m_isActive;
}

// FUNCTION: CMR2 0x004a9b10
void CGame::FUN_004a9b10(int param1)
{
    m_unk0x00663dc4 = param1;
}

// FUNCTION: CMR2 0x004a9b20
int CGame::FUN_004a9b20(void)
{
    return m_unk0x00663dc4;
}

// FUNCTION: CMR2 0x004aaaf0
bool CGame::CreateDirectPlay(void)
{
    HRESULT hr;
    LPVOID pInterface;

    pInterface = NULL;
    hr = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, &pInterface);
    if (hr != CLASS_E_NOAGGREGATION && hr != REGDB_E_CLASSNOTREG && hr == S_OK) {
        m_pDirectPlay4A = (IDirectPlay4A *)pInterface;
        return true;
    }
    return false;
}

// FUNCTION: CMR2 0x004aab60
bool CGame::CreateDirectPlayLobby(void)
{
    HRESULT hr;
    LPVOID pInterface;

    pInterface = NULL;
    hr = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, &pInterface);
    if (hr != CLASS_E_NOAGGREGATION && hr != REGDB_E_CLASSNOTREG && hr == S_OK) {
        m_pDirectPlayLobby3A = (IDirectPlayLobby3A *)pInterface;
        return true;
    }
    return false;
}

// FUNCTION: CMR2 0x004aa8e0
int __cdecl CGame::CompareConnections(const void *a, const void *b)
{
    if (((DPlayConnection *)a)->guidSP == DPSPGUID_IPX)
        return -1;
    return ((DPlayConnection *)b)->guidSP == DPSPGUID_IPX;
}

// FUNCTION: CMR2 0x004aa880
void CGame::ClearConnections(void)
{
    int i;

    for (i = 0; i < 10; i++) {
        sprintf(m_connections[i].name, CMain::m_logFileBlankLine);
        if (m_connections[i].pConnection != NULL) {
            CFileBuffer::FreeGenericFileBuffer(m_connections[i].pConnection);
            m_connections[i].pConnection = NULL;
        }
        m_connections[i].pConnection = NULL;
        m_connections[i].guidSP.Data1 = 0;
        m_connections[i].guidSP.Data2 = 0;
        m_connections[i].guidSP.Data3 = 0;
        memset(m_connections[i].guidSP.Data4, 0, sizeof(m_connections[i].guidSP.Data4));
    }
    m_connectionCount = 0;
    m_maxConnections = 10;
}

// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004aa930
void CGame::AddConnection(char *name, void *pConnection, unsigned int size, GUID *pGuidSP)
{
    void *pCopy;

    if (m_connectionCount < 10) {
        strcpy(m_connections[m_connectionCount].name, name);
        m_connections[m_connectionCount].guidSP = *pGuidSP;
        pCopy = CFileBuffer::AllocateLockedBuffer(size);
        m_connections[m_connectionCount].pConnection = pCopy;
        if (pCopy != NULL) {
            memcpy(pCopy, pConnection, size);
            m_connectionCount++;
        }
    }
    qsort(m_connections, m_connectionCount, sizeof(DPlayConnection), CompareConnections);
}

// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)


// FUNCTION: CMR2 0x004aad00
DPlayConnection *CGame::GetConnection(BYTE index)
{
    if (index < m_connectionCount)
        return &m_connections[index];
    return NULL;
}

// FUNCTION: CMR2 0x004aad30
bool CGame::FUN_004aad30(BYTE param1, int param2, int param3)
{
    return false;
}

struct Menu;
void FUN_0041f4c0(void);
void FUN_0041f4d0(void);

// Item callbacks of the menu built by 0x475f00.
// FUNCTION: CMR2 0x0049c070
void FUN_0049c070(Menu *pMenu, int param)
{
    FUN_0041f4c0();
}

// FUNCTION: CMR2 0x0049c080
void FUN_0049c080(Menu *pMenu, int param)
{
    FUN_0041f4c0();
    FUN_0041f4d0();
}

int FUN_004055e0(void);
int FUN_004055f0(void);

// Draws the in-race menu built by 0x475f00: the title, then one row per item
// with its banner sprite, the item text and the separator line above the list
// and under every row. The selected row and the lines around it are drawn in
// the bright colour, the rest in the dim one.
// FUNCTION: CMR2 0x0049bcb0
void FUN_0049bcb0(Menu *pMenu)
{
    BYTE colourWhite[4];
    BYTE colourText[4];
    BYTE colourDim[4];
    short line[4];
    short rect[4];
    MenuItem *pItem;
    int cursor;
    int x;
    int i;

    colourText[3] = 0xff;
    colourDim[3] = 0xff;
    colourWhite[0] = 0xff;
    colourWhite[1] = 0xff;
    colourWhite[2] = 0xff;
    colourWhite[3] = 0xff;
    colourText[0] = 0x4f;
    colourText[1] = 0x4f;
    colourText[2] = 0x4f;
    colourDim[0] = 0x4f;
    colourDim[1] = 0x4f;
    colourDim[2] = 0x4f;

    cursor = pMenu->cursor;
    Font_DrawText(0, CFrontend::GetTextString(0xf3), (int)(g_pGraphics->resX * 0xf0) / 0x280,
                  (int)(g_pGraphics->resY * 0xc8) / 0x1e0, (int *)colourText, 0x11);
    x = (int)(g_pGraphics->resX * 0xf0) / 0x280;
    rect[1] = 0;
    rect[0] = (short)x;
    if (FUN_004055e0() != 0) {
        rect[2] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->w;
        rect[3] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->h;
    }
    line[0] = (short)x;
    line[1] = (short)((int)(g_pGraphics->resY * 0xd7) / 0x1e0);
    line[3] = 1;
    line[2] = (short)((int)(g_pGraphics->resX * 0xa2) / 0x280);
    if (cursor == 0)
        Sprite_FillRect((int)g_pGraphics + 0x150, line, colourWhite, 1);
    else
        Sprite_FillRect((int)g_pGraphics + 0x150, line, colourDim, 1);
    pItem = pMenu->items;
    for (i = 0; i < 2; i++, pItem++) {
        if (FUN_004055e0() != 0)
            rect[1] = (short)(line[1] + (int)(g_pGraphics->resY * 0xe) / 0x1e0 -
                              ((SpriteRect *)(FUN_004055e0() + 0x11c))->h / 2);
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                CFrontend::GetTextString(pItem->id));
        if (i == cursor) {
            Font_DrawText(0, CFrontend::m_stringDest,
                          (int)(g_pGraphics->resX * 0xf0) / 0x280 +
                              (int)(g_pGraphics->resX * 0x14) / 0x280,
                          line[1] + (int)(g_pGraphics->resY * 0x12) / 0x1e0, (int *)colourWhite, 0x11);
            if (FUN_004055f0() != 0)
                Sprite_Queue((SpriteRect *)(FUN_004055f0() + 0x11c), (SpriteRect *)rect,
                             (Texture *)FUN_004055f0(), 1, 0, NULL, NULL, colourWhite, 8);
        } else {
            Font_DrawText(0, CFrontend::m_stringDest,
                          (int)(g_pGraphics->resX * 0xf0) / 0x280 +
                              (int)(g_pGraphics->resX * 0x14) / 0x280,
                          line[1] + (int)(g_pGraphics->resY * 0x12) / 0x1e0, (int *)colourText, 0x11);
            if (FUN_004055f0() != 0)
                Sprite_Queue((SpriteRect *)(FUN_004055f0() + 0x11c), (SpriteRect *)rect,
                             (Texture *)FUN_004055f0(), 1, 0, NULL, NULL, colourText, 8);
        }
        line[1] = (short)((int)(g_pGraphics->resY * 0x18) / 0x1e0 * (i + 1) +
                          (int)(g_pGraphics->resY * 0xd7) / 0x1e0);
        if (i == cursor || i + 1 == cursor)
            Sprite_FillRect((int)g_pGraphics + 0x150, line, colourWhite, 1);
        else
            Sprite_FillRect((int)g_pGraphics + 0x150, line, colourDim, 1);
    }
}

// FUNCTION: CMR2 0x0049c090
int CGame::GetCallbackCount(void)
{
    return m_unk0x00593ba0;
}

// FUNCTION: CMR2 0x0049c0f0
void CGame::UnwindCallbacks(int count)
{
    while (m_unk0x00593ba0-- > count)
        ((void (*)(void))m_callbacks[m_unk0x00593ba0])();
    m_unk0x00593ba0 = count;
}

// FUNCTION: CMR2 0x0049c140
void CGame::FUN_0049c140(void)
{
    m_unk0x00593cac = 1;
}

// FUNCTION: CMR2 0x0049c400
int CGame::FUN_0049c400(void)
{
    return m_unk0x0059ce18;
}

// FUNCTION: CMR2 0x0049c410
int CGame::FUN_0049c410(void)
{
    return m_unk0x0059ce20;
}

// FUNCTION: CMR2 0x0049c420
void CGame::FUN_0049c420(int param1)
{
    m_unk0x0059ce14 = param1;
}

// FUNCTION: CMR2 0x0049c430
int CGame::FUN_0049c430(void)
{
    return m_unk0x0059ce14;
}

// Sets field 0x2c of the mesh triangles whose flags match the group mask
// (bits 0..6 against flags 0..6, bits 7..13 against flags 9..15).
// FUNCTION: CMR2 0x0049c440
void FUN_0049c440(Mesh *pMesh, int mask, int value)
{
    MeshTriangle *pTri;
    int low = mask & 0x7f;
    int high = (mask >> 7) & 0x7f;
    int i;

    if (pMesh != NULL) {
        for (i = 0; i < pMesh->triangleCount; i++) {
            if ((pMesh->pTriangles[i].flags & low & 0x7f) != 0 ||
                (high & (pMesh->pTriangles[i].flags >> 9)) != 0)
                pMesh->pTriangles[i].field_0x2c = value;
        }
    }
}

// Same as FUN_0049c440 for field 0x30 (the mesh must exist).
// FUNCTION: CMR2 0x0049c4b0
void FUN_0049c4b0(Mesh *pMesh, int mask, int value)
{
    int low = mask & 0x7f;
    int high = (mask >> 7) & 0x7f;
    int i;
    MeshTriangle *pTri;

    for (i = 0; i < pMesh->triangleCount; i++) {
        pTri = &pMesh->pTriangles[i];
        if ((pTri->flags & low & 0x7f) != 0 || (high & (pTri->flags >> 9)) != 0)
            pTri->field_0x30 = value;
    }
}

extern unsigned short g_unk0x0059be74[];

// Draws the triangles of a mesh in runs that share a texture, using the mesh
// slot already reserved in the shared vertex buffer, and clamps texture
// addressing for the material groups that need it.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049c510
void FUN_0049c510(Mesh *pMesh)
{
    int i;
    int texture;
    int prev = -1;
    int count = 0;
    int total = pMesh->triangleCount;
    MeshTriangle *pTri = pMesh->pTriangles;

    for (i = total; i > 0; i--) {
        texture = *(int *)((BYTE *)pTri + 4 + pTri->field_0x2c * 4);
        if (prev != texture) {
            if (count > 0) {
                CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
                    D3DPT_TRIANGLELIST,
                    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
                    pMesh->vertexOffset, pMesh->field_0x10, g_unk0x0059be74, count, 0);
            }
            count = 0;
            CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[texture]);
            prev = texture;
        }
        g_unk0x0059be74[count++] = pTri->vertexIndex[0];
        g_unk0x0059be74[count++] = pTri->vertexIndex[1];
        g_unk0x0059be74[count++] = pTri->vertexIndex[2];
        CGame::m_unk0x0059ce18++;
        if ((pTri->flags & 0x7f) == 0x70 || (pTri->flags & 0x7f) == 0x71 ||
            (pTri->flags & 0x7f) == 0x73 || (pTri->flags & 0x7f) == 0x74) {
            CGraphics::SetTextureAddressClamp(0);
        } else {
            CGraphics::SetTextureAddressClamp(1);
        }
        pTri++;
    }
    if (count != 0) {
        CGraphics::FUN_004a4850(0, (int)CGraphics::m_pTextureManager->textureBuffer[
            *(int *)((BYTE *)&pMesh->pTriangles[total - 1] + 4 +
                     pMesh->pTriangles[total - 1].field_0x2c * 4)]);
        CGraphics::m_pTextureManager->pD3D->DrawIndexedPrimitiveVB(
            D3DPT_TRIANGLELIST,
            CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex],
            pMesh->vertexOffset, pMesh->field_0x10, g_unk0x0059be74, count, 0);
    }
}

void Graphics_DrawMeshLOD(Mesh *pMesh, int useParts, int clampTexture, int markTextures);

// Draws every mesh node of a node list whose view mask contains the given bit,
// recursing into the children of the nodes that match.
// FUNCTION: CMR2 0x0049ca50
void Game_DrawViewMaskNodes(SceneNode *pNode, int bit)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        if (pNode->type == SCENE_NODE_MESH) {
            Mesh *pMesh = (Mesh *)pNode->pObject;
            int mask = 1 << bit;
            if ((mask & pNode->field_0x17c) != 0) {
                if (pMesh != NULL) {
                    CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)pNode->worldF);
                    Graphics_DrawMeshLOD(pMesh, 0, 0, 0);
                }
                if ((pNode->field_0x17c & mask) != 0 && pNode->pFirstChild != NULL)
                    Game_DrawViewMaskNodes(pNode->pFirstChild, bit);
            }
        }
    }
}

// Draws a single node: if the node is a mesh whose view mask contains the
// given bit it binds the node world matrix and draws its mesh, and then
// recurses into the children that match the view mask as well.
// FUNCTION: CMR2 0x0049cad0
void Game_DrawViewMaskNode(SceneNode *pNode, int bit)
{
    if (pNode->type == SCENE_NODE_MESH) {
        Mesh *pMesh = (Mesh *)pNode->pObject;
        if ((pNode->field_0x17c & (1 << bit)) != 0) {
            if (pMesh != NULL) {
                CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                    (D3DMATRIX *)pNode->worldF);
                Graphics_DrawMeshLOD(pMesh, 0, 0, 0);
            }
        }
    }
    if ((pNode->field_0x17c & (1 << bit)) != 0 && pNode->pFirstChild != NULL)
        Game_DrawViewMaskNodes(pNode->pFirstChild, bit);
}

// FUNCTION: CMR2 0x0049cb50
void CGame::FUN_0049cb50(void *param1)
{
    m_unk0x00593cb0[m_unk0x0059ce28] = param1;
    m_unk0x0059ce28++;
}

// FUNCTION: CMR2 0x0049cb70
void CGame::FUN_0049cb70(void *param1)
{
    m_unk0x00597d04[m_unk0x0059ce2c] = param1;
    m_unk0x0059ce2c++;
}

// qsort comparator of the deferred draw list (0x49cc50): sorts by the
// depth at +0x114 of each entry's node, farthest first.
// FUNCTION: CMR2 0x0049cb90
int __cdecl FUN_0049cb90(const void *a, const void *b)
{
    int depthA = *(int *)(*(BYTE **)(*(BYTE **)a + 0xc) + 0x114);
    int depthB = *(int *)(*(BYTE **)(*(BYTE **)b + 0xc) + 0x114);
    return depthA < depthB ? 1 : -1;
}

// Drawing view of a sector's stage objects (StageObject in Sector.h). It lives
// here instead of in the shared header because touching Sector.h perturbs the
// codegen of unrelated translation units.
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
    int lightLevel;             // 0x9c light level when not lit per vertex
};
typedef char StageObjectDraw_size[sizeof(StageObjectDraw) == 0xa0 ? 1 : -1];

// Non-zero while the draw lists are depth sorted before being drawn
// (defined in Graphics.cpp).
extern int g_unk0x005207b4;

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Draws the objects the deferred pass of the static stage objects queued:
// sorts them by view depth (farthest first) and, for each one, binds its
// texture, sets its world matrix and marks the mesh for the culling test of
// the next frame before drawing it.
// FUNCTION: CMR2 0x0049cc50
void Game_DrawDeferredObjects(void)
{
    unsigned int i;

    if ((unsigned int)CGame::m_unk0x0059ce28 >= 1) {
        if (g_unk0x005207b4 != 0)
            qsort(CGame::m_unk0x00593cb0, CGame::m_unk0x0059ce28, 4, FUN_0049cb90);
        for (i = 0; i < (unsigned int)CGame::m_unk0x0059ce28; i++) {
            FUN_004a3e20((Unk0x004a3e20 *)CGraphics::m_pTextureManager->textureBuffer[
                ((int *)((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->pMesh->pTriangles)[1]], 0);
            CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                (D3DMATRIX *)((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->matrix);
            if (*(int *)((BYTE *)((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->pMesh + 0x114) < 0xc80000)
                ((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->pMesh->flags |= 8;
            else
                ((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->pMesh->flags &= 0xfffffff7;
            Graphics_DrawMeshLOD(((StageObjectDraw *)CGame::m_unk0x00593cb0[i])->pMesh, 1, 0, 0);
        }
        CGame::m_unk0x0059ce28 = 0;
    }
}

// qsort comparator of the transparent draw list (0x49cd20): type 0x14 goes
// last, type 5 sorts after type 0 at equal depth, else farthest first.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049cbc0
int __cdecl FUN_0049cbc0(const void *a, const void *b)
{
    BYTE *pA = *(BYTE **)a;
    BYTE *pB = *(BYTE **)b;
    unsigned int typeA;
    unsigned int typeB;
    int depthA;
    int depthB;
    int diff;

    typeA = *(unsigned int *)(pA + 0x30) & 0xff;
    if (typeA == 0x14)
        return 1;
    typeB = *(unsigned int *)(pB + 0x30) & 0xff;
    if (typeB == 0x14)
        return 1;
    depthA = *(int *)(pA + 0x16c);
    depthB = *(int *)(pB + 0x16c);
    diff = depthA - depthB;
    if (diff < 0)
        diff = depthB - depthA;
    if (diff < 0x10000) {
        if (typeA == 5 && typeB == 0)
            return 1;
        if (typeA == 0 && typeB == 5)
            return -1;
    }
    return depthA < depthB ? 1 : -1;
}

extern unsigned short g_unk0x006ed5f0[];
// GLOBAL: CMR2 0x0059be6c
SceneNode *g_unk0x0059be6c;
// GLOBAL: CMR2 0x00597cc0
D3DMATRIX g_unk0x00597cc0;

void Graphics_DrawMeshLOD(Mesh *pMesh, int useParts, int clampTexture, int markTextures);

// Draws the static stage objects of the culled sectors. Objects whose mesh is
// flagged for the deferred pass (mesh flag 2) get their world matrix and view
// depth updated and are queued; the rest are drawn right away.
// FUNCTION: CMR2 0x0049d040
void FUN_0049d040(void)
{
    StageObjectDraw *pObject;
    unsigned int i;
    FixVector delta;

    for (i = 0; i < (unsigned int)g_sectorCullEnabled; i++) {
        pObject = (StageObjectDraw *)g_sectors[g_unk0x006ed5f0[i]]->pObjects;
        while (pObject != NULL) {
            if (pObject->field_0x14 != 0) {
                if ((pObject->pMesh->flags & 2) != 0) {
                    *(D3DMATRIX *)pObject->matrix = g_unk0x00597cc0;
                    if (pObject->scaleX != 0x10000 || pObject->scaleY != 0x10000 ||
                        pObject->scaleZ != 0x10000) {
                        float scale;
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
                }
                if ((pObject->pMesh->flags & 2) != 0) {
                    delta.x = g_unk0x0059be6c->world.position.x - pObject->position.x;
                    delta.y = 0;
                    delta.z = g_unk0x0059be6c->world.position.z - pObject->position.z;
                    *(int *)((BYTE *)pObject->pMesh + 0x114) = FixVecLength(&delta);
                    CGame::FUN_0049cb50(pObject);
                } else {
                    Graphics_DrawMeshLOD(pObject->pMesh, 1, 0, 0);
                }
            }
            pObject = pObject->pNext;
        }
    }
}

// Identity world transform: the deferred draw paths set it before drawing
// geometry whose vertices are already in world space.
// GLOBAL: CMR2 0x005207b8
D3DMATRIX g_unk0x005207b8 = {
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

void Graphics_DrawMeshLOD(Mesh *pMesh, int useParts, int clampTexture, int markTextures);
void Quad2D_DrawLayer(unsigned int layer);
void Pulse_Update(unsigned int dt);
extern Mesh **g_sceneShadowMeshes;

// Draws the queue of scene nodes collected by 0x0049d290: depth sorts it
// (farthest first) when depth sorting is enabled, draws every node of the
// queue with the given view mask bit and empties it.
// FUNCTION: CMR2 0x0049cd20
void Game_DrawSortedNodes(int bit)
{
    unsigned int i;

    if ((unsigned int)CGame::m_unk0x0059ce2c >= 1) {
        if (g_unk0x005207b4 != 0)
            qsort(CGame::m_unk0x00597d04, CGame::m_unk0x0059ce2c, 4, FUN_0049cbc0);
        for (i = 0; i < (unsigned int)CGame::m_unk0x0059ce2c; i++)
            Game_DrawViewMaskNode((SceneNode *)CGame::m_unk0x00597d04[i], bit);
        CGame::m_unk0x0059ce2c = 0;
    }
}

// Draws every mesh node of the scene in world space with Z test and write
// enabled, then the 2D layer 0x10.
// FUNCTION: CMR2 0x0049cd90
void FUN_0049cd90(void)
{
    SceneNode *pNode;
    Mesh *pMesh;
    unsigned int i;

    CGraphics::SetZEnable(1);
    CGraphics::SetZWriteEnable(1);
    for (i = 0; i < (unsigned int)g_sceneNodeCount; i++) {
        pNode = g_sceneNodes[i];
        if (pNode != NULL && pNode->type == SCENE_NODE_MESH) {
            pMesh = (Mesh *)pNode->pObject;
            if (pNode->field_0x17c != 0 && pMesh != NULL) {
                CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)pNode->worldF);
                Graphics_DrawMeshLOD(pMesh, 0, 0, 0);
            }
        }
    }
    Quad2D_DrawLayer(0x10);
}

// Draws the view-mask scene nodes of the culled sectors that were not queued
// for depth sorting (visible == 0), with Z test and Z write disabled, then
// restores them.
// FUNCTION: CMR2 0x0049ce40
void Game_DrawUnsortedNodes(int bit)
{
    SceneNode *pNode;
    unsigned int i;

    CGraphics::SetZEnable(0);
    CGraphics::SetZWriteEnable(0);
    for (i = 0; i < (unsigned int)g_sectorCullEnabled; i++) {
        pNode = g_sectors[g_unk0x006ed5f0[i]]->pFirstNode;
        while (pNode != NULL) {
            if (pNode->visible == 0)
                Game_DrawViewMaskNode(pNode, bit);
            pNode = pNode->pNextInSector;
        }
    }
    CGraphics::SetZEnable(1);
    CGraphics::SetZWriteEnable(1);
}

// Advances the pulse effect by the frame delta and draws the ground mesh of
// every culled sector in world space.
// The two render passes have independent lazy frame stamps.
// GLOBAL: CMR2 0x00597d00
BYTE g_sectorFrameInit;
// GLOBAL: CMR2 0x00597d01
BYTE g_shadowFrameInit;
// GLOBAL: CMR2 0x0059bd68
unsigned int g_sectorFrameStart;
// GLOBAL: CMR2 0x00597cb0
unsigned int g_sectorFramePrevious;
// GLOBAL: CMR2 0x00597cb4
unsigned int g_shadowFrameStart;
// GLOBAL: CMR2 0x0059be70
unsigned int g_shadowFramePrevious;

// FUNCTION: CMR2 0x0049cec0
void FUN_0049cec0(void)
{
    Mesh *pMesh;
    unsigned int i;

    if ((g_sectorFrameInit & 1) == 0) {
        g_sectorFrameInit |= 1;
        g_sectorFrameStart = CMain::GetFrameDelta();
    }
    if ((g_sectorFrameInit & 2) == 0) {
        g_sectorFrameInit |= 2;
        g_sectorFramePrevious = CMain::GetFrameDelta();
    }
    Pulse_Update(CMain::GetFrameDelta() - g_sectorFramePrevious);
    g_sectorFramePrevious = CMain::GetFrameDelta();
    CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_unk0x005207b8);
    for (i = 0; i < (unsigned int)g_sectorCullEnabled; i++) {
        pMesh = (Mesh *)g_sectors[g_unk0x006ed5f0[i]]->pMesh;
        if (pMesh != NULL) {
            if ((pMesh->flags & 0x1000) != 0)
                Graphics_DrawMeshLOD(pMesh, 0, 1, 0);
            else
                Graphics_DrawMeshLOD(pMesh, 1, 0, 0);
        }
    }
}

// Advances the pulse effect by the frame delta and draws the shadow mesh of
// every culled sector in world space, with Z writes disabled.
// FUNCTION: CMR2 0x0049cf80
void FUN_0049cf80(void)
{
    Mesh *pMesh;
    unsigned int i;

    if ((g_shadowFrameInit & 1) == 0) {
        g_shadowFrameInit |= 1;
        g_shadowFrameStart = CMain::GetFrameDelta();
    }
    if ((g_shadowFrameInit & 2) == 0) {
        g_shadowFrameInit |= 2;
        g_shadowFramePrevious = CMain::GetFrameDelta();
    }
    Pulse_Update(CMain::GetFrameDelta() - g_shadowFramePrevious);
    g_shadowFramePrevious = CMain::GetFrameDelta();
    CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_unk0x005207b8);
    if (g_sceneShadowMeshes != NULL) {
        CGraphics::SetZWriteEnable(0);
        for (i = 0; i < (unsigned int)g_sectorCullEnabled; i++) {
            pMesh = g_sceneShadowMeshes[g_unk0x006ed5f0[i]];
            if (pMesh != NULL)
                Graphics_DrawMeshLOD(pMesh, 1, 0, 1);
        }
        CGraphics::SetZWriteEnable(1);
    }
}

// Computes the view depth of every visible node of the culled sectors and
// queues them for drawing.
// FUNCTION: CMR2 0x0049d290
void FUN_0049d290(int param1)
{
    SceneNode *pNode;
    FixVector delta;
    unsigned short *pIndex;
    unsigned int i;

    i = 0;
    if ((unsigned int)g_sectorCullEnabled > 0) {
        pIndex = (unsigned short *)g_unk0x006ed5f0;
        do {
            pNode = g_sectors[*pIndex]->pFirstNode;
            while (pNode != NULL) {
                if (pNode->visible != 0) {
                    delta.x = g_unk0x0059be6c->world.position.x - pNode->world.position.x;
                    delta.y = 0;
                    delta.z = g_unk0x0059be6c->world.position.z - pNode->world.position.z;
                    *(int *)((BYTE *)pNode + 0x16c) = FixVecLength(&delta);
                    CGame::FUN_0049cb70(pNode);
                }
                pNode = pNode->pNextInSector;
            }
            i++;
            pIndex++;
        } while (i < (unsigned int)g_sectorCullEnabled);
    }
}

// Draws one frame of the race view: sets the viewport and the render states of
// the 3D pass and of the 2D layers, draws the world (relit sectors, static and
// deferred objects, ground and shadow meshes, view masked nodes) with the given
// view bit, then restores the full screen viewport and flushes the remaining 2D
// layers. Returns 1 (also when the device refuses to start a scene).
void Graphics_SetLightingMode(int mode);
void Graphics_EnableFog(void);
void Graphics_DisableFog(void);
void Graphics_SetRenderTarget(Texture *pTexture);
void ScreenLine2D_Draw(int layer);
void Tri2D_DrawLayer(int layer);
void Line2D_Draw(void);
void Quad2D_DrawLayer(unsigned int layer);
void Billboard_Draw(SceneNode *pCamera);
void Glow_Draw(SceneNode *pCamera, BYTE view);
void Particle_DrawAll(int param, BYTE view);
void Scene_DrawShadowBatches(unsigned int view);
void Scene_RelightSector(int sector);
BYTE Flare_SampleVisibility(short *pRect, BYTE *pColour, BYTE tolerance);
void FUN_004b7de0(SceneNode *pNode, int unused);
void FUN_004b21e0(void);
int FUN_004bcae0(void);
void FUN_0049dcc0(int enable);
void FUN_0049d040(void);
void FUN_0049cd90(void);
void FUN_0049cec0(void);
void FUN_0049cf80(void);
void Game_DrawUnsortedNodes(int bit);
void Game_DrawSortedNodes(int bit);
void FUN_0049d290(int param1);
void Game_DrawDeferredObjects(void);
extern FixMatrix g_unk0x0059bd28;
D3DMATRIX *FixMatrix_ToFloat(D3DMATRIX *pOut, FixMatrix *pIn);

// match 94%: the residual diff is register allocation (the relight loop walks
// the sector list from a register where the original keeps the pointer in the
// argument slot, and the inlined fixed point normalisation uses different
// scratch slots), plus the 0.0f store which the original materialises as an
// immediate.
// FUNCTION: CMR2 0x0049d3f0
int FUN_0049d3f0(int param1, int param2, void *param3, int bit, BYTE flag)
{
    D3DVIEWPORT7 viewport;
    FixVector cameraPosition;
    FixVector forward;
    int farPlane;
    unsigned int i;
    unsigned short *pIndex;

    g_unk0x0059be6c = (SceneNode *)param2;
    CGame::m_unk0x0059ce18 = 0;
    CGame::m_unk0x0059ce1c = 0;
    CGame::m_unk0x0059ce20 = 0;
    if (CGraphics::m_pTextureManager->pD3D->BeginScene() != D3D_OK)
        return 1;
    memset(&viewport, 0, sizeof(viewport));
    viewport.dvMinZ = 0.0f;
    viewport.dvMaxZ = 1.0f;
    viewport.dwX = ((short *)param3)[0];
    viewport.dwY = ((short *)param3)[1];
    viewport.dwWidth = ((short *)param3)[2];
    viewport.dwHeight = ((short *)param3)[3];
    CGraphics::m_pTextureManager->pD3D->SetViewport(&viewport);
    FUN_0049dcc0(1);
    CGraphics::SetCullMode(3);
    Graphics_SetLightingMode(5);
    ScreenLine2D_Draw(4);
    Tri2D_DrawLayer(4);
    Sprite_DrawLayer(4);
    if (flag != 0) {
        if (CGraphics::m_unk0x0072d56c != 0) {
            CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                                             &g_unk0x005207b8);
            FUN_004b7de0((SceneNode *)param2, (int)param3);
            pIndex = (unsigned short *)g_unk0x006ed5f0;
            for (i = 0; i < (unsigned int)g_sectorCullEnabled; i++, pIndex++)
                Scene_RelightSector(*pIndex);
        }
        CGraphics::SetCullMode(CGame::FUN_0049dcb0());
        FixMatrix_GetPosition(&cameraPosition, &((SceneNode *)param2)->world);
        Particle_DrawAll(param2, (BYTE)bit);
        forward.x = g_unk0x0059be6c->world.forward.x;
        forward.y = 0;
        forward.z = g_unk0x0059be6c->world.forward.z;
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
        if ((unsigned int)g_sectorCount > 0) {
            CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1c,
                                                               FUN_004bcae0());
            Graphics_DisableFog();
            Graphics_SetLightingMode(5);
            farPlane = CGraphics::m_farPlaneFixed;
            CGraphics::SetProjection(0, 0, 0x960000, 0);
            Game_DrawUnsortedNodes(bit);
            CGraphics::SetProjection(0, 0, farPlane, 0);
            Graphics_EnableFog();
            Graphics_SetLightingMode(0);
            FUN_0049cec0();
            Graphics_SetLightingMode(5);
            FUN_0049cf80();
            FUN_0049dcc0(1);
            Glow_Draw((SceneNode *)param2, (BYTE)bit);
            Quad2D_DrawLayer(8);
            Graphics_SetLightingMode(4);
            Quad2D_DrawLayer(0x20);
            Graphics_SetLightingMode(1);
            if (CGame::m_unk0x0059ce14 != 0) {
                FUN_0049d040();
                Game_DrawDeferredObjects();
                CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD,
                                                                 &g_unk0x005207b8);
                FUN_0049dcc0(1);
                CGraphics::FUN_004a3e40(5, 6);
                CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1c, 0);
                Graphics_SetLightingMode(5);
                Quad2D_DrawLayer(0x10);
                Line2D_Draw();
                Billboard_Draw((SceneNode *)param2);
                Graphics_SetLightingMode(1);
                FUN_0049d290(bit);
                Game_DrawSortedNodes(bit);
            } else {
                FUN_0049d040();
                FUN_0049d290(bit);
                Game_DrawSortedNodes(bit);
                Game_DrawDeferredObjects();
            }
        } else {
            FUN_0049cd90();
        }
        CGraphics::m_pTextureManager->pD3D->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_unk0x005207b8);
        FUN_0049dcc0(1);
        CGraphics::FUN_004a3e40(5, 6);
        CGraphics::m_pTextureManager->pD3D->SetRenderState((D3DRENDERSTATETYPE)0x1c, 0);
        Graphics_SetLightingMode(5);
        Quad2D_DrawLayer(0x10);
        Graphics_SetLightingMode(4);
        Quad2D_DrawLayer(0x40);
        Graphics_SetLightingMode(5);
        Line2D_Draw();
        Billboard_Draw((SceneNode *)param2);
        Scene_DrawShadowBatches(bit);
    }
    Flare_SampleVisibility(NULL, NULL, 0);
    CGraphics::SetZWriteEnable(0);
    CGraphics::SetZEnable(0);
    CGraphics::FUN_004a3e40(5, 6);
    memset(&viewport, 0, sizeof(viewport));
    viewport.dwX = 0;
    viewport.dwY = 0;
    viewport.dwWidth = g_pGraphics->resX;
    viewport.dwHeight = g_pGraphics->resY;
    viewport.dvMinZ = 0.0f;
    viewport.dvMaxZ = 1.0f;
    CGraphics::m_pTextureManager->pD3D->SetViewport(&viewport);
    CGraphics::SetCullMode(3);
    ScreenLine2D_Draw(3);
    Tri2D_DrawLayer(3);
    Sprite_DrawLayer(3);
    ScreenLine2D_Draw(2);
    Tri2D_DrawLayer(2);
    Sprite_DrawLayer(2);
    ScreenLine2D_Draw(1);
    Tri2D_DrawLayer(1);
    Sprite_DrawLayer(1);
    CGraphics::SetCullMode(CGame::FUN_0049dcb0());
    FUN_004b21e0();
    CGraphics::m_pTextureManager->pD3D->EndScene();
    return 1;
}

// FUNCTION: CMR2 0x0049dca0
void CGame::FUN_0049dca0(int param1)
{
    m_unk0x005207f8 = param1;
}

// FUNCTION: CMR2 0x0049dcb0
int CGame::FUN_0049dcb0(void)
{
    return m_unk0x005207f8;
}

// GLOBAL: CMR2 0x00537f5c
int g_unk0x00537f5c;
// GLOBAL: CMR2 0x00665324
int g_unk0x00665324;

// FUNCTION: CMR2 0x0041f260
void CGame::FUN_0041f260(void)
{
    g_unk0x00537f5c = GetCallbackCount();
}

// FUNCTION: CMR2 0x004aae10
int FUN_004aae10(void)
{
    g_unk0x00665324 = 0;
    return 1;
}

// FUNCTION: CMR2 0x004aad50
void CGame::FUN_004aad50(void)
{
    RegisterCallback(FUN_004aae10, NULL);
    g_unk0x00665324 = 1;
}


// FUNCTION: CMR2 0x004764c0
void FUN_004764c0(void *param1)
{
    if (g_unk0x0058d49c[*((BYTE *)param1 + 2)] != NULL)
        CGame::FUN_0049c420(0);
}

// FUNCTION: CMR2 0x00476500
void FUN_00476500(void *param1)
{
    if (g_unk0x0058d49c[*((BYTE *)param1 + 2)] != NULL)
        CGame::FUN_0049c420(1);
}

void FUN_00486b20(BYTE *pCar, BYTE *pInfo);
void FUN_0048d7b0(BYTE *pCar, BYTE *pInfo);

// Dispatches by the object type stored at +4.
// FUNCTION: CMR2 0x00423810
void FUN_00423810(BYTE *pObject, BYTE *pInfo)
{
    switch (*(int *)(pObject + 4)) {
    case 1:
    case 2:
    case 10:
        FUN_00486b20(pObject, pInfo);
        return;
    case 7:
        FUN_0048d7b0(pObject, pInfo);
    }
}

struct Unk004238e0 {
    int field_0x0;
    int field_0x4;
};

void FUN_00486c00(BYTE *p, BYTE *q);
void FUN_00486be0(BYTE *p, int unused);
void FUN_00476500(void *param1);
void FUN_0048d850(BYTE *pCar, BYTE *pInfo);

// Dispatches by the object type stored at +4.
// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423900
void FUN_00423900(BYTE *pObject, BYTE *pInfo)
{
    switch (*(int *)(pObject + 4)) {
    case 3:
        FUN_00476500(pObject);
        return;
    case 2:
        FUN_00486be0(pObject, (int)pInfo);
        return;
    case 1:
    case 10:
        FUN_00486c00(pObject, pInfo);
        return;
    case 7:
        FUN_0048d850(pObject, pInfo);
    }
}

void FUN_00486b90(BYTE *pCar, BYTE *pInfo);
void FUN_004764e0(BYTE *p);
FixMatrix *FUN_00423d70(BYTE index);
void FUN_00447be0(BYTE *pDst, BYTE *pSrc, FixMatrix *pM);
void FUN_0048d800(BYTE *pInfo, BYTE *pCar);

// Dispatches by the object type stored at +4.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423860
void FUN_00423860(BYTE *pObject, BYTE *pInfo)
{
    switch (*(int *)(pObject + 4)) {
    case 1:
    case 2:
    case 10:
        FUN_00486b90(pObject, pInfo);
        return;
    case 3:
        FUN_004764e0(pObject);
        return;
    case 4:
    case 5:
        FUN_00447be0(pObject, pInfo, FUN_00423d70(pObject[2]));
        return;
    case 7:
        FUN_0048d800(pObject, pInfo);
    }
}

// FUNCTION: CMR2 0x004238e0
void FUN_004238e0(Unk004238e0 *param1, int param2)
{
    if (param1->field_0x4 == 3)
        FUN_004764c0(param1);
}

typedef HRESULT (__stdcall *DPMethod0)(void *pThis);
typedef HRESULT (__stdcall *DPMethod2)(void *pThis, void *p1, DWORD p2);
typedef HRESULT (__stdcall *DPSendFn)(void *pThis, DPID from, DPID to, DWORD flags, void *data, DWORD size);
typedef HRESULT (__stdcall *DPMethod4)(void *pThis, DWORD a1, DWORD a2, DWORD a3, DWORD a4);

// GLOBAL: CMR2 0x005a0068
BYTE g_unk0x005a0068[0x50];

// The current session description and the list of enumerated sessions
#define SESSION (*(DPSESSIONDESC2 *)g_unk0x005a0068)
#define SESSIONS ((DPSESSIONDESC2 *)CGameInfo::m_unk0x0059fa20)

// Reads the description of the joined session into SESSION (the session
// name is copied to m_unk0x005a00b8).
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a0d60
BOOL FUN_004a0d60(void)
{
    IDirectPlay4A *pDP;
    DPSESSIONDESC2 *pDesc;
    DWORD size;

    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return FALSE;
    if (((DPMethod2)(*(void ***)pDP)[0x58 / 4])(pDP, NULL, (DWORD)&size) != DPERR_BUFFERTOOSMALL)
        return FALSE;
    pDesc = (DPSESSIONDESC2 *)CFileBuffer::AllocateLockedBuffer(size);
    if (pDesc == NULL)
        return FALSE;
    switch (((DPMethod2)(*(void ***)pDP)[0x58 / 4])(pDP, pDesc, (DWORD)&size)) {
    case DPERR_INVALIDOBJECT:
        free(pDesc);
        return FALSE;
    case DPERR_NOCONNECTION:
        CFileBuffer::FreeGenericFileBuffer(pDesc);
        return FALSE;
    case DP_OK:
        SESSION.dwSize = pDesc->dwSize;
        SESSION.dwFlags = pDesc->dwFlags;
        SESSION.guidInstance = pDesc->guidInstance;
        SESSION.guidApplication = pDesc->guidApplication;
        SESSION.dwMaxPlayers = pDesc->dwMaxPlayers;
        SESSION.dwCurrentPlayers = pDesc->dwCurrentPlayers;
        strcpy((char *)&CGameInfo::m_unk0x005a00b8, pDesc->lpszSessionNameA);
        SESSION.dwReserved1 = pDesc->dwReserved1;
        SESSION.dwReserved2 = pDesc->dwReserved2;
        SESSION.dwUser1 = pDesc->dwUser1;
        SESSION.dwUser2 = pDesc->dwUser2;
        SESSION.dwUser3 = pDesc->dwUser3;
        SESSION.dwUser4 = pDesc->dwUser4;
        CFileBuffer::FreeGenericFileBuffer(pDesc);
        return TRUE;
    }
    CFileBuffer::FreeGenericFileBuffer(pDesc);
    return FALSE;
}

// The GUID of the CMR2 DirectPlay application (shared with GameInfo.cpp).
extern int g_unk0x00511cd8[4];

// Creates (hosts) a DirectPlay session with the given name, password and user
// values. Returns 1 when the session was created.
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a0ec0
int FUN_004a0ec0(char *pSessionName, char *pPassword, DWORD user1, DWORD user2,
                 DWORD user3, DWORD user4, DWORD maxPlayers)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    memset(g_unk0x005a0068, 0, sizeof(g_unk0x005a0068));
    SESSION.dwSize = sizeof(DPSESSIONDESC2);
    SESSION.dwFlags = 0x2064;
    SESSION.guidApplication = *(GUID *)g_unk0x00511cd8;
    SESSION.dwMaxPlayers = maxPlayers;
    strcpy((char *)&CGameInfo::m_unk0x005a00b8, pSessionName);
    SESSION.lpszSessionNameA = (LPSTR)&CGameInfo::m_unk0x005a00b8;
    strcpy((char *)&CGameInfo::m_unk0x005a02c0, pPassword);
    SESSION.lpszPasswordA = (LPSTR)&CGameInfo::m_unk0x005a02c0;
    SESSION.dwUser1 = user1;
    SESSION.dwUser2 = user2;
    SESSION.dwUser3 = user3;
    SESSION.dwUser4 = user4;
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = ((DPMethod2)(*(void ***)pDP)[0x60 / 4])(pDP, g_unk0x005a0068, DPOPEN_CREATE);
    switch (hr) {
    case DPERR_INVALIDPARAM:
    case DPERR_ALREADYINITIALIZED:
    case DPERR_ACCESSDENIED:
    case DPERR_INVALIDFLAGS:
    case DPERR_NOCONNECTION:
    case DPERR_TIMEOUT:
    case DPERR_USERCANCEL:
    case DPERR_UNINITIALIZED:
    case DPERR_NONEWPLAYERS:
    case DPERR_INVALIDPASSWORD:
    case DPERR_CONNECTING:
    case DPERR_AUTHENTICATIONFAILED:
    case DPERR_CANTLOADSSPI:
    case DPERR_ENCRYPTIONFAILED:
    case DPERR_SIGNFAILED:
    case DPERR_CANTLOADSECURITYPACKAGE:
    case DPERR_CANTLOADCAPI:
    case DPERR_LOGONDENIED:
        return 0;
    case DP_OK:
        CGameInfo::m_unk0x005a0060 = 1;
        CGameInfo::m_unk0x005a1814 = 1;
        return 1;
    }
    return 0;
}

// Opens (joins) the enumerated session at index into SESSION using the given
// password and pushes it to DirectPlay. Returns 1 when the join is still in
// progress, and sets *pInvalidPassword when DirectPlay rejects the password.
// FUNCTION: CMR2 0x004a10b0
int FUN_004a10b0(BYTE index, char *pPassword, BYTE *pInvalidPassword)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    *pInvalidPassword = 0;
    if (index < CGameInfo::m_unk0x005a01bc) {
        memset(g_unk0x005a0068, 0, sizeof(g_unk0x005a0068));
        SESSION.dwSize = sizeof(DPSESSIONDESC2);
        SESSION.guidInstance = SESSIONS[index].guidInstance;
        SESSION.lpszSessionNameA = (LPSTR)&CGameInfo::m_unk0x005a00b8;
        SESSION.lpszPasswordA = pPassword;
        pDP = CGame::GetDirectPlay();
        if (pDP != NULL) {
            hr = ((DPMethod2)(*(void ***)pDP)[0x60 / 4])(pDP, g_unk0x005a0068, DPOPEN_JOIN);
            switch (hr) {
            case DPERR_INVALIDPASSWORD:
                *pInvalidPassword = 1;
                return 0;
            case DPERR_ALREADYINITIALIZED:
                return 0;
            case DPERR_ACCESSDENIED:
                return 0;
            case DPERR_INVALIDFLAGS:
                return 0;
            case DPERR_INVALIDPARAM:
                return 0;
            case DPERR_NOCONNECTION:
                return 0;
            case DPERR_TIMEOUT:
                return 0;
            case DPERR_USERCANCEL:
                return 0;
            case DPERR_UNINITIALIZED:
                return 0;
            case DPERR_NONEWPLAYERS:
                return 0;
            case DPERR_CONNECTING:
                return 0;
            case DPERR_AUTHENTICATIONFAILED:
                return 0;
            case DPERR_CANTLOADSSPI:
                return 0;
            case DPERR_ENCRYPTIONFAILED:
                return 0;
            case DPERR_SIGNFAILED:
                return 0;
            case DPERR_CANTLOADSECURITYPACKAGE:
                return 0;
            case DPERR_CANTLOADCAPI:
                return 0;
            case DPERR_LOGONDENIED:
                return 0;
            default:
                return 1;
            case DP_OK:
                CGameInfo::m_unk0x005a1814 = TRUE;
                FUN_004a0d60();
                return 1;
            }
        }
    }
    return 0;
}

// Closes the DirectPlay session object.
// FUNCTION: CMR2 0x004a1280
int FUN_004a1280(void)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = ((DPMethod0)(*(void ***)pDP)[0x10 / 4])(pDP);
    if (hr <= (HRESULT)0x887700dc || hr != 0)
        return 0;
    CGameInfo::m_unk0x005a1814 = hr;
    return 1;
}

// FUNCTION: CMR2 0x004a14e0
bool FUN_004a14e0(void)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return FALSE;
    hr = ((DPMethod2)(*(void ***)pDP)[0x7c / 4])(pDP, g_unk0x005a0068, 0);
    if (hr <= (HRESULT)0x887700dc || hr == (HRESULT)0x88770168 || hr != 0)
        return false;
    return true;
}

// FUNCTION: CMR2 0x004a1480
unsigned int FUN_004a1480(void)
{
    return *(unsigned int *)&CGameInfo::m_unk0x005a01bc & 0xff;
}

// FUNCTION: CMR2 0x004a1490
char *FUN_004a1490(BYTE index)
{
    if (index < CGameInfo::m_unk0x005a01bc)
        return SESSIONS[index].lpszSessionNameA;
    return NULL;
}

// FUNCTION: CMR2 0x004a14c0
LPVOID *FUN_004a14c0(void)
{
    return g_sessionNamePtr;
}

// FUNCTION: CMR2 0x004a14d0
LPVOID *FUN_004a14d0(void)
{
    return g_sessionPasswordPtr;
}

// FUNCTION: CMR2 0x004a15b0
void FUN_004a15b0(BOOL param1)
{
    CGameInfo::m_unk0x005a0060 = param1;
}

// FUNCTION: CMR2 0x004a15c0
int FUN_004a15c0(BYTE index, GUID *pOut)
{
    if (index < CGameInfo::m_unk0x005a01bc) {
        *pOut = SESSIONS[index].guidInstance;
        return 1;
    }
    return 0;
}

// Sets one of the four session user values and pushes the description.
// FUNCTION: CMR2 0x004a16c0
void Session_SetUserValue(BYTE index, int value)
{
    FUN_004a0d60();
    switch (index) {
    case 0:
        *(int *)(g_unk0x005a0068 + 0x40) = value;
        FUN_004a14e0();
        return;
    case 1:
        *(int *)(g_unk0x005a0068 + 0x44) = value;
        FUN_004a14e0();
        return;
    case 2:
        *(int *)(g_unk0x005a0068 + 0x48) = value;
        FUN_004a14e0();
        return;
    }
    *(int *)(g_unk0x005a0068 + 0x4c) = value;
    FUN_004a14e0();
}

// FUNCTION: CMR2 0x004a1720
DWORD FUN_004a1720(int index)
{
    if (index < 0)
        return SESSION.dwCurrentPlayers;
    return SESSIONS[index].dwCurrentPlayers;
}

// FUNCTION: CMR2 0x004a1740
DWORD FUN_004a1740(BYTE index)
{
    return SESSIONS[index].dwMaxPlayers;
}

// FUNCTION: CMR2 0x004a1760
void FUN_004a1760(DPSESSIONDESC2 *pDesc)
{
    SESSION = *pDesc;
    g_sessionNamePtr = (LPVOID *)&CGameInfo::m_unk0x005a00b8;
    g_sessionPasswordPtr = (LPVOID *)&CGameInfo::m_unk0x005a02c0;
}

// FUNCTION: CMR2 0x004a1790
BYTE FUN_004a1790(BYTE index)
{
    BYTE r = (BYTE)(SESSIONS[index].dwFlags >> 10);
    r &= 1;
    return r;
}

// Adds a remote player to the session player table (at most 7 players,
// ignoring the local player and players already listed).
// FUNCTION: CMR2 0x004a1850
void FUN_004a1850(char *shortName, char *longName, DPID dpId)
{
    Unk0x005a1820 *pPlayer;
    int i;

    if (CGame::m_unk0x005a1ea0 == dpId)
        return;
    for (i = 0; i < 7; i++) {
        if (CGame::m_unk0x005a1820[i].field_0xc8 == dpId)
            return;
    }
    if (CGame::m_unk0x005a1818 >= 7)
        return;
    for (i = 0; i < 7; i++) {
        if (CGame::m_unk0x005a1820[i].field_0xcc == 0) {
            if (shortName != NULL)
                strcpy(CGame::m_unk0x005a1820[i].field_0x0, shortName);
            if (longName != NULL)
                strcpy(CGame::m_unk0x005a1820[i].field_0x64, longName);
            CGame::m_unk0x005a1820[i].field_0xc8 = dpId;
            CGame::m_unk0x005a1820[i].field_0xcc = 1;
            CGame::m_unk0x005a1818++;
            return;
        }
    }
}

// IDirectPlay4::EnumPlayers callback of FUN_004a1af0.
// FUNCTION: CMR2 0x004a1ad0
BOOL FAR PASCAL FUN_004a1ad0(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
    FUN_004a1850(lpName->lpszShortNameA, lpName->lpszLongNameA, dpId);
    return 1;
}

// FUNCTION: CMR2 0x004a1af0
int FUN_004a1af0(void)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    CGame::FUN_004a17f0(0);
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = ((DPMethod4)(*(void ***)pDP)[0x30 / 4])(pDP, 0, (DWORD)FUN_004a1ad0, 0, 0);
    if (hr <= (HRESULT)0x887700fa || hr != 0)
        return 0;
    return 1;
}

// EnumConnections callback: keeps every service provider connection.
// FUNCTION: CMR2 0x004aabd0
BOOL __stdcall FUN_004aabd0(LPCGUID lpguidSP, LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName, DWORD dwFlags, LPVOID lpContext)
{
    CGame::AddConnection(lpName->lpszShortNameA, lpConnection, dwConnectionSize, (GUID *)lpguidSP);
    return TRUE;
}

// FUNCTION: CMR2 0x004aac00
bool FUN_004aac00(void)
{
    HRESULT hr;

    CGame::ClearConnections();
    hr = ((DPMethod4)(*(void ***)CGame::m_pDirectPlay4A)[0x8c / 4])(CGame::m_pDirectPlay4A, 0, (DWORD)FUN_004aabd0, 0, 0);
    if (hr == (HRESULT)0x80070057 || hr == (HRESULT)0x88770078)
        return false;
    if (hr == 0)
        return true;
    return false;
}

// Sets the local player data (guaranteed).
// FUNCTION: CMR2 0x004a1cb0
char FUN_004a1cb0(int data, int size)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    pDP = CGame::GetDirectPlay();
    if (pDP != NULL) {
        hr = ((DPMethod4)(*(void ***)pDP)[0x74 / 4])(pDP, CGame::m_unk0x005a1ea0, data, size, 2);
        if (hr > (HRESULT)0x88770082 && hr != (HRESULT)0x88770096 &&
            hr != (HRESULT)0x88770168 && hr == 0)
            return 1;
    }
    return 0;
}

// Sends a message to a player (0 = all), guaranteed when requested.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a1c50
char FUN_004a1c50(int to, int guaranteed, int data, int size)
{
    BOOL flags;
    IDirectPlay4A *pDP;
    HRESULT hr;

    flags = FALSE;
    if (guaranteed == 1)
        flags = TRUE;
    pDP = CGame::GetDirectPlay();
    if (pDP != NULL) {
        hr = ((DPSendFn)(*(void ***)pDP)[0x68 / 4])(pDP, CGame::m_unk0x005a1ea0, to, flags, (void *)data, size);
        if (hr > (HRESULT)0x8877010e && hr != (HRESULT)0x88770816 && hr == 0)
            return 1;
    }
    return 0;
}

typedef HRESULT (__stdcall *DPMethod5)(void *pThis, DWORD a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5);
typedef HRESULT (__stdcall *DPMethod6)(void *pThis, DWORD a1, DWORD a2, DWORD a3, DWORD a4, DWORD a5, DWORD a6);

// GLOBAL: CMR2 0x005a1fa8
DPNAME g_networkPlayerName;
// FUNCTION: CMR2 0x004a1a10
int FUN_004a1a10(int param1, int param2, int param3, int param4)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    memset(&g_networkPlayerName, 0, sizeof(g_networkPlayerName));
    g_networkPlayerName.dwSize = sizeof(g_networkPlayerName);
    g_networkPlayerName.lpszShortNameA = (char *)param1;
    g_networkPlayerName.lpszLongNameA = (char *)param2;
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = pDP->CreatePlayer(&CGame::m_unk0x005a1ea0, &g_networkPlayerName, NULL,
                          (void *)param3, param4, 0);
    if (hr <= (HRESULT)0x88770078 || hr == (HRESULT)0x887700aa || hr != 0)
        return 0;
    CGame::m_unk0x005a1fc0 = 1;
    return 1;
}


// Releases the scene resources held by the 0x58d3xx/0x58d5xx/0x58d6xx blocks
// (the 16 rows at 0x58d3b8 hold file buffers, not scene nodes).
// FUNCTION: CMR2 0x004779e0
BOOL FUN_004779e0(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if (g_unk0x0058d49c[i] != NULL) {
            if (*(int *)(g_unk0x0058d530 + i * 0x1c) != 0)
                SceneNode_Destroy((SceneNode *)*(int *)(g_unk0x0058d530 + i * 0x1c));
            SceneNode_Destroy((SceneNode *)g_unk0x0058d49c[i]);
            *(int *)(g_unk0x0058d530 + i * 0x1c + 0x0) = 0;
            *(int *)(g_unk0x0058d530 + i * 0x1c + 0x4) = 0;
            *(int *)(g_unk0x0058d530 + i * 0x1c + 0x8) = 0;
            *(int *)(g_unk0x0058d530 + i * 0x1c + 0xc) = 0;
            *(int *)(g_unk0x0058d530 + i * 0x1c + 0x10) = 0;
        }
    }
    for (i = 0; i < 2; i++) {
        if (g_unk0x0058d6a0[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x0058d6a0[i]);
            g_unk0x0058d6a0[i] = NULL;
        }
    }
    for (i = 0; i < 2; i++) {
        g_unk0x0058d4c0[i * 2] = 0;
        g_unk0x0058d4c0[i * 2 + 1] = 0;
    }
    for (i = 0; i < 16; i++) {
        if (*(void **)(g_unk0x0058d3b8 + i * 0xc) != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*(void **)(g_unk0x0058d3b8 + i * 0xc));
            *(void **)(g_unk0x0058d3b8 + i * 0xc) = NULL;
        }
        *(int *)(g_unk0x0058d3b8 + i * 0xc + 0x0) = 0;
        *(int *)(g_unk0x0058d3b8 + i * 0xc + 0x4) = 0;
        *(int *)(g_unk0x0058d3b8 + i * 0xc + 0x8) = 0;
    }
    return TRUE;
}

// Adds the player slot to the DirectPlay session.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004aac40
bool FUN_004aac40(BYTE param1)
{
    IDirectPlay4A *pDP;
    HRESULT hr;
    int local1;
    int local2;

    if ((BYTE)param1 >= CGame::m_connectionCount)
        return false;
    local1 = 0;
    local2 = 0;
    if (CGame::FUN_004aad30(param1, (int)&local2, (int)&local1)) {
        pDP = CGame::m_pDirectPlay4A;
        hr = ((DPMethod2)(*(void ***)pDP)[0x98 / 4])(pDP, (void *)(int)local1, 0);
    } else {
        pDP = CGame::m_pDirectPlay4A;
        hr = ((DPMethod2)(*(void ***)pDP)[0x98 / 4])(pDP,
            CGame::m_connections[param1 & 0xff].pConnection, 0);
    }
    if (hr <= (HRESULT)0x88770078) {
        if (hr == (HRESULT)0x88770078 || hr == (HRESULT)0x80070057 ||
            hr != (HRESULT)0x88770005)
            return false;
    } else {
        if (hr == (HRESULT)0x887700fa || hr != 0)
            return false;
    }
    CGame::m_maxConnections = (BYTE)param1;
    return true;
}

// Enumera las sesiones o vuelca el buffer recibido en *param2.
// FUNCTION: CMR2 0x004a1b90
int FUN_004a1b90(int param1, void **param2)
{
    DWORD bufferSize = CGame::m_unk0x005a1fbc;
    DPID receiver;
    IDirectPlay4A *pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    HRESULT hr = pDP->Receive((LPDPID)param1, &receiver, DPRECEIVE_ALL,
                             CGame::m_unk0x005a1fb8, &bufferSize);
    if (hr <= DPERR_INVALIDOBJECT) {
        if (hr == DPERR_INVALIDOBJECT || hr == DPERR_GENERIC || hr == DPERR_INVALIDPARAMS ||
            hr != DPERR_BUFFERTOOSMALL)
            goto done;
        if (CGame::m_unk0x005a1fb8 != NULL) {
            CFileBuffer::FreeGenericFileBuffer(CGame::m_unk0x005a1fb8);
            CGame::m_unk0x005a1fb8 = NULL;
        }
        CGame::m_unk0x005a1fb8 = CFileBuffer::AllocateLockedBuffer(bufferSize);
        if (CGame::m_unk0x005a1fb8 != NULL)
            CGame::m_unk0x005a1fbc = bufferSize;
    } else {
        if (hr == DPERR_INVALIDPLAYER || hr == DPERR_NOMESSAGES || hr != 0)
            goto done;
        *param2 = CGame::m_unk0x005a1fb8;
        return 1;
    }
done:
    return 0;
}


// Removes a player (by DirectPlay id) from the session player table.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a1940
void FUN_004a1940(DPID *pId)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (CGame::m_unk0x005a1820[i].field_0xc8 == *pId) {
            sprintf(CGame::m_unk0x005a1820[i].field_0x0, CMain::m_logFileBlankLine);
            sprintf(CGame::m_unk0x005a1820[i].field_0x64, CMain::m_logFileBlankLine);
            CGame::m_unk0x005a1820[i].field_0xc8 = 0;
            CGame::m_unk0x005a1820[i].field_0xcc = 0;
            CGame::m_unk0x005a1818--;
            return;
        }
    }
}

// One of the four user values of a listed session.
// FUNCTION: CMR2 0x004a1610
int Session_GetListedUserValue(unsigned int session, BYTE index)
{
    switch (index) {
    case 0:
        return (int)SESSIONS[session & 0xff].dwUser1;
    case 1:
        return (int)SESSIONS[session & 0xff].dwUser2;
    case 2:
        return (int)SESSIONS[session & 0xff].dwUser3;
    }
    return (int)SESSIONS[session & 0xff].dwUser4;
}

// FUNCTION: CMR2 0x004a19c0
int FUN_004a19c0(DPID *pId, char *pIndex)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (CGame::m_unk0x005a1820[i].field_0xc8 == *pId) {
            *pIndex = i;
            return 1;
        }
    }
    *pIndex = 0;
    return 0;
}

// FUNCTION: CMR2 0x004a1a00
DPID FUN_004a1a00(void)
{
    return CGame::m_unk0x005a1ea0;
}

// FUNCTION: CMR2 0x004a1b60
char *FUN_004a1b60(BYTE index)
{
    if (CGame::m_unk0x005a1820[index].field_0xcc != 0)
        return CGame::m_unk0x005a1820[index].field_0x64;
    return NULL;
}

// FUNCTION: CMR2 0x004a1b30
Unk0x005a1820 *FUN_004a1b30(BYTE index)
{
    if (CGame::m_unk0x005a1820[index].field_0xcc != 0)
        return &CGame::m_unk0x005a1820[index];
    return NULL;
}

BOOL FUN_004a0d60(void);
bool FUN_004a14e0(void);

// Session name, password and player limit of the network session description.
// FUNCTION: CMR2 0x004a1510
void Session_SetName(LPVOID pName)
{
    FUN_004a0d60();
    g_sessionNamePtr = (LPVOID *)pName;
    FUN_004a14e0();
}

// FUNCTION: CMR2 0x004a1530
void Session_SetPassword(LPVOID pPassword)
{
    FUN_004a0d60();
    g_sessionPasswordPtr = (LPVOID *)pPassword;
    FUN_004a14e0();
}

// FUNCTION: CMR2 0x004a1550
char Session_SetMaxPlayers(int count)
{
    FUN_004a0d60();
    *(int *)(g_unk0x005a0068 + 0x28) = count;
    return FUN_004a14e0();
}

void SceneNode_UpdateTree(SceneNode *pNode, int unused);
void SceneNode_FlushTransforms(SceneNode *pNode);
void Scene_SetViewFromCamera(SceneNode *pCamera);

// Updates a scene tree for drawing from a camera.
// FUNCTION: CMR2 0x0049ce10
int Game_PrepareScene(SceneNode *pRoot, SceneNode *pCamera, int unused, int param)
{
    SceneNode_UpdateTree(pRoot, param);
    SceneNode_FlushTransforms(pRoot);
    Scene_SetViewFromCamera(pCamera);
    return 1;
}

// Network session flag 0x20 (set when `open` is 0).
// FUNCTION: CMR2 0x004a1570
void Session_SetOpen(char open)
{
    FUN_004a0d60();
    if (open != 0) {
        *(DWORD *)(g_unk0x005a0068 + 4) &= 0xffffffdf;
        FUN_004a14e0();
        return;
    }
    *(DWORD *)(g_unk0x005a0068 + 4) |= 0x20;
    FUN_004a14e0();
}

// One of the four session user values (0x40..0x4c of the description).
// FUNCTION: CMR2 0x004a1680
int Session_GetUserValue(BYTE index)
{
    FUN_004a0d60();
    switch (index) {
    case 0:
        return *(int *)(g_unk0x005a0068 + 0x40);
    case 1:
        return *(int *)(g_unk0x005a0068 + 0x44);
    case 2:
        return *(int *)(g_unk0x005a0068 + 0x48);
    }
    return *(int *)(g_unk0x005a0068 + 0x4c);
}
