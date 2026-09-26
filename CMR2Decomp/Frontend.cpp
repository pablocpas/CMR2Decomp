#include "Frontend.h"
#include "Game.h"
#include "FileBuffer.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "InstallInfo.h"
#include "main.h"
#include "Graphics.h"
#include "Sprite.h"
#include "FixedPoint.h"
#include "Texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Not ported/declared elsewhere yet.
void Scene_SetAmbient(BYTE *pColour, int boost);
void FUN_004ae260(void);
void FUN_004b1150(void);
void FUN_004b7b20(void);
void FUN_004d0840(void);

// Callees that live in other translation units and are not declared in their
// headers yet.
BYTE *RallyData_FUN_00408c70(int index);
BYTE *RallyData_FUN_00408cb0(int index);
unsigned int RallyData_GetFlag30(void);
unsigned int RallyData_GetFlag23(void);
BYTE FUN_004086f0(BYTE param1);
void RallyData_MarkTyresChanged(int index);
BYTE FUN_004071c0(BYTE flags, char mode);
BYTE FUN_00448cb0(int index);
BYTE FUN_00448cc0(void);
unsigned int FUN_00448680(int index, int split);
int FUN_00448240(int car, int index);
int FUN_004582d0(int index);
int FUN_004583a0(void);
extern BYTE g_unk0x008180fa;

// Defined later in this file (device setup helpers).
BYTE *FUN_004d0280(int index);
BYTE *FUN_004d02d0(int index);
void FUN_004d0300(int *pDest, int *pSource);
void FUN_004d0310(int *pDest, int *pSource);
void FUN_004d0370(int *pDest, int *pSource);
void FUN_004d03b0(int *pDest, int *pSource);

char CFrontend::m_stringDest[MAX_PATH];

char CFrontend::m_feRes640CountrySpecific[19] = "%s%sFERes640%c.bfl";
char CFrontend::m_feRes640CCountrySpecific[20] = "%s%sFERes640%cC.bfl";
char CFrontend::m_feRes1024CountrySpecific[20] = "%s%sFERes1024%c.bfl";
char CFrontend::m_feRes1024CCountrySpecific[21] = "%s%sFERes1024%cC.bfl";
char CFrontend::m_feRes640[14] = "%s\\Res640.bfl";
char CFrontend::m_feRes640C[15] = "%s\\Res640C.bfl";
char CFrontend::m_feRes1024[15] = "%s\\Res1024.bfl";
char CFrontend::m_feRes1024C[16] = "%s\\Res1024C.bfl";
char CFrontend::m_strFrontendTexturesAr640ATGA[36] = "%s\\frontend\\Textures\\Ar_640A.tga";
char CFrontend::m_strFrontendTexturesAr640DTGA[36] = "%s\\frontend\\Textures\\Ar_640D.tga";
char CFrontend::m_strFrontendTexturesLgMatrixTGA[36] = "%s\\frontend\\Textures\\LgMatrix.tga";
char CFrontend::m_strFrontendTexturesSmMatrixTGA[36]= "%s\\frontend\\Textures\\SmMatrix.tga";

char CFrontend::m_strFrontEndBinkTGA[24] = "%s\\FrontEnd\\bink.tga";
char CFrontend::m_strFrontEndTexturesCopyright[40] = "%s\\FrontEnd\\Textures\\Copyright%d.tga";
char CFrontend::m_strFrontEndTexturesCmr2TGA[32] = "%s\\FrontEnd\\Textures\\cmr2.tga";

Texture* CFrontend::m_pAr640ATexture;
Texture* CFrontend::m_pAr640DTexture;
Texture* CFrontend::m_pLgMatrixTexture;
Texture* CFrontend::m_pSmMatrixTexture;
Texture* CFrontend::m_pSetupRepBanners[8];
Texture* CFrontend::m_pTinyFlags[8];
Texture* CFrontend::m_pTBronze;
Texture* CFrontend::m_pTSilver;
Texture* CFrontend::m_pTGold;

Texture* CFrontend::m_unk0x00817ebc;
Texture* CFrontend::m_unk0x00817fd0;
Texture* CFrontend::m_unk0x00817fd4[4];

char CFrontend::m_strUK[3] = "UK";
char CFrontend::m_strIta[4] = "Ita";
char CFrontend::m_strKen[4] = "Ken";
char CFrontend::m_strItaly[8] = "Italy";
char CFrontend::m_strAus[4] = "Aus";
char CFrontend::m_strSwe[4] = "Swe";
char CFrontend::m_strKenya[8] = "Kenya";
char CFrontend::m_strFra[4] = "Fra";
char CFrontend::m_strSweden[8] = "Sweden";
char CFrontend::m_strFrance[8] = "France";
char CFrontend::m_strGre[4] = "Gre";
char CFrontend::m_strFin[4] = "Fin";
char CFrontend::m_strFinland[8] = "Finland";
char CFrontend::m_strGreece[8] = "Greece";

char CFrontend::m_strEsc[4] = "Esc";
char CFrontend::m_strPum[4] = "Pum";
char CFrontend::m_str205[4] = "205";
char CFrontend::m_strStr[4] = "Str";
char CFrontend::m_str6R4[4] = "6R4";
char CFrontend::m_strMin[4] = "Min";
char CFrontend::m_strSie[4] = "Sie";
char CFrontend::m_strIA2[4] = "IA2";
char CFrontend::m_strIA1[4] = "IA1";
char CFrontend::m_strInt[4] = "Int";
char CFrontend::m_strSea[4] = "Sea";
char CFrontend::m_str206[4] = "206";
char CFrontend::m_strSubShort[4] = "Sub";
char CFrontend::m_strCor[4] = "Cor";
char CFrontend::m_strMA3[4] = "MA3";
char CFrontend::m_strMA2[4] = "MA2";
char CFrontend::m_strMA1[4] = "MA1";
char CFrontend::m_strMit[4] = "Mit";
char CFrontend::m_strFA2[4] = "FA2";
char CFrontend::m_strFA1[4] = "FA1";
char CFrontend::m_strF99Short[4] = "F99";
char CFrontend::m_strFoc[4] = "Foc";
char CFrontend::m_strEscort[7] = "escort";
char CFrontend::m_strPuma[7] = "puma";
char CFrontend::m_strStrts[7] = "strts";
char CFrontend::m_str6r4[4] = "6r4";
char CFrontend::m_strMini[7] = "mini";
char CFrontend::m_strSerra[7] = "serra";
char CFrontend::m_strDelta[7] = "delta";
char CFrontend::m_strCrdba[7] = "crdba";
char CFrontend::m_strSub[4] = "sub";
char CFrontend::m_strCrlla[7] = "crlla";
char CFrontend::m_strLncer[7] = "lncer";
char CFrontend::m_strF99[4] = "f99";
char CFrontend::m_strF2000[7] = "f2000";

char CFrontend::m_strSetupRepTexturesBanners[40] = "%s\\setuprep\\Textures\\banners\\b%s.tga";
char CFrontend::m_strFrontendTinyFlags[40] = "%s\\frontend\\Textures\\tinyflags\\t%s.tga";
char CFrontend::m_strFrontendTexturesTBronzeTGA[36] = "%s\\frontend\\Textures\\tbronze.tga";
char CFrontend::m_strFrontendTexturesTSilverTGA[36] = "%s\\frontend\\Textures\\tsilver.tga";
char CFrontend::m_strFrontendTexturesTGoldTGA[32] = "%s\\frontend\\Textures\\tgold.tga";

char CFrontend::m_strFrontendTexturesCarsLivery[40] = "%s\\frontend\\Textures\\cars\\livery%d.tga";
char CFrontend::m_strFrontendTexturesCarsB01[36] = "%s\\frontend\\Textures\\cars\\%sB01.tga";
char CFrontend::m_strFrontendTexturesCarsF01[36] = "%s\\frontend\\Textures\\cars\\%sF01.tga";

