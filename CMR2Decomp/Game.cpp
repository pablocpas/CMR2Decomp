#include "Game.h"
#include "main.h"
#include "RegKey.h"
#include "GameInfo.h"
#include "InstallInfo.h"
#include "NetworkLeaderboards.h"
#include "Graphics.h"
#include "Input.h"
#include "FileBuffer.h"
#include "Frontend.h"
#include "Texture.h"
#include "Sound.h"
#include "Font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

BOOL CGame::m_shouldExit = FALSE;
BOOL CGame::m_isActive = FALSE;
int CGame::m_unk0x0059ce14;
int CGame::m_unk0x0059ce18;
int CGame::m_unk0x0059ce20;
int CGame::m_unk0x0059ce28;
int CGame::m_unk0x0059ce2c;
void *CGame::m_unk0x00593cb0[4117];
void *CGame::m_unk0x00597d04[4096];
int CGame::m_unk0x005207f8;
int CGame::m_unk0x00663dc4;
DPlayConnection CGame::m_connections[10];
BYTE CGame::m_maxConnections = 10;
BYTE CGame::m_connectionCount;
int CGame::m_unk0x00523c58 = -1;
int CGame::m_unk0x00523c5c = -1;
Unk0049c2c0 CGame::m_unk0x00817da0;
int CGame::m_unk0x0052ea4c;
BYTE CGame::m_unk0x0052ea51;
bool CGame::m_unk0x00817eb0 = false;
Unk00817d98 CGame::m_unk0x00817d98;
BYTE CGame::m_unk0x00523c18 = 0;
BYTE CGame::m_unk0x00593cac;
BYTE CGame::m_unk0x00593ba8;
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