GenericFile CFrontend::m_unk0x00818260;
GenericFile CFrontend::m_languageFiles[5];
GenericFile CFrontend::m_commonFile;
char CFrontend::m_strCommonBfl[14] = "%s\\Common.bfl";
char CFrontend::m_strEnglishTextBfl[20] = "%s%sEnglishText.bfl";
char CFrontend::m_strFrenchTextBfl[19] = "%s%sFrenchText.bfl";
char CFrontend::m_strGermanTextBfl[19] = "%s%sGermanText.bfl";
char CFrontend::m_strSpanishTextBfl[20] = "%s%sSpanishText.bfl";
char CFrontend::m_strItalianTextBfl[20] = "%s%sItalianText.bfl";
char CFrontend::m_strPolishTextBfl[19] = "%s%sPolishText.bfl";
char CFrontend::m_strEngUSATextBfl[19] = "%s%sEngUSAText.bfl";

Texture* CFrontend::m_unk0x00818530[3];
char* CFrontend::m_unk0x0081853c;
Texture* CFrontend::m_unk0x008182cc[22];
Texture* CFrontend::m_unk0x0081884c[22];

// FUNCTION: CMR2 0x004b7520
DWORD CFrontend::FUN_004b7520(void)
{
    return CGraphics::m_d3dDeviceDesc7.field0xa8;
}

// FUNCTION: CMR2 0x004d21e0
void CFrontend::FUN_004d21e0(void)
{
    unsigned int regionID;
    char regionKey;

    if (CGenericFileLoader::m_genericFile.buffer != NULL)
    {
        CFileBuffer::FreeGenericFileBuffer(CGenericFileLoader::m_genericFile.buffer);
        CGenericFileLoader::m_genericFile.buffer = NULL;
    }

    CGenericFileLoader::m_genericFile.didFileLoad = 0;
    CGenericFileLoader::m_genericFile.fileSize = 0;

    if (CGameInfo::GetScreenWidth() >= 1024U)
    {
        if (FUN_004b7560(1024) != 0)
        {
            if (FUN_004b7590(1024) != 0)
            {
                if (FUN_004a9700() != FALSE)
                    sprintf(m_stringDest, m_feRes1024C, CInstallInfo::GetFrontendDir());

                else
                    sprintf(m_stringDest, m_feRes1024, CInstallInfo::GetFrontendDir());
            }
        }
    }
    else
    {

        if (FUN_004a9700() != FALSE)
            sprintf(m_stringDest, m_feRes640C, CInstallInfo::GetFrontendDir());
        else
            sprintf(m_stringDest, m_feRes640, CInstallInfo::GetFrontendDir());
    }

    CGenericFileLoader::FUN_004a9d70(&CGenericFileLoader::m_genericFile, m_stringDest);
    if (m_unk0x00818260.buffer != NULL)
    {
        CFileBuffer::FreeGenericFileBuffer(m_unk0x00818260.buffer);
        m_unk0x00818260.buffer = NULL;
    }
    m_unk0x00818260.didFileLoad = 0;
    m_unk0x00818260.fileSize = 0;

    // almost certainly not how this was written but this gets us better instruction matching
    // this is basically (if regionID is 3, then use poland)
    regionID = CGameInfo::GetGameRegion();
    if (regionID && --regionID && --regionID)
        regionKey = 0x50; // P
    else
        regionKey = 0x45; // E

    if (CGameInfo::GetScreenWidth() >= 1024U)
    {
        if (FUN_004b7560(1024) != FALSE)
        {
            if (FUN_004b7590(1024) != FALSE)
            {
                if (FUN_004a9700())
                    sprintf(m_stringDest, m_feRes1024CCountrySpecific, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory(), regionKey);
                else
                    sprintf(m_stringDest, m_feRes1024CountrySpecific, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory(), regionKey);
            }
        }
    }
    else
    {
        if (FUN_004a9700())
            sprintf(m_stringDest, m_feRes640CCountrySpecific, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory(), regionKey);
        else
            sprintf(m_stringDest, m_feRes640CountrySpecific, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory(), regionKey);
    }

    CGenericFileLoader::FUN_004a9d70(&m_unk0x00818260, m_stringDest);
}

// FUNCTION: CMR2 0x004b7560
BOOL CFrontend::FUN_004b7560(unsigned int param_1)
{
    if (CGraphics::m_d3dDeviceDesc7.minTextureWidth <= param_1 && param_1 <= CGraphics::m_d3dDeviceDesc7.maxTextureWidth)
        return TRUE;

    return FALSE;
}

// FUNCTION: CMR2 0x004b7590
BOOL CFrontend::FUN_004b7590(unsigned int param_1)
{
    if (CGraphics::m_d3dDeviceDesc7.minTextureHeight <= param_1 && param_1 <= CGraphics::m_d3dDeviceDesc7.maxTextureHeight)
        return TRUE;

    return FALSE;
}

// FUNCTION: CMR2 0x004a9700
BOOL CFrontend::FUN_004a9700(void)
{
    if (!CGraphics::m_hasTexFormatDXT1_16 && !CGraphics::m_hasTexFormatDXT1_32)
        return FALSE;

    return TRUE;
}

// FUNCTION: CMR2 0x004d2590
void CFrontend::FUN_004d2590(void) {
    int index;
    int ix;
    const char* carNames[22] = {
        m_strF2000,
        m_strF99,
        CMain::m_logFileBlankLine,
        CMain::m_logFileBlankLine,
        m_strLncer,
        CMain::m_logFileBlankLine,
        CMain::m_logFileBlankLine,
        CMain::m_logFileBlankLine,
        m_strCrlla,
        m_strSub,
        m_str206,
        m_strCrdba,
        m_strDelta,
        CMain::m_logFileBlankLine,
        CMain::m_logFileBlankLine,
        m_strSerra,
        m_strMini,
        m_str6r4,
        m_strStrts,
        m_str205,
        m_strPuma,
        m_strEscort
    };

    index = -2;
    ix = 0;
    
    do {
        const char* carName = carNames[ix];
        if (carName[0] != '\0') {
            // Load front texture (F01)
            sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesCarsF01, CInstallInfo::GetGameCDPath(), carName);
            m_unk0x0081884c[ix] = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, 0, 0, 0, 0);
            
            // Load back texture (B01)
            sprintf(CFrontend::m_stringDest,CFrontend::m_strFrontendTexturesCarsB01, CInstallInfo::GetGameCDPath(), carName);
            m_unk0x008182cc[ix] = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, 0, 0, 0, 0);
        } else {
            // Handle blank entries with switch/case based on index
            switch(index) {
                case 0:  // index 0, 1
                case 1:
                    m_unk0x0081884c[ix] = m_unk0x0081884c[1];
                    m_unk0x008182cc[ix] = m_unk0x008182cc[1];
                    break;
                    
                case 3:   // index 3, 4, 5
                case 4:
                case 5:
                    m_unk0x0081884c[ix] = m_unk0x0081884c[4];
                    m_unk0x008182cc[ix] = m_unk0x008182cc[4];
                    break;
                    
                case 11:  // index 11, 12
                case 12:
                    m_unk0x0081884c[ix] = m_unk0x0081884c[12];
                    m_unk0x008182cc[ix] = m_unk0x008182cc[12];
                    break;
            }
        }
        
        index++;
        ix++;
    } while (index + 2 < 22);
    
    // Load livery textures (1-3)
    Texture** pTexture = m_unk0x00818530;
    int liveryIndex = 0;

    do {
        liveryIndex++;  // Increment first so we use 1, 2, 3
        sprintf(CFrontend::m_stringDest, CFrontend::m_strFrontendTexturesCarsLivery, CInstallInfo::GetGameCDPath(), liveryIndex);
        *pTexture = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, 0, 0, 0, 0);
        pTexture++;
    } while ((int)pTexture < (int)&m_unk0x0081853c[0]);
}

// Returns the common frontend archive (Common.bfl).
// FUNCTION: CMR2 0x004d2190
GenericFile* CFrontend::FUN_004d2190(void) {
    return &m_commonFile;
}

// Returns the archive the copyright screens are loaded from (the one
// FUN_004d21e0 opens).
// FUNCTION: CMR2 0x004d21b0
GenericFile* CFrontend::FUN_004d21b0(void) {
    return &m_unk0x00818260;
}

// Returns the archive of one frontend language.
// FUNCTION: CMR2 0x004d21c0
GenericFile* CFrontend::FUN_004d21c0(int language) {
    return &m_languageFiles[language];
}

// Loads the splash screen textures: the CMR2 logo, one "Copyright" screen per
// page and the Bink logo. The textures live inside the .bfl archives that
// LoadLanguageFiles opens, so they are looked up by name in those files.
// FUNCTION: CMR2 0x004d08d0
bool CFrontend::LoadSplashScreens(bool param1) {
    DWORD colour = 0xffffffff;
    int screenCount;
    int i;

    if (param1) {
        CGraphics::FUN_004a78a0(CGameInfo::GetScreenWidth(), CGameInfo::GetScreenHeight(), CGameInfo::GetColourDepth(), CGameInfo::FUN_00405bd0(), CGameInfo::FUN_00405c00());
        FUN_004b7b20();
        Sprite_Init();
        Line2D_Init();
        Tri2D_Init();
    }

    RallyData_ValidateIndex(0);
    FUN_004ae260();
    FUN_004d20c0();
    LoadLanguageFiles();

    screenCount = 3;
    if (CGameInfo::GetScreenWidth() >= 0x400) {
        if (FUN_004b7560(0x400) != 0 && FUN_004b7590(0x400) != 0)
            screenCount = 4;
    }

    FUN_004d0840();
    CGame::FUN_0049dca0(3);
    FUN_004b1150();
    Scene_SetAmbient((BYTE *)&colour, 0);

    sprintf(m_stringDest, m_strFrontEndTexturesCmr2TGA, CInstallInfo::GetGameCDPath());
    m_unk0x00817fd0 = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), m_stringDest, 0, 0, 0, 0);

    for (i = 0; i < screenCount; i++) {
        sprintf(m_stringDest, m_strFrontEndTexturesCopyright, CInstallInfo::GetGameCDPath(), i + 1);
        m_unk0x00817fd4[i] = CTexture::FindLoadTexture(FUN_004d21b0(), m_stringDest, 0, 0, 0, 0);
    }

    sprintf(m_stringDest, m_strFrontEndBinkTGA, CInstallInfo::GetGameCDPath());
    m_unk0x00817ebc = CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), m_stringDest, 0, 0, 0, 0);

    return true;
}

// FUNCTION: CMR2 0x004d2100
bool CFrontend::ReleaseLanguageFiles(void)
{
    int i;

    if (m_commonFile.buffer != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_commonFile.buffer);
        m_commonFile.buffer = NULL;
    }
    m_commonFile.didFileLoad = FALSE;
    m_commonFile.fileSize = 0;
    if (CGenericFileLoader::m_genericFile.buffer != NULL) {
        CFileBuffer::FreeGenericFileBuffer(CGenericFileLoader::m_genericFile.buffer);
        CGenericFileLoader::m_genericFile.buffer = NULL;
    }
    CGenericFileLoader::m_genericFile.didFileLoad = FALSE;
    CGenericFileLoader::m_genericFile.fileSize = 0;
    if (m_unk0x00818260.buffer != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_unk0x00818260.buffer);
        m_unk0x00818260.buffer = NULL;
    }
    m_unk0x00818260.didFileLoad = FALSE;
    m_unk0x00818260.fileSize = 0;
    for (i = 0; i < 5; i++) {
        if (m_languageFiles[i].buffer != NULL) {
            CFileBuffer::FreeGenericFileBuffer(m_languageFiles[i].buffer);
            m_languageFiles[i].buffer = NULL;
        }
        m_languageFiles[i].didFileLoad = FALSE;
        m_languageFiles[i].fileSize = 0;
    }
    return true;
}

// FUNCTION: CMR2 0x004d2380
void CFrontend::LoadLanguageFiles(void)
{
    char *format;

    sprintf(m_stringDest, m_strCommonBfl, CInstallInfo::GetFrontendDir());
    CGenericFileLoader::FUN_004a9d70(&m_commonFile, m_stringDest);
    FUN_004d21e0();

    switch (CGameInfo::GetGameRegion()) {
    case 0:
        sprintf(m_stringDest, m_strEnglishTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[0], m_stringDest);
        sprintf(m_stringDest, m_strFrenchTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[1], m_stringDest);
        sprintf(m_stringDest, m_strGermanTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[4], m_stringDest);
        sprintf(m_stringDest, m_strSpanishTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[2], m_stringDest);
        sprintf(m_stringDest, m_strItalianTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[3], m_stringDest);
        break;
    case 1:
        sprintf(m_stringDest, m_strEngUSATextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[0], m_stringDest);
        sprintf(m_stringDest, m_strFrenchTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[1], m_stringDest);
        sprintf(m_stringDest, m_strSpanishTextBfl, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[2], m_stringDest);
        break;
    case 2:
        format = m_strEnglishTextBfl;
        sprintf(m_stringDest, format, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[0], m_stringDest);
        break;
    default:
        format = m_strPolishTextBfl;
        sprintf(m_stringDest, format, CInstallInfo::GetCountrySpecificDir(), CGameInfo::GetGameRegionDirectory());
        CGenericFileLoader::FUN_004a9d70(&m_languageFiles[0], m_stringDest);
        break;
    }
    CGame::RegisterCallback(ReleaseLanguageFiles, NULL);
}

char **CFrontend::m_textStrings;
int CFrontend::m_textFirstId;
int CFrontend::m_textCount;
char CFrontend::m_strInvalidTextString[20] = "INVALID TEXT STRING";
BYTE CFrontend::m_unk0x0065aa70;
BYTE CFrontend::m_unk0x0065aa71;

// Looks up one of the loaded text strings; out-of-range ids fall back to the
// "INVALID TEXT STRING" placeholder.
// Selects the language of the text strings.
// FUNCTION: CMR2 0x004a3c30
void FUN_004a3c30(int language)
{
    if (language < CFrontend::m_unk0x0065aa71) {
        CFrontend::m_unk0x0065aa70 = language;
        CFrontend::m_textFirstId = CFrontend::m_textCount * language;
    }
}

BYTE *FUN_004a3d50(BYTE *p, char next);

// Splits the loaded text files (one per language) into the string table.
// FUNCTION: CMR2 0x004a3c90
void CFrontend::FUN_004a3c90(int languages, int count, BYTE **pFiles)
{
    int i;
    int j;
    int row;
    BYTE *p;

    m_unk0x0065aa71 = (BYTE)languages;
    m_unk0x0065aa70 = 0;
    m_textFirstId = 0;
    m_textCount = count;
    m_textStrings = (char **)CFileBuffer::AllocateLockedBuffer(languages * count * 4);
    if (m_textStrings != NULL) {
        for (i = 0, row = 0; i < languages; i++, row += count) {
            p = pFiles[i];
            for (j = 0; j < count; j++) {
                m_textStrings[row + j] = (char *)p;
                if (j < count - 1)
                    p = FUN_004a3d50(p, 1);
                else
                    FUN_004a3d50(p, 0);
            }
        }
    }
}

// FUNCTION: CMR2 0x004a3c60
char *CFrontend::GetTextString(int index)
{
    if (index >= m_textCount || index < 0)
        return m_strInvalidTextString;
    return m_textStrings[m_textFirstId + index];
}

// GLOBAL: CMR2 0x00817fe4
int g_unk0x00817fe4;
// GLOBAL: CMR2 0x00817fe8
int g_unk0x00817fe8;
// GLOBAL: CMR2 0x00817fec
BYTE g_unk0x00817fec;
// GLOBAL: CMR2 0x00817ff4
unsigned int g_unk0x00817ff4;

// FUNCTION: CMR2 0x004d20c0
void CFrontend::FUN_004d20c0(void)
{
    g_unk0x00817fe8 = CGame::GetCallbackCount();
}

// FUNCTION: CMR2 0x004d20d0
unsigned int CFrontend::FUN_004d20d0(void)
{
    return g_unk0x00817ff4;
}