FuncTableGroup CGame::m_initializeGameGroupedFuncTable[10] = {
    {InitializeGame,
     FUN_00501680},
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
    if (strcmp(skuValue, skuRegion) == 0)
       CGameInfo::SetGameRegion(0);
    else
    {
       // america sku check
       skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
       skuRegion = CRegKey::m_skuAmerica;
       if (strcmp(skuValue, skuRegion) == 0)
           CGameInfo::SetGameRegion(1);
       else
       {
           // japan sku check
           skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
           skuRegion = CRegKey::m_skuJapan;
           if (strcmp(skuValue, skuRegion) == 0)
               CGameInfo::SetGameRegion(2);
           else
           {
               // poland sku check
               skuValue = CRegKey::GetValueFromKey(CRegKey::m_regKeySkuType);
               skuRegion = CRegKey::m_skuPoland;
               if (strcmp(skuValue, skuRegion) == 0)
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
        FUN_004083e0(0);
        FUN_00406810(0);
        FUN_004067e0();
        CGameInfo::FUN_00405de0(0);

        // if (CInstallInfo::FUN_0040e8d0() != 0)
        // {
            CGameInfo::FUN_00510410();
            CGameInfo::FUN_00406560();
            CGameInfo::FUN_00406580();
            CGameInfo::SetupInputs();
            didLoadGameInfo = CGameInfo::LoadGameInfo();
            CNetworkLeaderboards::Reset();
            CNetworkLeaderboards::LoadLeaderboards();
            CGameInfo::FUN_00405de0(0);
            if (CGraphics::InitializeDirectX()) {
                CGraphics::SetDefaults();
            }
            CInput::LoadControllerInfo();
            CGameInfo::FUN_0049ea90(1);
            CGameInfo::FUN_004d05d0();
            if (FUN_004aaa40() != false) {
                if (CGame::FUN_004d0a50(true)) {
                    return;
                }
            }
        // }
    }

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

    m_unk0x00593ba8 = m_unk0x00593ba8 & 0xffffff00;
    if (param1->count > 0)
    {
        do
        {
            m_unk0x00593ba4 = &param1->unk[m_unk0x00593ba8];
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

            m_unk0x00593ba8++;
        } while (m_unk0x00593ba8 < param1->count);
    }
}

// FUNCTION: CMR2 0x0049c150
void CGame::FUN_0049c150(Unk00817d98 *param1, int param2, int param3)
{
    param1->field0x2 = 0;
    param1->field0x1 =
        (param1->field0x1 & 0xfc000000) | (param2 & 0xffU) | ((param3 & 0xffU) << 8);
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
// TODO: CMR2 0x0049c1c0 (implemented, match 45%)
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
// GLOBAL: CMR2 0x00526ee0
FuncTableGroup g_unk0x00526ee0[7];
// GLOBAL: CMR2 0x00526f18
void *g_unk0x00526f18;

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
// STUB: CMR2 0x0041b060
BOOL CGame::FUN_0041b060() { return FALSE; }

// FUNCTION: CMR2 0x00501680
void CGame::FUN_00501680(struct Unk0049c2c0 *, BYTE) { return; }

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

// FUNCTION: CMR2 0x004a17f0
void CGame::FUN_004a17f0(bool param1) {
    if (param1)
        m_unk0x005a1fc0 = false;

    Unk0x005a1820 *dest = m_unk0x005a1820;
    do {
        sprintf(dest->field_0x64, CMain::m_logFileBlankLine);
        sprintf(dest->field_0x0, CMain::m_logFileBlankLine);
        dest->field_0xc8 = 0;
        dest->field_0xcc = 0;
        dest++;
    } while ((int)dest < (int)&m_unk0x005a1e34);

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
  FUN_004a17b0();
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
    if (((DPlayConnection *)a)->guidSP == DPSPGUID_TCPIP)
        return -1;
    return ((DPlayConnection *)b)->guidSP == DPSPGUID_TCPIP;
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
        *(DWORD *)&m_connections[i].guidSP.Data4[0] = 0;
        *(DWORD *)&m_connections[i].guidSP.Data4[4] = 0;
    }
    m_connectionCount = 0;
    m_maxConnections = 10;
}

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

// FUNCTION: CMR2 0x004aacf0
unsigned int CGame::GetConnectionCount(void)
{
    unsigned int count = 0;

    count = m_connectionCount;
    return count;
}

// FUNCTION: CMR2 0x004aad00
DPlayConnection *CGame::GetConnection(BYTE index)
{
    if (index < m_connectionCount)
        return &m_connections[index];
    return NULL;
}

// FUNCTION: CMR2 0x004aad30
bool CGame::FUN_004aad30(int param1, int param2, int param3)
{
    return false;
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

// GLOBAL: CMR2 0x0058d49c
void *g_unk0x0058d49c[8];

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

struct Unk004238e0 {
    int field_0x0;
    int field_0x4;
};

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
// TODO: CMR2 0x004a0d60 (implemented, match 81%)
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
    return CGameInfo::m_unk0x005a0098;
}

// FUNCTION: CMR2 0x004a14d0
LPVOID *FUN_004a14d0(void)
{
    return CGameInfo::m_unk0x005a009c;
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
    CGameInfo::m_unk0x005a0098 = (LPVOID *)&CGameInfo::m_unk0x005a00b8;
    CGameInfo::m_unk0x005a009c = (LPVOID *)&CGameInfo::m_unk0x005a02c0;
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
// TODO: CMR2 0x004a1850 (implemented, match 80%)
void FUN_004a1850(char *shortName, char *longName, DPID dpId)
{
    Unk0x005a1820 *pPlayer;
    int i;

    if (CGame::m_unk0x005a1ea0 == dpId)
        return;
    for (pPlayer = CGame::m_unk0x005a1820; pPlayer < CGame::m_unk0x005a1820 + 7; pPlayer++) {
        if (pPlayer->field_0xc8 == dpId)
            return;
    }
    if (CGame::m_unk0x005a1818 >= 7)
        return;
    i = 0;
    for (pPlayer = CGame::m_unk0x005a1820; pPlayer < CGame::m_unk0x005a1820 + 7; pPlayer++, i++) {
        if (pPlayer->field_0xcc == 0) {
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
BOOL FAR PASCAL FUN_004a1ad0(DPID dpId, DWORD dwPlayerType, LPCDPNAME lpName)
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
    return hr == 0;
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
int g_unk0x005a1fa8;
// GLOBAL: CMR2 0x005a1fac
int g_unk0x005a1fac;
// GLOBAL: CMR2 0x005a1fb0
int g_unk0x005a1fb0;
// GLOBAL: CMR2 0x005a1fb4
int g_unk0x005a1fb4;
// TODO: CMR2 0x004a1a10 (implemented, match 48%)
int FUN_004a1a10(int param1, int param2, int param3, int param4)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    g_unk0x005a1fa8 = 0;
    g_unk0x005a1fa8 = 0x10;
    g_unk0x005a1fac = 0;
    g_unk0x005a1fb0 = 0;
    g_unk0x005a1fb0 = param1;
    g_unk0x005a1fb4 = 0;
    g_unk0x005a1fb4 = param2;
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = ((DPMethod6)(*(void ***)pDP)[0x18 / 4])(pDP, CGame::m_unk0x005a1ea0, (DWORD)&g_unk0x005a1fa8,
                                                0, param3, param4, 0);
    if (hr <= (HRESULT)0x88770078 || hr == (HRESULT)0x887700aa || hr != 0)
        return 0;
    CGame::m_unk0x005a1fc0 = 1;
    return 1;
}

// GLOBAL: CMR2 0x0058d3b8
BYTE g_unk0x0058d3b8[0xc0];
// GLOBAL: CMR2 0x0058d4c4
int g_unk0x0058d4c4[4];
// GLOBAL: CMR2 0x0058d530
BYTE g_unk0x0058d530[0x1c * 2];
// GLOBAL: CMR2 0x0058d6a0
void *g_unk0x0058d6a0[2];

// Releases the scene resources held by the 0x58d3xx/0x58d5xx/0x58d6xx blocks.
// TODO: CMR2 0x004779e0 (implemented, match 74%)
bool FUN_004779e0(void)
{
    void **pA;
    BYTE *pB;
    void **pC;
    int *pD;

    pA = g_unk0x0058d49c;
    pB = g_unk0x0058d530;
    while (pA < &g_unk0x0058d49c[2]) {
        if (*pA != NULL) {
            if (*(int *)pB != 0)
                SceneNode_Destroy((SceneNode *)*(int *)pB);
            SceneNode_Destroy((SceneNode *)*pA);
            *(int *)(pB + 0x0) = 0;
            *(int *)(pB + 0x4) = 0;
            *(int *)(pB + 0x8) = 0;
            *(int *)(pB + 0xc) = 0;
            *(int *)(pB + 0x10) = 0;
        }
        pA++;
        pB += 0x1c;
    }
    pC = g_unk0x0058d6a0;
    while (pC < &g_unk0x0058d6a0[2]) {
        if (*pC != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*pC);
            *pC = NULL;
        }
        pC++;
    }
    pD = g_unk0x0058d4c4;
    do {
        pD[-1] = 0;
        pD[0] = 0;
        pD += 2;
    } while (pD < &g_unk0x0058d4c4[4]);
    pB = g_unk0x0058d3b8;
    while (pB < &g_unk0x0058d3b8[0xc0]) {
        if (*(void **)pB != NULL) {
            SceneNode_Destroy((SceneNode *)*(void **)pB);
            *(void **)pB = NULL;
        }
        *(int *)(pB + 0x0) = 0;
        *(int *)(pB + 0x4) = 0;
        *(int *)(pB + 0x8) = 0;
        pB += 0xc;
    }
    return true;
}

// Adds the player slot to the DirectPlay session.
// TODO: CMR2 0x004aac40 (implemented, match 63%)
bool FUN_004aac40(int param1)
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
// TODO: CMR2 0x004a1b90 (implemented, match 44%)
int FUN_004a1b90(int param1, void **param2)
{
    IDirectPlay4A *pDP;
    HRESULT hr;
    BOOL local1;
    int local2;
    void *pBuffer;

    local1 = CGame::m_unk0x005a1fbc;
    local2 = 0;
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return 0;
    hr = ((DPMethod5)(*(void ***)pDP)[0x64 / 4])(pDP, (DWORD)CGame::m_unk0x005a1fb8, 1,
                                                (DWORD)&local2, (DWORD)param1, (DWORD)&local1);
    if (hr > (HRESULT)0x88770082) {
        if (hr == (HRESULT)0x88770096 || hr == (HRESULT)0x887700be || hr != 0)
            return 0;
        *param2 = CGame::m_unk0x005a1fb8;
        return 1;
    }
    if (hr == (HRESULT)0x88770082 || hr == (HRESULT)0x80004005 ||
        hr == (HRESULT)0x80070057 || hr != (HRESULT)0x8877001e)
        return 0;
    if (CGame::m_unk0x005a1fb8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(CGame::m_unk0x005a1fb8);
        CGame::m_unk0x005a1fb8 = NULL;
    }
    pBuffer = CFileBuffer::AllocateLockedBuffer(local2);
    CGame::m_unk0x005a1fb8 = pBuffer;
    if (pBuffer == NULL)
        return 0;
    CGame::m_unk0x005a1fbc = local2;
    return 0;
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