// FUNCTION: CMR2 0x004d20e0
int CFrontend::FUN_004d20e0(void)
{
    return g_unk0x00817fe4;
}

// FUNCTION: CMR2 0x004d20f0
BYTE CFrontend::FUN_004d20f0(void)
{
    return g_unk0x00817fec;
}

// GLOBAL: CMR2 0x00818540
char *g_unk0x00818540;
// GLOBAL: CMR2 0x00818544
char *g_unk0x00818544;

// Colours of the background matrix cells (RGBA), indexed by the cell map.
// GLOBAL: CMR2 0x00523ef0
unsigned int g_matrixColours[16] = {
    0xff404040, 0xffffffff, 0xff0000ff, 0xff00ff00,
    0xffff0000, 0xff00ffff, 0xffde9c08, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xff000000,
    0xff000000, 0xff000000, 0xff000000, 0xffff00ff,
};
// Ripple phases of the background matrix, advanced with the frame time.
// GLOBAL: CMR2 0x008189b0
short g_matrixPhase1;
// GLOBAL: CMR2 0x008189b2
short g_matrixPhase2;
extern short g_unk0x008189a8[4];

// Height of a ripple centred on pCentre at (x, y): sine of the distance,
// with the given phase and wavelength (all 16.16).
// FUNCTION: CMR2 0x004d2bd0
int FUN_004d2bd0(int *pCentre, int x, int y, int phase, int wavelength)
{
    FixVector d;

    d.z = 0;
    d.x = x - pCentre[0];
    d.y = y - pCentre[1];
    return g_sinTable[(FixDiv(FixVecLength(&d) % wavelength, wavelength) + phase) & 0xfff];
}

// Draws the one-pixel outline of pRect (x, y, width, height) in pColour as
// four one-pixel thick rectangles.
// FUNCTION: CMR2 0x004d27e0
void FUN_004d27e0(short *pRect, BYTE *pColour)
{
    short rect[4];

    rect[0] = pRect[0];
    rect[1] = pRect[1] + 1;
    rect[2] = 1;
    rect[3] = pRect[3] - 2;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 4);
    rect[0] = pRect[2] + pRect[0] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 4);
    rect[0] = pRect[0];
    rect[1] = pRect[1];
    rect[2] = pRect[2];
    rect[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 4);
    rect[1] = pRect[1] + pRect[3] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 4);
}

// Draws the animated frontend background: an 18x12 grid of the large matrix
// texture, coloured by pMap and rippling from two centres (top-left and
// top-right of the screen).
// FUNCTION: CMR2 0x004d28c0
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
void FUN_004d28c0(short x0, short y0, char *pMap)
{
    int centre2[2];
    int centre1[2];
    BYTE colour[4];
    char *pCell;
    int u;
    int v;
    int wave1;
    int wave;
    int brightness;
    int i;
    int j;

    colour[0] = 0xff;
    centre1[0] = 0;
    centre1[1] = 0;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    centre2[0] = g_pGraphics->resX;
    centre2[1] = 0;
    g_matrixPhase1 = (short)(((CMain::GetFrameDelta() + 1) * -0x6000) / 360);
    g_matrixPhase2 = (short)(((CMain::GetFrameDelta() + 1) * -0x3000) / 360);
    g_unk0x008189a8[2] = CFrontend::m_pLgMatrixTexture->width;
    g_unk0x008189a8[3] = CFrontend::m_pLgMatrixTexture->height;
    for (i = 0; i < 0x12; i++) {
        pCell = pMap + i;
        for (j = 0; j < 0xc; j++) {
            g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 18) / 640 * i + x0;
            g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 18) / 480 * j + y0;
            u = FixDiv((int)(__int64)(g_unk0x008189a8[0] * CGraphics::m_65536),
                       (int)(__int64)((int)g_pGraphics->resX * CGraphics::m_65536));
            v = FixDiv((int)(__int64)(g_unk0x008189a8[1] * CGraphics::m_65536),
                       (int)(__int64)((int)g_pGraphics->resX * CGraphics::m_65536));
            wave1 = FUN_004d2bd0(centre1, u, v, g_matrixPhase1, 0x20000);
            wave = (FUN_004d2bd0(centre2, u, v, g_matrixPhase2, 0x140000) + wave1) / 2;
            g_unk0x008189a8[0] -= FixMulShift32(0x30000, wave);
            g_unk0x008189a8[1] -= FixMulShift32(0x30000, wave);
            *(unsigned int *)colour = g_matrixColours[*pCell];
            brightness = wave / 4 + 0xc000;
            colour[0] = FixMulShift32((int)(__int64)(colour[0] * CGraphics::m_65536), brightness);
            colour[1] = FixMulShift32((int)(__int64)(colour[1] * CGraphics::m_65536), brightness);
            colour[2] = FixMulShift32((int)(__int64)(colour[2] * CGraphics::m_65536), brightness);
            Sprite_Queue((SpriteRect *)&CFrontend::m_pLgMatrixTexture->field_0x11c, (SpriteRect *)g_unk0x008189a8,
                         CFrontend::m_pLgMatrixTexture, 1, 0, NULL, NULL, colour, 8);
            pCell += 0x12;
        }
    }
}

// GLOBAL: CMR2 0x00818848
BYTE g_unk0x00818848;

// FUNCTION: CMR2 0x004d27d0
BYTE FUN_004d27d0(void)
{
    return g_unk0x00818848;
}

// FUNCTION: CMR2 0x004d2790
void CFrontend::FUN_004d2790(void)
{
    CFrontend::m_unk0x0081853c = GetTextString(0xd0);
    g_unk0x00818540 = GetTextString(0xd1);
    g_unk0x00818544 = GetTextString(0xd2);
}

// Releases the text string table.
// Terminates the line at p and, when asked, returns the start of the next one.
// FUNCTION: CMR2 0x004a3d50
BYTE *FUN_004a3d50(BYTE *p, char next)
{
    while (*p >= 0x20)
        p++;
    *p = 0;
    if (next != 0) {
        do {
            p++;
        } while (*p < 0x20);
        return p;
    }
    return NULL;
}

// FUNCTION: CMR2 0x004a3d80
void CFrontend::FUN_004a3d80(void)
{
    if (m_textStrings != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_textStrings);
        m_textStrings = NULL;
        m_unk0x0065aa70 = 0;
        m_textFirstId = 0;
        m_unk0x0065aa71 = 0;
        m_textCount = 0;
    }
}

// GLOBAL: CMR2 0x00817404
int g_unk0x00817404;
// GLOBAL: CMR2 0x008173fc
int g_unk0x008173fc;

// FUNCTION: CMR2 0x004cfe50
unsigned int CFrontend::FUN_004cfe50(void)
{
    GameInfo0xa4 *pInfo;
    int index;

    pInfo = CGameInfo::FUN_00405fe0();
    index = g_unk0x00817404 * 3 + g_unk0x008173fc;
    return (*(unsigned int *)((char *)pInfo + index * 8 + 0x1214) >> 7) & 0xffff;
}

// FUNCTION: CMR2 0x0040ede0
char *CFrontend::FUN_0040ede0(int offset)
{
    if (CGame::FUN_004057d0() == 0)
        return GetTextString(offset + 0x98);
    if (CGame::FUN_004057d0() == 3)
        return GetTextString(offset + 0xc2);
    return NULL;
}

// FUNCTION: CMR2 0x0040ee20
char *CFrontend::FUN_0040ee20(int offset)
{
    if (CGame::FUN_004057d0() == 0)
        return GetTextString(offset + 0xae);
    if (CGame::FUN_004057d0() == 3)
        return GetTextString(offset + 0xd8);
    return NULL;
}

// One entry of the input state block (0x817420): two one-shot flags, the bit
// mask of the keys held and a counter of the frames each key has been held.
struct InputKeyState {
    BYTE field_0x0[2];
    BYTE field_0x2;
    BYTE field_0x3;
    short field_0x4;
    short field_0x6;
    short field_0x8;
};

// GLOBAL: CMR2 0x008173f0
int g_unk0x008173f0;
// GLOBAL: CMR2 0x008173f4
int g_unk0x008173f4;
// GLOBAL: CMR2 0x00817400
int g_unk0x00817400;
// Per-player input state, at most 4 players; the split-time arrays at
// 0x817448 start right after the 4th record.
// GLOBAL: CMR2 0x00817420
BYTE g_unk0x00817420[4 * 0xa];
// GLOBAL: CMR2 0x00817410
BYTE g_unk0x00817410;
// GLOBAL: CMR2 0x00817411
BYTE g_unk0x00817411;
// GLOBAL: CMR2 0x00817412
BYTE g_unk0x00817412;
// GLOBAL: CMR2 0x00817413
BYTE g_unk0x00817413;
// GLOBAL: CMR2 0x00817414
short g_unk0x00817414;
// GLOBAL: CMR2 0x00817416
short g_unk0x00817416;
// GLOBAL: CMR2 0x00817418
short g_unk0x00817418;

void FUN_004d0230(void);

// Reads the player/mode values of the game info block, clears the per-device
// input state for every device and resets the global key state.
// FUNCTION: CMR2 0x004cf060
void CFrontend::FUN_004cf060(void)
{
    InputKeyState *pState;
    int i;

    g_unk0x008173f0 = CGameInfo::FUN_00405d70() & 0xff;
    g_unk0x008173f4 = CGameInfo::FUN_00405d80() & 0xff;
    g_unk0x00817400 = CGameInfo::FUN_00405d90() & 0xff;
    FUN_004d0230();
    pState = (InputKeyState *)g_unk0x00817420;
    for (i = 0; i < g_unk0x008173f0; i++) {
        pState->field_0x8 = 0;
        pState->field_0x4 = 0;
        pState->field_0x6 = 0;
        pState->field_0x0[0] = 0;
        pState->field_0x0[1] = 0;
        pState->field_0x2 = 0;
        pState->field_0x3 = 0;
        pState++;
    }
    g_unk0x00817418 = 0;
    g_unk0x00817414 = 0;
    g_unk0x00817416 = 0;
    g_unk0x00817410 = 0;
    g_unk0x00817411 = 0;
    g_unk0x00817412 = 0;
    g_unk0x00817413 = 0;
}

// FUNCTION: CMR2 0x004cf0f0
void CFrontend::FUN_004cf0f0(void)
{
    int i;
    BYTE index;

    for (i = 0; i < g_unk0x008173f0; i++) {
        index = RallyDataStageIndex();
        g_unk0x00817420[i * 0xa] = 0;
        if (index == 0)
            g_unk0x00817420[i * 0xa] = index;
    }
    index = RallyDataStageIndex();
    g_unk0x00817410 = 0;
    if (index == 0)
        g_unk0x00817410 = index;
}

void RallyData_FUN_00408fc0(int index);

// Accumulates the pressed keys of every input device (and of the two global
// flags) into the per-key counters.
// FUNCTION: CMR2 0x004cf260
void FUN_004cf260(void)
{
    InputKeyState *pState;
    int i;
    int j;

    for (i = 0; i < 0xb; i++) {
        for (j = 0; j < g_unk0x008173f0; j++) {
            pState = (InputKeyState *)(g_unk0x00817420 + j * 0xa);
            if ((1 << i) & pState->field_0x4) {
                pState->field_0x8++;
                RallyData_FUN_00408fc0(j);
            }
        }
        if ((1 << i) & g_unk0x00817414) {
            g_unk0x00817418++;
            CGame::FUN_004057c0();
        }
    }
    for (i = 0; i < 8; i++) {
        for (j = 0; j < g_unk0x008173f0; j++) {
            pState = (InputKeyState *)(g_unk0x00817420 + j * 0xa);
            if ((1 << i) & pState->field_0x6) {
                pState->field_0x8++;
                RallyData_FUN_00408fc0(j);
            }
        }
        if ((1 << i) & g_unk0x00817416) {
            g_unk0x00817418++;
            CGame::FUN_004057c0();
        }
    }
    for (j = 0; j < g_unk0x008173f0; j++) {
        pState = (InputKeyState *)(g_unk0x00817420 + j * 0xa);
        if (pState->field_0x2 != 0) {
            pState->field_0x8++;
            RallyData_FUN_00408fc0(j);
        }
        if (pState->field_0x3 != 0) {
            pState->field_0x8++;
            RallyData_FUN_00408fc0(j);
        }
    }
    if (g_unk0x00817412 != 0) {
        g_unk0x00817418++;
        CGame::FUN_004057c0();
    }
    if (g_unk0x00817413 != 0) {
        g_unk0x00817418++;
        CGame::FUN_004057c0();
    }
}

// GLOBAL: CMR2 0x008173f8
int g_unk0x008173f8;

// Stores the country/stage selection of the current rally data record.
// FUNCTION: CMR2 0x004d0230
void FUN_004d0230(void)
{
    g_unk0x008173f8 = RallyDataCountryIndex() & 0xff;
    if ((BYTE)RallyData_GetFlag24() != 0) {
        g_unk0x008173fc = RallyData_FUN_00406950() & 0xff;
        g_unk0x00817404 = RallyData_FUN_00406940() & 0xff;
        return;
    }
    g_unk0x008173fc = RallyDataStageIndex() & 0xff;
}

// Indexed by id. A mix of small integers (car/stage index and 0/1 option
// flags) and pointers to the CFrontend car directory names, so it cannot be a
// uniform pointer array. Size derived from the 0x516c78 boundary (next known
// global), so it may cover further undeclared values.
struct Unk0x00516b40 {
    int ids[28];           // 0x00 car/stage index
    const char *dirs[22];  // 0x70 car directory name
    int flags[28];         // 0xc8 option flag
};
// GLOBAL: CMR2 0x00516b40
Unk0x00516b40 g_unk0x00516b40 = {
    { 0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 4, 5, 6, 6, 6, 7, 8, 9, 10, 11, 12, 13, 0, 4, 8, 9, 10, 11 },
    {
        CFrontend::m_strFoc, CFrontend::m_strF99Short, CFrontend::m_strFA1, CFrontend::m_strFA2,
        CFrontend::m_strMit, CFrontend::m_strMA1, CFrontend::m_strMA2, CFrontend::m_strMA3,
        CFrontend::m_strCor, CFrontend::m_strSubShort, CFrontend::m_str206, CFrontend::m_strSea,
        CFrontend::m_strInt, CFrontend::m_strIA1, CFrontend::m_strIA2, CFrontend::m_strSie,
        CFrontend::m_strMin, CFrontend::m_str6R4, CFrontend::m_strStr, CFrontend::m_str205,
        CFrontend::m_strPum, CFrontend::m_strEsc,
    },
    { 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
};

// FUNCTION: CMR2 0x0040ee60
void *CFrontend::FUN_0040ee60(int index)
{
    return (void *)g_unk0x00516b40.dirs[index];
}

// FUNCTION: CMR2 0x0040ee70
void *CFrontend::FUN_0040ee70(int index)
{
    return (void *)g_unk0x00516b40.flags[index];
}

// FUNCTION: CMR2 0x0040ee80
void *CFrontend::FUN_0040ee80(int index)
{
    return (void *)g_unk0x00516b40.flags[index + 14];
}

// FUNCTION: CMR2 0x0040ee90
void *CFrontend::FUN_0040ee90(int index)
{
    return (void *)g_unk0x00516b40.ids[index];
}

// FUNCTION: CMR2 0x0040eea0
void *CFrontend::FUN_0040eea0(int index)
{
    return (void *)g_unk0x00516b40.ids[index + 22];
}

struct Unk0x004a3e20 {
    BYTE field_0x0[0x118];
    int field_0x118;
};

// FUNCTION: CMR2 0x004a3e20
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value)
{
    if (pObject != NULL)
        pObject->field_0x118 = value;
}

// ---------------------------------------------------------------------------
// Frontend device setup: option words, per-player key state and split times.
// ---------------------------------------------------------------------------

// Split times mirrored out of the loaded game info record; 10 entries for the
// rally records, the arcade records only fill the first 6.
// GLOBAL: CMR2 0x00817448
int g_unk0x00817448[10];
// Lowest split value seen so far, reset to a whole stage length.
// GLOBAL: CMR2 0x00817570
int g_unk0x00817570;
// Placeholder string copied (and discarded) by FUN_004cfe80.
// GLOBAL: CMR2 0x00523bb4
char g_str0x00523bb4[8] = "ABCDEFG";

// Merges the frontend menu keys into the per-player key state: the first flag
// word of each state comes from the "keys" flag and the second from the
// "buttons" flag, each one shifted by the current stage index.
// FUNCTION: CMR2 0x004cf140
void FUN_004cf140(void)
{
    InputKeyState *pState;
    BYTE stage;
    int i;

    if (RallyData_GetFlag30()) {
        for (i = 0; i < g_unk0x008173f0; i++) {
            pState = (InputKeyState *)(g_unk0x00817420 + i * 0xa);
            pState->field_0x4 |= (short)(pState->field_0x0[0] << RallyDataStageIndex());
        }
        g_unk0x00817414 |= (short)(g_unk0x00817410 << RallyDataStageIndex());
    }
    if (RallyData_GetFlag24()) {
        for (i = 0; i < g_unk0x008173f0; i++) {
            pState = (InputKeyState *)(g_unk0x00817420 + i * 0xa);
            pState->field_0x4 |= (short)(FUN_00448cb0(i) << RallyData_FUN_00406950());
        }
        g_unk0x00817414 |= (short)(FUN_00448cc0() << RallyData_FUN_00406950());
    }
    stage = FUN_004071c0(RallyDataCountryIndex(), (char)CGameInfo::FUN_00405d90());
    if (RallyDataStageIndex() == stage) {
        for (i = 0; i < g_unk0x008173f0; i++) {
            pState = (InputKeyState *)(g_unk0x00817420 + i * 0xa);
            pState->field_0x6 |= (short)(pState->field_0x0[1] << RallyDataStageIndex());
        }
        g_unk0x00817416 |= (short)(g_unk0x00817411 << RallyDataStageIndex());
    }
}

// Clears the two key fields of a device option word.
// FUNCTION: CMR2 0x004cf390
void FUN_004cf390(int index)
{
    unsigned int *pValue = (unsigned int *)FUN_004d02d0(index);

    if (pValue != NULL)
        *pValue &= 0xffffc3ff;
}

// Advances the 4-bit option field of a device word; only for the first mode.
// FUNCTION: CMR2 0x004cf3b0
void FUN_004cf3b0(int index, int mode)
{
    unsigned int *pValue = (unsigned int *)FUN_004d02d0(index);

    if (pValue != NULL && mode == 0)
        *pValue = (((*pValue & 0xfffffc00) + 0x400) ^ *pValue) & 0x3c00 ^ *pValue;
}

// Resets the value and the three stat counters of a device record.
// match 50%: MSVC keeps the zero in a register (xor ecx / cmp eax,ecx) instead of an
// immediate store plus test; the code is the same.
// FUNCTION: CMR2 0x004cf3f0
void FUN_004cf3f0(int index)
{
    BYTE *pDevice = RallyData_FUN_00408c70(index);

    if (pDevice == NULL)
        return;
    *(int *)(pDevice + 4) = 0;
    pDevice += 8;
    *(short *)pDevice = 0;
    *(pDevice + 2) = 0;
}

// Adds to the value field of a device record and bumps one of its three stats.
// FUNCTION: CMR2 0x004cf420
void FUN_004cf420(int index, int amount, int stat)
{
    BYTE *pDevice = RallyData_FUN_00408c70(index);

    if (pDevice != NULL) {
        *(int *)(pDevice + 4) = *(int *)(pDevice + 4) + amount;
        if (stat < 3)
            pDevice[8 + stat]++;
    }
}

// Stores a value into the option block of a device (see FUN_004d0280).
// FUNCTION: CMR2 0x004cf450
void FUN_004cf450(int index, int arg, int value)
{
    int *pOption;

    FUN_004d0230();
    pOption = (int *)FUN_004d0280(index);
    if (pOption != NULL)
        *pOption = value;
}

// Writes a device option word: the option index in the low nibble, a 2-bit and
// a 4-bit field above it, and the value in the following dword.
// match 52%: MSVC schedules the *pValue load after the stores and allocates
// different registers for option/field; the code is the same.
// FUNCTION: CMR2 0x004cf470
void FUN_004cf470(int index, int value, unsigned int option, unsigned int field)
{
    unsigned int *pValue = (unsigned int *)FUN_004d02d0(index);
    unsigned int word;

    if (pValue != NULL) {
        word = (*pValue & 0xfffffc00) ^ (option & 0xf);
        pValue[1] = value;
        word = ((((option & 3) << 4) | (field & 0xf)) << 4) | word;
        *pValue = word;
        if (0x300 < (word & 0x300)) {
            word &= 0xffffc3ff;
            *pValue = word;
        }
    }
}

// Writes the two 6-bit fields of a device record and clears its second dword.
// match 83%: MSVC folds the two AND masks into 0xffffc000 where the original
// keeps 0xffffc03f then 0xffffffc0.
// FUNCTION: CMR2 0x004cf4d0
void FUN_004cf4d0(int index, unsigned int value, unsigned int field)
{
    unsigned int *pValue = (unsigned int *)RallyData_FUN_00408c70(index);
    unsigned int word;

    if (pValue != NULL) {
        word = ((field & 0xff) << 6) | (*pValue & 0xffffc03f);
        word &= 0xffffffc0;
        word ^= value & 0xf;
        word |= (value & 3) << 4;
        pValue[1] = 0;
        *pValue = word;
        if (0x30 < (word & 0x30)) {
            word &= 0xffffffcf;
            *pValue = word;
        }
    }
}

// Stores a value into the extra dword of a device record.
// FUNCTION: CMR2 0x004cf530
void FUN_004cf530(int index, int value)
{
    int *pField;

    FUN_004d0230();
    pField = (int *)(RallyData_FUN_00408c70(index) + 0x20);
    if (pField != NULL)
        *pField = value;
}

// Writes the 6-bit, 2-bit and 3-bit fields of a device record and its extra
// dword.
// match 87%: MSVC folds (*p & 0xfffff81f) & 0xffffffe0 and orders the pops
// differently; the code is the same.
// FUNCTION: CMR2 0x004cf550
void FUN_004cf550(int index, unsigned int value, unsigned int field, int extra)
{
    unsigned int *pValue = (unsigned int *)(RallyData_FUN_00408c70(index) + 0x18);
    unsigned int word;

    if (pValue != NULL) {
        pValue[1] = extra;
        word = ((field & 0x3f) << 5) | (*pValue & 0xfffff81f);
        word &= 0xffffffe0;
        word ^= value & 7;
        word |= (value & 3) << 3;
        *pValue = word;
        if (0x18 < (word & 0x18)) {
            word &= 0xffffffe7;
            *pValue = word;
        }
    }
}

// Rebuilds the option value of a device into the stage setup block when it is
// better than the stored one, and marks the player's key state as dirty.
// FUNCTION: CMR2 0x004cf5b0
BYTE FUN_004cf5b0(int index, int pBlock)
{
    unsigned int *pOption;
    unsigned int *pEntry;
    unsigned int value;

    pOption = (unsigned int *)FUN_004d0280(index);
    pEntry = (unsigned int *)(pBlock + 0x150 + (g_unk0x008173fc + g_unk0x008173f8 * 0xc) * 8);
    if (pOption != NULL && (*pOption < pEntry[1] || (*pEntry & 0x80) == 0)) {
        *pEntry |= 0x80;
        value = rand();
        *pEntry = (value & 0x1f) << 8 | (*pEntry & 0xffffe0ff);
        FUN_004d0300((int *)(pEntry + 1), (int *)pOption);
        *pEntry = (RallyData_FUN_004086b0((BYTE)index) & 0x3f) | (*pEntry & 0xffffffc0);
        *pEntry = (FUN_004086f0((BYTE)index) & 1) << 6 | (*pEntry & 0xffffffbf);
        RallyData_MarkTyresChanged(index);
        g_unk0x00817420[index * 0xa] = 1;
        return 1;
    }
    return 0;
}

// Same as FUN_004cf5b0 for the button option word (uses the record option
// block of FUN_004d02d0 and the three-field merge of FUN_004d0310).
// match 81%: MSVC keeps *pOption in a register instead of re-reading it and uses
// ebp for the entry word; the code is the same.
// FUNCTION: CMR2 0x004cf660
BYTE FUN_004cf660(int index, int pBlock)
{
    unsigned int *pOption;
    unsigned int *pEntry;
    unsigned int value;
    BOOL better;

    pOption = (unsigned int *)FUN_004d02d0(index);
    pEntry = (unsigned int *)(pBlock + (g_unk0x008173f8 * 3 + 4 + g_unk0x00817400) * 0xc);
    better = FALSE;
    if (pOption != NULL) {
        value = pEntry[1];
        if ((*pOption & 0xf) < (value & 0xf))
            better = TRUE;
        if ((((*pOption ^ value) & 0xf) == 0 && pOption[1] < pEntry[2]) || better ||
            (*pEntry & 0x80) == 0) {
            *pEntry |= 0x80;
            value = rand();
            *pEntry = (value & 0x1f) << 8 | (*pEntry & 0xffffe0ff);
            FUN_004d0310((int *)(pEntry + 1), (int *)pOption);
            *pEntry = (RallyData_FUN_004086b0((BYTE)index) & 0x3f) | (*pEntry & 0xffffffc0);
            *pEntry = (FUN_004086f0((BYTE)index) & 1) << 6 | (*pEntry & 0xffffffbf);
            RallyData_MarkTyresChanged(index);
            g_unk0x00817420[index * 0xa + 1] = 1;
            return 1;
        }
    }
    return 0;
}

// Same as FUN_004cf5b0 for the per-player word of the second dword, using the
// two 4-bit fields merge of FUN_004d0370.
// match 59%: register allocation of the device pointer and of the "better" flag
// differs (the original keeps more values on the stack).
// FUNCTION: CMR2 0x004cf740
BYTE FUN_004cf740(int index, int pBlock)
{
    unsigned int *pDevice;
    unsigned int *pEntry;
    unsigned int value;
    BOOL better;

    pDevice = (unsigned int *)RallyData_FUN_00408c70(index);
    better = FALSE;
    if (pDevice != NULL) {
        pEntry = (unsigned int *)(g_unk0x00817400 * 0x10 + pBlock);
        value = pEntry[1];
        if ((*pDevice & 0xf) < (value & 0xf))
            better = TRUE;
        if ((((*pDevice ^ value) & 0xf) == 0 && (*pDevice & 0x3fc0) > (pEntry[1] & 0x3fc0)) ||
            better || (*pEntry & 0x80) == 0) {
            *pEntry |= 0x80;
            value = rand();
            *pEntry = (value & 0x1f) << 8 | (*pEntry & 0xffffe0ff);
            FUN_004d0370((int *)(pEntry + 1), (int *)pDevice);
            *pEntry = (RallyData_FUN_004086b0((BYTE)index) & 0x3f) | (*pEntry & 0xffffffc0);
            *pEntry = (FUN_004086f0((BYTE)index) & 1) << 6 | (*pEntry & 0xffffffbf);
            RallyData_MarkTyresChanged(index);
            g_unk0x00817420[index * 0xa + 2] = 1;
            return 1;
        }
    }
    return 0;
}

// Rebuilds the arcade-record option value of a device from the category record
// (pointer at +0x20) into the arcade block.
// FUNCTION: CMR2 0x004cf830
BYTE FUN_004cf830(int index)
{
    int base;
    unsigned int *pDevice;
    unsigned int *pEntry;
    unsigned int value;

    base = (int)RallyData_FUN_00408cb0(index);
    pDevice = (unsigned int *)((int)RallyData_FUN_00408c70(index) + 0x20);
    pEntry = (unsigned int *)(base + 0x4bc + (g_unk0x008173fc + g_unk0x00817404 * 3) * 8);
    if (pDevice != NULL && (*pDevice < pEntry[1] || (*pEntry & 0x80) == 0)) {
        *pEntry |= 0x80;
        value = rand();
        *pEntry = (value & 0x1f) << 8 | (*pEntry & 0xffffe0ff);
        FUN_004d0300((int *)(pEntry + 1), (int *)pDevice);
        *pEntry = (RallyData_FUN_004086b0((BYTE)index) & 0x3f) | (*pEntry & 0xffffffc0);
        *pEntry = (FUN_004086f0((BYTE)index) & 1) << 6 | (*pEntry & 0xffffffbf);
        RallyData_MarkTyresChanged(index);
        return 1;
    }
    return 0;
}

// Same as FUN_004cf740 for the 3-bit/6-bit option word at +0x18, using the
// three 3-bit fields merge of FUN_004d03b0.
// FUNCTION: CMR2 0x004cf8e0
BYTE FUN_004cf8e0(int index, int pBlock)
{
    unsigned int *pValue;
    unsigned int *pEntry;
    unsigned int value;
    BOOL better;

    pValue = (unsigned int *)(RallyData_FUN_00408c70(index) + 0x18);
    pEntry = (unsigned int *)(pBlock + (g_unk0x00817404 * 3 + 0x5c + g_unk0x00817400) * 0xc);
    if (pValue != NULL) {
        value = pEntry[1];
        better = FALSE;
        if ((*pValue & 7) < (value & 7))
            better = TRUE;
        if ((((*pValue ^ value) & 7) == 0 && (*pValue & 0x7e0) > (pEntry[1] & 0x7e0)) || better ||
            (*pEntry & 0x80) == 0) {
            *pEntry |= 0x80;
            value = rand();
            *pEntry = (value & 0x1f) << 8 | (*pEntry & 0xffffe0ff);
            FUN_004d03b0((int *)(pEntry + 1), (int *)pValue);
            *pEntry = (RallyData_FUN_004086b0((BYTE)index) & 0x3f) | (*pEntry & 0xffffffc0);
            *pEntry = (FUN_004086f0((BYTE)index) & 1) << 6 | (*pEntry & 0xffffffbf);
            RallyData_MarkTyresChanged(index);
            g_unk0x00817420[index * 0xa + 3] = 1;
            return 1;
        }
    }
    return 0;
}

// Keeps the lowest option value of a device in the running minimum.
// FUNCTION: CMR2 0x004cf9d0
int FUN_004cf9d0(int param_1, int param_2)
{
    unsigned int *pOption;

    RallyData_FUN_00408c70(param_2);
    pOption = (unsigned int *)FUN_004d0280(param_2);
    if (pOption != NULL && *pOption < (unsigned int)g_unk0x00817570) {
        g_unk0x00817570 = *pOption;
        return 1;
    }
    return 0;
}

// Copies a stage record name and its split times into the mirror array; only
// when the option value beats the stored one.
// match 70%: register allocation and the stack frame differ (the original uses
// push ecx where we allocate two slots); the code is the same.
// FUNCTION: CMR2 0x004cfa10
BYTE FUN_004cfa10(int param_1, int param_2, char *pName)
{
    GameInfo0xa4 *pInfo;
    GameInfo0xa4SubStruct8 *pRecord;
    short *pSplits;
    unsigned int *pOption;
    int index;
    int i;

    RallyData_FUN_00408c70(param_2);
    pOption = (unsigned int *)FUN_004d0280(param_2);
    pInfo = CGameInfo::FUN_00405fe0();
    index = g_unk0x008173fc + g_unk0x008173f8 * 0xb;
    pRecord = &pInfo->rallyStageRecordTimes[index];
    pSplits = pInfo->rallyStageRecordSplits[index];
    if (pOption == NULL || ((pRecord->value >> 7) & 0xffff) <= *pOption)
        return 0;
    strcpy(pRecord->ident, pName);
    pRecord->value = (RallyData_FUN_004086b0((BYTE)param_2) & 0x3f) | (pRecord->value & 0xffffffc0);
    pRecord->value = (FUN_004086f0((BYTE)param_2) & 1) << 6 | (pRecord->value & 0xffffffbf);
    pRecord->value = (*pOption & 0xffff) << 7 | (pRecord->value & 0xff80007f);
    {
        int *pMirror = g_unk0x00817448;
        short *pSplit = pSplits;

        i = 0;
        do {
            unsigned short value = (unsigned short)FUN_00448680(param_1, i);
            *pSplit = value;
            *pMirror = value;
            pSplit++;
            pMirror++;
            i++;
        } while ((int)pMirror < (int)&g_unk0x00817448[10]);
    }
    g_unk0x00817410 = 1;
    return 1;
}

// Keeps the lowest arcade-record option value in the running minimum.
// FUNCTION: CMR2 0x004cfe20
int FUN_004cfe20(int param_1, int param_2)
{
    unsigned int *pDevice;
    unsigned int value;

    pDevice = (unsigned int *)((int)RallyData_FUN_00408c70(param_2) + 0x20);
    if (pDevice != NULL) {
        value = *pDevice;
        if (value < (unsigned int)g_unk0x00817570) {
            g_unk0x00817570 = value;
            return 1;
        }
    }
    return 0;
}

// Copies an arcade record name and its (6) split times into the mirror array.
// match 76%: register allocation and the stack frame differ; the code is the
// same.
// FUNCTION: CMR2 0x004cfe80
BYTE FUN_004cfe80(int param_1, int param_2)
{
    GameInfo0xa4 *pInfo;
    GameInfo0xa4SubStruct8 *pRecord;
    short *pSplits;
    unsigned int *pOption;
    char placeholder[16];
    int index;
    int i;

    pOption = (unsigned int *)RallyData_FUN_00408c70(param_2);
    pInfo = CGameInfo::FUN_00405fe0();
    index = g_unk0x008173fc + g_unk0x00817404 * 3;
    FUN_004582d0(param_1);
    strcpy(placeholder, g_str0x00523bb4);
    pRecord = &pInfo->arcadeRecordTimes[index];
    pSplits = pInfo->arcadeRecordSplits[index];
    if ((unsigned int *)((int)pOption + 0x20) != NULL) {
        unsigned int value = *(unsigned int *)((int)pOption + 0x20);

        if (value < ((pRecord->value >> 7) & 0xffff)) {
            pRecord->value = (value & 0xffff) << 7 | (pRecord->value & 0xff80007f);
            {
                int *pMirror = g_unk0x00817448;
                short *pSplit = pSplits;

                i = 0;
                do {
                    unsigned short split = (unsigned short)FUN_00448240(param_1, i);
                    *pSplit = split;
                    *pMirror = split;
                    pSplit++;
                    pMirror++;
                    i++;
                } while ((int)pMirror < (int)&g_unk0x00817448[6]);
            }
            value = (pRecord->value >> 7) & 0xffff;
            i = FUN_004583a0();
            g_unk0x00817448[i] = value;
            strcpy(pRecord->ident, (char *)RallyData_GetRecord((BYTE)param_2));
            pRecord->value = (RallyData_FUN_004086b0((BYTE)param_2) & 0x3f) | (pRecord->value & 0xffffffc0);
            pRecord->value = (FUN_004086f0((BYTE)param_2) & 1) << 6 | (pRecord->value & 0xffffffbf);
            return 1;
        }
    }
    return 0;
}

// Loads the current set of split times (rally or arcade) into the mirror array
// and resets the running minimum to a whole stage length.
// FUNCTION: CMR2 0x004d0180
void FUN_004d0180(void)
{
    GameInfo0xa4 *pInfo;
    int *pDest;
    int index;
    int i;

    pInfo = CGameInfo::FUN_00405fe0();
    if ((BYTE)RallyData_GetFlag23()) {
        FUN_004d0230();
        i = 0;
        pDest = g_unk0x00817448;
        do {
            if ((BYTE)RallyData_GetFlag24()) {
                index = g_unk0x008173fc + g_unk0x00817404 * 3;
                *pDest = ((unsigned short *)pInfo->arcadeRecordSplits[index])[i];
            } else {
                index = g_unk0x008173fc + g_unk0x008173f8 * 0xb;
                *pDest = ((unsigned short *)pInfo->rallyStageRecordSplits[index])[i];
            }
            pDest++;
            i++;
        } while ((int)pDest < (int)&g_unk0x00817448[10]);
        RallyData_GetFlag24();
        g_unk0x00817570 = 360000;
        return;
    }
    g_unk0x00817570 = 360000;
}

// Stores a value into the option block of the given device (see FUN_004d0280).
// FUNCTION: CMR2 0x004d0220
int FUN_004d0220(int index)
{
    return g_unk0x00817448[index];
}

// Returns the option block of a device when the current menu mode has one
// (five option words per device), or NULL.
// FUNCTION: CMR2 0x004d0280
BYTE *FUN_004d0280(int index)
{
    BYTE *pDevice = RallyData_FUN_00408c70(index);

    switch (g_unk0x008173f4) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 8:
    case 9:
    case 10:
        return pDevice + 0x14;
    }
    return NULL;
}

// Returns the setup word of a device in the single-player and the first
// multi-player modes.
// FUNCTION: CMR2 0x004d02d0
BYTE *FUN_004d02d0(int index)
{
    BYTE *pDevice = RallyData_FUN_00408c70(index);

    if (g_unk0x008173f4 >= 0 && (g_unk0x008173f4 <= 1 || g_unk0x008173f4 == 8))
        return pDevice + 0xc;
    return NULL;
}

// Copies one 32-bit option word.
// FUNCTION: CMR2 0x004d0300
void FUN_004d0300(int *pDest, int *pSource)
{
    *pDest = *pSource;
}

// Merges the four 4-bit fields of a source word into a destination word.
// FUNCTION: CMR2 0x004d0310
void FUN_004d0310(int *pDest, int *pSource)
{
    pDest[1] = pSource[1];
    *pDest = (((*pDest ^ *pSource) & 0xf) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0xf0) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x300) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x3c00) ^ *pDest);
}

// Merges the 4-bit, 2-bit and 6-bit fields of a source word into a
// destination word.
// FUNCTION: CMR2 0x004d0370
void FUN_004d0370(int *pDest, int *pSource)
{
    pDest[1] = pSource[1];
    *pDest = (((*pDest ^ *pSource) & 0xf) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x30) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x3fc0) ^ *pDest);
}

// Merges the 3-bit, 2-bit and 6-bit fields of a source word into a
// destination word.
// FUNCTION: CMR2 0x004d03b0
void FUN_004d03b0(int *pDest, int *pSource)
{
    pDest[1] = pSource[1];
    *pDest = (((*pDest ^ *pSource) & 7) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x18) ^ *pDest);
    *pDest = (((*pDest ^ *pSource) & 0x7e0) ^ *pDest);
}

// cross-range: 0x4d0770 belongs to the Game.cpp range but is only used by
// FUN_004d2070 here; it returns the callback machine state block.
// FUNCTION: CMR2 0x004d0770
Unk0049c2c0 *FUN_004d0770(void)
{
    return &CGame::m_unk0x00817da0;
}

// Enables/disables the debug overlay channels and pushes the new flags into
// the grouped callback machine.
// FUNCTION: CMR2 0x004d2070
void FUN_004d2070(BYTE param1, BYTE param2, BYTE param3)
{
    Unk0049c2c0 *p;

    p = (Unk0049c2c0 *)FUN_004d0770();
    CGame::m_unk0x00523d68 = param1;
    CGame::m_unk0x008180f9 = param2;
    g_unk0x008180fa = param3;
    if (param1 == 0 && param2 == 0 && param3 == 0)
        CGame::FUN_0049c1c0(p, 0, 7, 2);
    else
        CGame::FUN_0049c1c0(p, 0, 0, 2);
}
