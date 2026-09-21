#include "GameInfo.h"
#include "Graphics.h"
#include "Input.h"
#include "Frontend.h"
#include "InstallInfo.h"
#include "FileBuffer.h"
#include "GenericFileLoader.h"
#include "main.h"
#include "Game.h"
#include "Sound.h"
#include "RallyData.h"

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
LPVOID *CGameInfo::m_unk0x005a0098;
void *CGameInfo::m_unk0x005a00b8;
LPVOID *CGameInfo::m_unk0x005a009c;
void *CGameInfo::m_unk0x005a02c0;
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
unsigned char CGameInfo::FUN_00405d80(void)
{
    return m_gameInfo.field_0x14 >> 3 & 0x7f;
}

// FUNCTION: CMR2 0x00405d90
unsigned char CGameInfo::FUN_00405d90(void)
{
    return m_gameInfo.field_0x14 >> 10 & 7;
}

// FUNCTION: CMR2 0x00405d70
unsigned char CGameInfo::FUN_00405d70(void)
{
    return m_gameInfo.field_0x14 >> 13 & 0xf;
}

// FUNCTION: CMR2 0x00405da0
unsigned char CGameInfo::FUN_00405da0(void)
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
void CGameInfo::FUN_004f4b40(void)
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
void CGameInfo::FUN_00405de0(BYTE param1)
{
    m_gameInfo.field_0x14 = ((param1 & 1) << 0x13) | (m_gameInfo.field_0x14 & 0xfff7ffffU);
}

// FUNCTION: CMR2 0x00405dc0
unsigned char CGameInfo::FUN_00405dc0(void)
{
    return m_gameInfo.field_0x14 >> 18 & 1;
}

// FUNCTION: CMR2 0x00405dd0
unsigned char CGameInfo::FUN_00405dd0(void)
{
    return m_gameInfo.field_0x14 >> 20 & 7;
}

// FUNCTION: CMR2 0x00405e00
unsigned char CGameInfo::FUN_00405e00(void)
{
    return m_gameInfo.field_0x14 >> 19 & 1;
}

// FUNCTION: CMR2 0x00405e10
void CGameInfo::FUN_00405e10(unsigned int param1)
{
    m_gameInfo.field_0x18 = m_gameInfo.field_0x18 ^ (m_gameInfo.field_0x18 ^ param1) & 0x7f;
    CSound::FUN_004a31f0(m_gameInfo.field_0x18 & 0x7f);
}

// FUNCTION: CMR2 0x00405e40
unsigned int CGameInfo::FUN_00405e40(void)
{
    return m_gameInfo.field_0x18 & 0x7f;
}

// FUNCTION: CMR2 0x00405e50
void CGameInfo::FUN_00405e50(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 0x7f) << 7) | (m_gameInfo.field_0x18 & 0xffffc07fU);
}

// FUNCTION: CMR2 0x00405e70
unsigned int CGameInfo::FUN_00405e70(void)
{
    return m_gameInfo.field_0x18 >> 7 & 0x7f;
}

// FUNCTION: CMR2 0x00405e80
void CGameInfo::FUN_00405e80(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 0x7f) << 14) | (m_gameInfo.field_0x18 & 0xffe03fffU);
}

// FUNCTION: CMR2 0x00405ea0
unsigned int CGameInfo::FUN_00405ea0(void)
{
    return m_gameInfo.field_0x18 >> 14 & 0x7f;
}

// FUNCTION: CMR2 0x00405eb0
unsigned int CGameInfo::FUN_00405eb0(void)
{
    return m_gameInfo.field_0x18 >> 31;
}

// FUNCTION: CMR2 0x00405ec0
void CGameInfo::FUN_00405ec0(BYTE param1)
{
    m_gameInfo.field_0x18 = (param1 << 31) | (m_gameInfo.field_0x18 & 0x7fffffffU);
}

// FUNCTION: CMR2 0x00405ef0
unsigned int CGameInfo::FUN_00405ef0(void)
{
    return m_gameInfo.field_0x1c >> 1 & 3;
}

// FUNCTION: CMR2 0x00405f00
void CGameInfo::FUN_00405f00(unsigned int param1)
{
    m_gameInfo.field_0x18 = ((param1 & 3) << 24) | (m_gameInfo.field_0x18 & 0xfcffffffU);
}

// FUNCTION: CMR2 0x00405f20
void CGameInfo::FUN_00405f20(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 27) | (m_gameInfo.field_0x18 & 0xf7ffffffU);
}

// FUNCTION: CMR2 0x00405f40
void CGameInfo::FUN_00405f40(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 26) | (m_gameInfo.field_0x18 & 0xfbffffffU);
}

// FUNCTION: CMR2 0x00405f60
void CGameInfo::FUN_00405f60(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 28) | (m_gameInfo.field_0x18 & 0xefffffffU);
}

// FUNCTION: CMR2 0x00405f80
void CGameInfo::FUN_00405f80(BYTE param1)
{
    m_gameInfo.field_0x18 = ((param1 & 1) << 29) | (m_gameInfo.field_0x18 & 0xdfffffffU);
}

// FUNCTION: CMR2 0x00405fa0
void CGameInfo::FUN_00405fa0(DWORD *param1, WORD param2, DWORD param3)
{
    m_gameInfo.field_0x88 = param1[1];
    m_gameInfo.field_0x8c = param1[2];
    m_gameInfo.field_0x94 = param2;
    m_gameInfo.field_0x90 = param3;
}

// FUNCTION: CMR2 0x00405fd0
void CGameInfo::FUN_00405fd0(unsigned int param1)
{
    *(unsigned int *)&m_gameInfo.field_0x98 = param1;
}

// FUNCTION: CMR2 0x00405db0
unsigned int *CGameInfo::FUN_00405db0(void)
{
    return &m_gameInfo.field_0x9c;
}

// FUNCTION: CMR2 0x00405fe0
GameInfo0xa4 *CGameInfo::FUN_00405fe0(void)
{
    return &m_gameInfo.field_0xa4;
}

// FUNCTION: CMR2 0x00405ff0
GameInfo0xa4 *CGameInfo::FUN_00405ff0(int param1)
{
    return param1 != 0 ? &m_gameInfo.field_0x1368 : &m_gameInfo.field_0x262c;
}

// FUNCTION: CMR2 0x00405b30
void CGameInfo::SetFullscreen(BYTE fullscreen)
{
    m_gameInfo.unknownGraphicsOptions = (fullscreen & 1) | (m_gameInfo.unknownGraphicsOptions & 0xfffffffeU);
}

// FUNCTION: CMR2 0x00405b50
unsigned int CGameInfo::FUN_00405b50(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 1 & 3;
}

// FUNCTION: CMR2 0x00405b60
void CGameInfo::FUN_00405b60(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 1) | (m_gameInfo.unknownGraphicsOptions & 0xfffffff9U);
}

// FUNCTION: CMR2 0x00405b80
void CGameInfo::FUN_00405b80(BYTE param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 1) << 3) | (m_gameInfo.unknownGraphicsOptions & 0xfffffff7U);
}

// FUNCTION: CMR2 0x00405ba0
unsigned int CGameInfo::FUN_00405ba0(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 4 & 1;
}

// FUNCTION: CMR2 0x00405bb0
void CGameInfo::FUN_00405bb0(BYTE param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 1) << 4) | (m_gameInfo.unknownGraphicsOptions & 0xffffffefU);
}

// FUNCTION: CMR2 0x00405be0
void CGameInfo::FUN_00405be0(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 0xf) << 5) | (m_gameInfo.unknownGraphicsOptions & 0xfffffe1fU);
}

// FUNCTION: CMR2 0x00405c70
unsigned int CGameInfo::FUN_00405c70(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 18 & 3;
}

// FUNCTION: CMR2 0x00405c80
void CGameInfo::FUN_00405c80(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 18) | (m_gameInfo.unknownGraphicsOptions & 0xfff3ffffU);
}

// FUNCTION: CMR2 0x00405cb0
void CGameInfo::FUN_00405cb0(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 0xf) << 21) | (m_gameInfo.unknownGraphicsOptions & 0xfe1fffffU);
}

// FUNCTION: CMR2 0x00405cd0
unsigned int CGameInfo::FUN_00405cd0(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 25 & 3;
}

// FUNCTION: CMR2 0x00405ce0
void CGameInfo::FUN_00405ce0(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 25) | (m_gameInfo.unknownGraphicsOptions & 0xf9ffffffU);
}

// FUNCTION: CMR2 0x00405d00
unsigned int CGameInfo::FUN_00405d00(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 27 & 3;
}

// FUNCTION: CMR2 0x00405d20
void CGameInfo::FUN_00405d20(unsigned int param1)
{
    m_gameInfo.unknownGraphicsOptions = ((param1 & 3) << 27) | (m_gameInfo.unknownGraphicsOptions & 0xe7ffffffU);
}

// FUNCTION: CMR2 0x00405d60
unsigned int CGameInfo::FUN_00405d60(void)
{
    return m_gameInfo.unknownGraphicsOptions >> 29 & 1;
}

// FUNCTION: CMR2 0x00405d10
unsigned int CGameInfo::FUN_00405d10(void)
{
    return m_gameInfo.field_0x34 & 3;
}

// FUNCTION: CMR2 0x00405d40
void CGameInfo::FUN_00405d40(unsigned int param1)
{
    m_gameInfo.field_0x34 = m_gameInfo.field_0x34 ^ (m_gameInfo.field_0x34 ^ param1) & 3;
}

// FUNCTION: CMR2 0x00406310
unsigned int CGameInfo::FUN_00406310(void)
{
    return m_gameInfo.field_0x18 >> 30 & 1;
}

// FUNCTION: CMR2 0x00406320
BYTE CGameInfo::FUN_00406320(void)
{
    return m_unk0x0052ea52;
}

// FUNCTION: CMR2 0x00406330
void CGameInfo::FUN_00406330(BYTE param1)
{
    m_unk0x0052ea52 = param1;
}

// FUNCTION: CMR2 0x00406340
void CGameInfo::FUN_00406340(BYTE param1)
{
    m_gameInfo.field_0x14 = ((param1 & 1) << 18) | (m_gameInfo.field_0x14 & 0xfffbffffU);
}

// FUNCTION: CMR2 0x00406360
unsigned int CGameInfo::FUN_00406360(int param1)
{
    return (BYTE)(1 << param1) & m_gameInfo.field_0x20;
}

// FUNCTION: CMR2 0x00406380
void CGameInfo::FUN_00406380(int param1, int param2)
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
bool CGameInfo::FUN_004063d0(int param1)
{
    return ((BYTE)(m_gameInfo.field_0x20 >> 8) & (BYTE)(1 << param1)) != 0;
}

// FUNCTION: CMR2 0x004063f0
bool CGameInfo::FUN_004063f0(int param1)
{
    return ((BYTE)(m_gameInfo.field_0x20 >> 16) & (BYTE)(1 << param1)) != 0;
}

// FUNCTION: CMR2 0x00406430
unsigned int CGameInfo::FUN_00406430(void)
{
    return ((BYTE *)&m_gameInfo.field_0x20)[2];
}

// FUNCTION: CMR2 0x00406440
unsigned int CGameInfo::FUN_00406440(void)
{
    return m_gameInfo.field_0x1c >> 11 & 1;
}

// FUNCTION: CMR2 0x00406450
unsigned int CGameInfo::FUN_00406450(unsigned int **param1)
{
    if (param1 != NULL)
        *param1 = &m_gameInfo.field_0x38f4;
    return m_gameInfo.field_0x38f4;
}

// FUNCTION: CMR2 0x00406470
void CGameInfo::FUN_00406470(void)
{
    m_unk0x0052af94 = FUN_00405d80();
    m_unk0x0052ea44 = FUN_00405d70();
    m_unk0x0052af9c = FUN_00405d90();
    m_unk0x0052af80 = RallyData_FUN_004069a0();
    m_unk0x0052af84 = (BYTE)RallyData_FUN_00406940();
    m_unk0x0052af8c = (BYTE)RallyData_FUN_00406950();
    m_unk0x0052e93c = (BYTE)RallyData_FUN_00406990();
    m_unk0x0052ea48 = (BYTE)RallyDataStageIndex();
    m_unk0x0052e940 = (BYTE)RallyDataCountryIndex();
    m_unk0x0052af98 = m_gameInfo.field_0x14 >> 23 & 0xf;
    m_unk0x0052af88 = m_gameInfo.field_0x14 >> 27 & 0xf;
}

// FUNCTION: CMR2 0x00406520
BYTE CGameInfo::FUN_00406520(int param1, int param2)
{
    return m_gameInfo.field_0x38f8[param1 * 11 + param2];
}

// FUNCTION: CMR2 0x00406540
void CGameInfo::FUN_00406540(int param1, int param2, BYTE param3)
{
    m_gameInfo.field_0x38f8[param1 * 11 + param2] = param3;
}

// FUNCTION: CMR2 0x00406690
char *CGameInfo::FUN_00406690(void)
{
    return m_gameInfo.field_0x3950;
}

// FUNCTION: CMR2 0x004066a0
void CGameInfo::FUN_004066a0(char *name)
{
    strcpy(m_gameInfo.field_0x3950, name);
}

// FUNCTION: CMR2 0x004066d0
char *CGameInfo::FUN_004066d0(void)
{
    return m_gameInfo.field_0x3965;
}

// FUNCTION: CMR2 0x004066e0
void CGameInfo::FUN_004066e0(char *name)
{
    strcpy(m_gameInfo.field_0x3965, name);
}

// FUNCTION: CMR2 0x00510410
void CGameInfo::FUN_00510410(void)
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

    FUN_00510570();

    m_gameInfo.field_0x98 = 4;
    m_gameInfo.field_0x9a = 0;
    m_gameInfo.field_0x18 = (m_gameInfo.field_0x18 & 0xfff93264) | 0x40393264;

    FUN_00406010(&m_gameInfo.field_0xa4);
    FUN_00406010(&m_gameInfo.field_0x1368);
    FUN_00406010(&m_gameInfo.field_0x262c);

    m_gameInfo.field_0x14 = m_gameInfo.field_0x14 & 0x7fffffff;
}

// FUNCTION: CMR2 0x00510570
void CGameInfo::FUN_00510570(void)
{
    m_gameInfo.field_0x88 = 0x18000;
    m_gameInfo.field_0x8c = 0x68000;
    m_gameInfo.field_0x94 = 0x2d;
    m_gameInfo.field_0x90 = 0xe0000;
}

// FUNCTION: CMR2 0x00406010
void CGameInfo::FUN_00406010(GameInfo0xa4 *param1)
{
    char cVar1;
    int *puVar2;
    int iVar3;
    int uVar4;
    int uVar5;
    int *puVar6;
    GameInfo0xa4 *pGVar7;
    GameInfo0xa4 *puVar7;
    int *puVar8;
    int loop2;
    int iVar9;
    int loop1;
    int someLoopLimit;
    short (*pasVar10)[10];
    short *psVar11;
    int *puVar12;
    GameInfo0xa4SubStruct12 *pGVar13;
    GameInfo0xa4SubStruct8 *pGVar14;
    short (*local_10)[10];
    unsigned int *local_c;
    int local_8;
    int local_4;
    GameInfo0xa4 *pGameInfo0xa4;
    int bitMask;
    int *bitMask2;
    char *cmrString;
    char *cmrString2;

    pGameInfo0xa4 = param1;
    bitMask = (int)&param1->firstLoop[0].flags;
    loop1 = 3;
    do
    {
        loop2 = 5;
        bitMask2 = (int *)bitMask;
        do
        {
            uVar4 = 0xffffffff;
            cmrString = CGameInfo::m_stringCMR;
            do
            {
                cmrString2 = cmrString;
                if (uVar4 == 0)
                    break;
                uVar4 = uVar4 - 1;
                cmrString2 = cmrString + 1;
                cVar1 = *cmrString;
                cmrString = cmrString2;
            } while (cVar1 != '\0');
            uVar4 = ~uVar4;
            puVar6 = (int *)(cmrString2 + -uVar4);
            puVar12 = bitMask2 + -1;
            for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1)
            {
                *puVar12 = *puVar6;
                puVar6 = puVar6 + 1;
                puVar12 = puVar12 + 1;
            }
            for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1)
            {
                *(char *)puVar12 = (char)*puVar6;
                puVar6 = (int *)((int)puVar6 + 1);
                puVar12 = (int *)((int)puVar12 + 1);
            }
            *bitMask2 = *bitMask2 & 0xffffc000 | 0x3c000;
            bitMask = (unsigned int)(bitMask2 + 3);
            loop2 = loop2 + -1;
            *(unsigned char *)(bitMask2 + 1) = 0;
            *(unsigned char *)((int)bitMask2 + 6) = 0;
            bitMask2 = (int *)bitMask;
        } while (loop2 != 0);
        loop1 = loop1 + -1;
    } while (loop1 != 0);
    someLoopLimit = 0xcb;
    local_10 = param1->rallyStageRecordSplits + 10;
    puVar2 = (int *)&param1->secondLoop[0].flags;
    local_c = &param1->rallyStageRecordTimes[10].value;
    do
    {
        local_8 = 3;
        do
        {
            iVar9 = 5;
            puVar8 = puVar2;
            do
            {
                uVar4 = 0xffffffff;
                cmrString = CGameInfo::m_stringCMR;
                do
                {
                    cmrString2 = cmrString;
                    if (uVar4 == 0)
                        break;
                    uVar4 = uVar4 - 1;
                    cmrString2 = cmrString + 1;
                    cVar1 = *cmrString;
                    cmrString = cmrString2;
                } while (cVar1 != '\0');
                uVar4 = ~uVar4;
                puVar2 = puVar8 + 3;
                puVar6 = (int *)(cmrString2 + -uVar4);
                pGVar13 = (GameInfo0xa4SubStruct12 *)(puVar8 + -1);
                for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1)
                {
                    *(unsigned int *)pGVar13->ident = *puVar6;
                    puVar6 = puVar6 + 1;
                    pGVar13 = (GameInfo0xa4SubStruct12 *)&pGVar13->flags;
                }
                for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1)
                {
                    pGVar13->ident[0] = (char)*puVar6;
                    puVar6 = (int *)((int)puVar6 + 1);
                    pGVar13 = (GameInfo0xa4SubStruct12 *)(pGVar13->ident + 1);
                }
                *puVar8 = *puVar8 & 0xffffff80;
                puVar8[1] = 360000;
                iVar9 = iVar9 + -1;
                *puVar8 = *puVar8 & 0xffff87ff | 0x780;
                puVar8 = puVar2;
            } while (iVar9 != 0);
            local_8 = local_8 + -1;
        } while (local_8 != 0);
        iVar9 = 0;
        do
        {
            uVar4 = 0xffffffff;
            cmrString = CGameInfo::m_stringCMR;
            do
            {
                cmrString2 = cmrString;
                if (uVar4 == 0)
                    break;
                uVar4 = uVar4 - 1;
                cmrString2 = cmrString + 1;
                cVar1 = *cmrString;
                cmrString = cmrString2;
            } while (cVar1 != '\0');
            uVar4 = ~uVar4;
            cmrString = cmrString2 + -uVar4;
            puVar8 = (int *)(param1->firstLoop[0].ident + (iVar9 + someLoopLimit) * 8 + -4);
            for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1)
            {
                *puVar8 = *(char *)cmrString;
                cmrString = cmrString + 4;
                puVar8 = puVar8 + 1;
            }
            for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1)
            {
                *(char *)puVar8 = *cmrString;
                cmrString = cmrString + 1;
                puVar8 = (int *)((int)puVar8 + 1);
            }
            uVar4 = *(unsigned int *)(param1->firstLoop[0].ident + (iVar9 + someLoopLimit) * 8);
            puVar6 = (int *)(param1->firstLoop[0].ident + (iVar9 + someLoopLimit) * 8);
            *puVar6 = uVar4 & 0xffffff80;
            if (iVar9 == 10)
            {
                uVar4 = uVar4 & 0xffaee000 | 0x2ee000;
                puVar6 = (int *)local_c;
            }
            else
            {
                uVar4 = uVar4 & 0xffddc000 | 0x5dc000;
            }
            iVar9 = iVar9 + 1;
            *puVar6 = uVar4;
        } while (iVar9 < 0xb);
        iVar9 = 0;
        do
        {
            if (iVar9 == 10)
            {
                iVar3 = 0;
                pasVar10 = local_10;
                do
                {
                    (*pasVar10)[0] = (short)iVar3 * 12000;
                    iVar3 = iVar3 + 1;
                    pasVar10 = (short (*)[10])(*pasVar10 + 1);
                } while (iVar3 < 10);
            }
            else
            {
                iVar3 = 0;
                psVar11 = (short *)((int)param1 + (iVar9 + someLoopLimit) * 0x14 + -0x6c8);
                do
                {
                    *psVar11 = (short)iVar3 * 6000;
                    iVar3 = iVar3 + 1;
                    psVar11 = psVar11 + 1;
                } while (iVar3 < 10);
            }
            iVar9 = iVar9 + 1;
        } while (iVar9 < 0xb);
        someLoopLimit = someLoopLimit + 0xb;
        local_10 = local_10 + 0xb;
        local_c = local_c + 0x16;
    } while (someLoopLimit < 0x123);
    local_4 = 3;
    puVar2 = (int *)&param1->thirdLoop[0].flags;
    do
    {
        param1 = (GameInfo0xa4 *)0x3;
        do
        {
            iVar9 = 5;
            puVar8 = puVar2;
            do
            {
                uVar4 = 0xffffffff;
                cmrString = CGameInfo::m_stringCMR;
                do
                {
                    cmrString2 = cmrString;
                    if (uVar4 == 0)
                        break;
                    uVar4 = uVar4 - 1;
                    cmrString2 = cmrString + 1;
                    cVar1 = *cmrString;
                    cmrString = cmrString2;
                } while (cVar1 != '\0');
                uVar4 = ~uVar4;
                puVar2 = puVar8 + 3;
                puVar6 = (int *)(cmrString2 + -uVar4);
                pGVar13 = (GameInfo0xa4SubStruct12 *)(puVar8 + -1);
                for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1)
                {
                    *(unsigned int *)pGVar13->ident = *puVar6;
                    puVar6 = puVar6 + 1;
                    pGVar13 = (GameInfo0xa4SubStruct12 *)&pGVar13->flags;
                }
                for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1)
                {
                    pGVar13->ident[0] = (char)*puVar6;
                    puVar6 = (int *)((int)puVar6 + 1);
                    pGVar13 = (GameInfo0xa4SubStruct12 *)(pGVar13->ident + 1);
                }
                iVar9 = iVar9 + -1;
                *puVar8 = *puVar8 & 0xffff0280 | 0x280;
                puVar8 = puVar2;
            } while (iVar9 != 0);
            param1 = (GameInfo0xa4 *)((int)param1[-1].arcadeRecordSplits[8] + 0xb);
        } while (param1 != NULL);
        local_4 = local_4 + -1;
    } while (local_4 != 0);
    puVar6 = (int *)&pGameInfo0xa4->arcadeRecordTimes[0].value;
    puVar7 = (GameInfo0xa4 *)pGameInfo0xa4->arcadeRecordSplits;
    param1 = (GameInfo0xa4 *)0x3;
    do
    {
        iVar9 = 3;
        do
        {
            uVar4 = 0xffffffff;
            cmrString = CGameInfo::m_stringCMR;
            do
            {
                cmrString2 = cmrString;
                if (uVar4 == 0)
                    break;
                uVar4 = uVar4 - 1;
                cmrString2 = cmrString + 1;
                cVar1 = *cmrString;
                cmrString = cmrString2;
            } while (cVar1 != '\0');
            uVar4 = ~uVar4;
            puVar12 = (int *)(cmrString2 + -uVar4);
            pGVar14 = (GameInfo0xa4SubStruct8 *)(puVar6 + -1);
            for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1)
            {
                *(unsigned int *)pGVar14->ident = *puVar12;
                puVar12 = puVar12 + 1;
                pGVar14 = (GameInfo0xa4SubStruct8 *)&pGVar14->value;
            }
            for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1)
            {
                pGVar14->ident[0] = (char)*puVar12;
                puVar12 = (int *)((int)puVar12 + 1);
                pGVar14 = (GameInfo0xa4SubStruct8 *)(pGVar14->ident + 1);
            }
            iVar3 = 0;
            *puVar6 = *puVar6 & 0xffaee000 | 0x2ee000;
            pGVar7 = puVar7;
            do
            {
                puVar7 = (GameInfo0xa4 *)(pGVar7->firstLoop[0].ident + 2);
                *(short *)pGVar7->firstLoop[0].ident = (short)iVar3 * 6000;
                iVar3 = iVar3 + 1;
                pGVar7 = puVar7;
            } while (iVar3 < 6);
            puVar6 = puVar6 + 2;
            iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        param1 = (GameInfo0xa4 *)((int)param1[-1].arcadeRecordSplits[8] + 0xb);
        if (param1 == NULL)
        {
            return;
        }
    } while (true);
}

// FUNCTION: CMR2 0x00406560
void CGameInfo::FUN_00406560(void)
{
    memset(m_gameInfo.field_0x38f8, 0x03, sizeof(m_gameInfo.field_0x38f8));
}

// FUNCTION: CMR2 0x00406580
void CGameInfo::FUN_00406580(void) {
    char bVar1;
    int uVar5 = 0;
    int uVar2 = 0;
    char *piVar4 = m_gameInfo.field_0x38f8;
    char *end = m_gameInfo.field_0x38f8 + sizeof(m_gameInfo.field_0x38f8);
    int iVar3 = 0;
    bool bVar6 = false;

    do {
        if ((piVar4 < (m_gameInfo.field_0x38f8 + 44)) && (*piVar4 & 2) != 0) {
            *piVar4 = *piVar4 & 0xfd;
        }

        if (uVar5 < (m_gameInfo.field_0x9c >> 8 & 0xf)) {
            iVar3 = 0;
            do {
                if ((piVar4[iVar3] & 2) != 0) {
                    piVar4[iVar3] = piVar4[iVar3] & 0xfd;
                }
                iVar3 ++;
            } while (iVar3 < 4);

            if (uVar5 < (m_gameInfo.field_0x9c >> 0xc & 0xf)) {
                iVar3 = 4;
                do {
                    if ((piVar4[iVar3] & 2) != 0) {
                        piVar4[iVar3] = piVar4[iVar3] & 0xfd;
                    }
                    iVar3 ++;
                } while (iVar3 < 8);
            
                if ((uVar5 < (m_gameInfo.field_0x9c >> 0x10 & 0xf)) && (m_gameInfo.field_0x9c & 1) != 0) {
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
            uVar2 = uVar2 - 1 | 0xfffffffe;
        }
        
        uVar2++; // idk but its in the asm
        if (uVar2) {
            uVar2 = 1 << (((uVar5 + 1) / 2 - 1) & 0x1f);

            if (((((uVar2 & m_gameInfo.field_0xa0 & 0x1f) != 0) ||
                (uVar2 & m_gameInfo.field_0xa0 >> 5 & 0x1f) != 0) ||
                ((uVar2 & m_gameInfo.field_0xa0 >> 10 & 0x1f) != 0)) && ((piVar4[10] & 2) != 0)
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
DWORD CGameInfo::SetupInputs(void) {
    if (CInput::DInputCreate()) {
        CInput::m_unk0x0059f8cc.field_0x0 = 0;
        CInput::m_unk0x0059f8cc.field_0x1 = 2;
        CInput::m_unk0x0059f8cc.field_0x2 = 0;
        
        int* pField18 = &CInput::m_availableDevices[0].field_0x18;
        int initVal = -1;
        
        do {
            *(pField18 - 6) = initVal;  // -24 bytes = field_0x0
            *pField18 = initVal;         // field_0x18
            pField18 = (int*)((BYTE*)pField18 + 0x50C);
        } while (pField18 < &CInput::m_availableDevices[8].field_0x18); // TODO: is this correct?
        
        CInput::SetupKeyboard();
        CInput::SetupMouse();
        CInput::GetAttachedJoysticks();
    }

    return CInput::m_unk0x0059f8cc.field_0x0 & 0xFF;
}

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

    switch (m_gameInfo.unknownGraphicsOptions & 6) {
        case 2:
            g_pGraphics->field913_0x3bc |= 8;
            g_pGraphics->field913_0x3bc &= 0xffffffef;
            g_pGraphics->field913_0x3bc &= 0xffffff7f;
        break;

        case 4:
            g_pGraphics->field913_0x3bc |= 8;
            g_pGraphics->field913_0x3bc |= 0x10;
            g_pGraphics->field913_0x3bc &= 0xffffff7f;
        break;
        
        case 6:
            g_pGraphics->field913_0x3bc |= 8;
            g_pGraphics->field913_0x3bc &= 0xffffffef;
            g_pGraphics->field913_0x3bc |= 0x80;
        break;

        default:
            g_pGraphics->field913_0x3bc &= 0xfffffff7;
            g_pGraphics->field913_0x3bc &= 0xffffffef;
        break;        
    }

    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions & 0x8) << 2 | g_pGraphics->field913_0x3bc & 0xffffffdf;
    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions & 0x10) << 2 | g_pGraphics->field913_0x3bc & 0xffffffbf;

    switch (m_gameInfo.unknownGraphicsOptions & 0xc0000) {
        default:
            g_pGraphics->field913_0x3bc &= 0xfffffffe;
            g_pGraphics->field913_0x3bc &= 0xfffffffd;
        break;
        
        case 0x40000:
            g_pGraphics->field913_0x3bc |= 1;
            g_pGraphics->field913_0x3bc &= 0xfffffffd;
        break;

        case 0x80000:
            g_pGraphics->field913_0x3bc &= 0xfffffffe;
            g_pGraphics->field913_0x3bc |= 2;
        break;

    }

    g_pGraphics->field917_0x3c0 = FUN_00405ca0();
    g_pGraphics->field913_0x3bc = (m_gameInfo.unknownGraphicsOptions >> 0x1b & 4) | g_pGraphics->field913_0x3bc & 0xfffffffb;

    if (FUN_00406410(0x11) != 0)
        FUN_004eac50(0x11);

    FUN_004d0590(0);
    CFileBuffer::FreeGenericFileBuffer(fileBuffer);

    return true;
}

// FUNCTION: CMR2 0x00405b20
unsigned int CGameInfo::IsFullscreen(void) {
    return m_gameInfo.unknownGraphicsOptions & 1;
}

// FUNCTION: CMR2 0x00405ca0
int CGameInfo::FUN_00405ca0(void)
{
  return (int)(m_gameInfo.unknownGraphicsOptions >> 0x15 & 0xf);
}

// FUNCTION: CMR2 0x00406410
bool CGameInfo::FUN_00406410(int param1) {
    bool response = m_gameInfo.field_0x38f0 & (1 << param1);
    return response;
}

// FUNCTION: CMR2 0x004eac50
bool CGameInfo::FUN_004eac50(int param1) {
    bool response = FUN_00406410(param1);
    m_gameInfo.field_0x38f0 = m_gameInfo.field_0x38f0 ^ 1 << param1;
    return !response;
}

// FUNCTION: CMR2 0x004d0590
void CGameInfo::FUN_004d0590(BYTE param1) {
    m_unk0x00817574 = param1;
}

// FUNCTION: CMR2 0x0049ea90
void CGameInfo::FUN_0049ea90(unsigned int param1) {
    FUN_0049e930(param1);
    m_unk0x0059f8d0 = param1;
}

// FUNCTION: CMR2 0x0049e930
void CGameInfo::FUN_0049e930(unsigned int param1) {
    m_unk0x00520870 = param1;
}

// FUNCTION: CMR2 0x004d05d0
void CGameInfo::FUN_004d05d0(void) {
    m_unk0x0081777c = 0;
    m_unk0x00817678 = FALSE;
    CGame::RegisterCallback(FUN_004d05a0, NULL);
}

// FUNCTION: CMR2 0x004d05a0
bool CGameInfo::FUN_004d05a0(void) {
    if (m_unk0x0081777c != NULL) {
        CFileBuffer::FreeGenericFileBuffer(m_unk0x0081777c);
        m_unk0x0081777c = NULL;
    }

    m_unk0x00817678 = FALSE;
    return true;
}

// FUNCTION: CMR2 0x004a0c60
void CGameInfo::FUN_004a0c60(void) {
    memset(m_unk0x0059fa20, 0, sizeof(m_unk0x0059fa20));

    m_unk0x005a0098 = &m_unk0x005a00b8;
    m_unk0x005a009c = &m_unk0x005a02c0;

    m_unk0x005a0060 = FALSE;
    m_unk0x005a1814 = FALSE;
    m_unk0x005a01bc = false;
}

// FUNCTION: CMR2 0x00405bd0
unsigned int CGameInfo::FUN_00405bd0(void) {
  return m_gameInfo.unknownGraphicsOptions >> 5 & 0xf;
}

// FUNCTION: CMR2 0x00405c00
unsigned int CGameInfo::FUN_00405c00(void) {
  return m_gameInfo.unknownGraphicsOptions >> 9 & 0xf;
}

// GLOBAL: CMR2 0x0082af88
int g_unk0x0082af88;
// GLOBAL: CMR2 0x0082b0a8
int g_unk0x0082b0a8;
// GLOBAL: CMR2 0x0082ac58
int g_unk0x0082ac58;
// GLOBAL: CMR2 0x0082ac5c
int g_unk0x0082ac5c;
// GLOBAL: CMR2 0x0082b0a0
int g_unk0x0082b0a0;
// GLOBAL: CMR2 0x0082b2c0
Unk0x0082b2c0 g_unk0x0082b2c0[8];
// GLOBAL: CMR2 0x00531c94
int g_unk0x00531c94[16];
// GLOBAL: CMR2 0x00531c98
int g_unk0x00531c98;

// FUNCTION: CMR2 0x005011a0
void CGameInfo::FUN_005011a0(void)
{
    g_unk0x0082af88 = CGame::GetCallbackCount();
}

// FUNCTION: CMR2 0x004f8a70
void CGameInfo::FUN_004f8a70(int index)
{
    CFrontend::GetTextString(index + 0x1f1);
}

// FUNCTION: CMR2 0x005011b0
int CGameInfo::FUN_005011b0(void)
{
    return FUN_00405d70() - g_unk0x0082b0a8;
}

// FUNCTION: CMR2 0x00500500
void CGameInfo::FUN_00500500(void)
{
    g_unk0x0082ac58 = 1;
    g_unk0x0082ac5c = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x005012c0
int CGameInfo::FUN_005012c0(void)
{
    int result;

    result = g_unk0x0082b0a0 - CMain::GetFrameDelta() + 0x17d5;
    if (result < 0)
        result = 0;
    return result;
}

// FUNCTION: CMR2 0x0040a420
int CGameInfo::FUN_0040a420(int index)
{
    if (FUN_00405d80() == 0xc)
        return g_unk0x00531c98;
    return g_unk0x00531c94[index];
}

// FUNCTION: CMR2 0x00501cc0
void CGameInfo::FUN_00501cc0(int index, int param2, int param3)
{
    Unk0x0082b2c0 *pEntry;

    pEntry = &g_unk0x0082b2c0[index];
    pEntry->field_0x8 = CMain::GetFrameDelta();
    pEntry->field_0x4 = param2;
    pEntry->field_0x0 = 0;
    pEntry->field_0xc = 1;
    pEntry->field_0x10 = param3;
}

// FUNCTION: CMR2 0x005004c0
int CGameInfo::FUN_005004c0(void)
{
    unsigned int delta;

    if (g_unk0x0082ac58 != 0) {
        delta = CMain::GetFrameDelta() - g_unk0x0082ac5c;
        return (unsigned char)~(delta / 10) & 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x00501230
int CGameInfo::FUN_00501230(void)
{
    if (RallyDataStageIndex() == 0xa ||
        FUN_00405d80() == 2 ||
        FUN_00405d80() == 3 ||
        FUN_00405d80() == 8 ||
        FUN_00405d80() == 9 ||
        FUN_00405d80() == 0xa)
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

// TODO: CMR2 0x00505e10 (implemented, match below 90%)
int CGameInfo::FUN_00505e10(BYTE param1)
{
    Unk0x0082c6c8 *pEntry;

    if (g_unk0x0082ca1c != 0xff) {
        pEntry = &g_unk0x0082c6c8[(signed char)g_unk0x0082ca1c];
        if (pEntry->field_0x4c != 0 || pEntry->field_0x1c != 0)
            return 0;
    }
    g_unk0x0082ca1c = param1;
    g_unk0x0082c6c0 = CMain::GetFrameDelta();
    g_unk0x0082cb44 = 0;
    return 1;
}

// GLOBAL: CMR2 0x0081a728
void *g_unk0x0081a728;
// GLOBAL: CMR2 0x0081a72c
void *g_unk0x0081a72c;
// GLOBAL: CMR2 0x0081a730
void *g_unk0x0081a730;
// GLOBAL: CMR2 0x0081a734
void *g_unk0x0081a734;

// FUNCTION: CMR2 0x004f48b0
bool FUN_004f48b0(void)
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
    g_unk0x0081a728 = NULL;
    return true;
}

// FUNCTION: CMR2 0x004f8290
void *FUN_004f8290(BYTE param1)
{
    if (param1 != 0 && CGameInfo::GetGameRegion() != 3 && CGameInfo::GetGameRegion() != 2)
        return (void *)0x8221d8;
    return (void *)0x81d6d8;
}

// GLOBAL: CMR2 0x00819128
int g_unk0x00819128;

// FUNCTION: CMR2 0x004f2360
void FUN_004f2360(BYTE *p, int param2)
{
    g_unk0x00819128 = CGraphics::GetSelectedDisplayDeviceIx();
    if (g_unk0x00819128 + 10 > (int)CGraphics::GetDisplayCount()) {
        g_unk0x00819128 = (int)CGraphics::GetDisplayCount() - 10;
        if (g_unk0x00819128 < 0)
            g_unk0x00819128 = 0;
    }
    p[0x1e] = (BYTE)CGraphics::GetDisplayCount();
    p[0x1f] = (BYTE)CGraphics::GetSelectedDisplayDeviceIx();
    CGame::FUN_004a9b10(0);
}

// TODO: CMR2 0x004f2b00 (implemented, match 73%)
void FUN_004f2b00(BYTE *p)
{
    unsigned int v;

    CGameInfo::FUN_00405e10(p[0x1f] * 10);
    CGameInfo::FUN_00405e50(p[0x33] * 10);
    CGameInfo::FUN_00405e80(p[0x47] * 10);
    v = (CGameInfo::FUN_00405e70() * 65536) / 100;
    CInput::FUN_0049ffc0(v / 4);
    CSound::FUN_004a28c0();
}

// GLOBAL: CMR2 0x0081b14c
void *g_unk0x0081b14c;
// GLOBAL: CMR2 0x0081b150
void **g_unk0x0081b150;
// GLOBAL: CMR2 0x0081b154
int g_unk0x0081b154;

// TODO: CMR2 0x004f4e80 (implemented, match 50%)
bool FUN_004f4e80(void)
{
    int i;

    for (i = 0; i < g_unk0x0081b154; i++) {
        if (g_unk0x0081b150[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x0081b150[i]);
            g_unk0x0081b150[i] = NULL;
        }
    }
    if (g_unk0x0081b14c != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0081b14c);
    g_unk0x0081b14c = NULL;
    g_unk0x0081b150 = NULL;
    g_unk0x0081b154 = 0;
    return true;
}

// GLOBAL: CMR2 0x00818ac8
int g_unk0x00818ac8;

// FUNCTION: CMR2 0x004ea480
void FUN_004ea480(int param1)
{
    if (param1 == 1) {
        if (!CGameInfo::FUN_00405da0() && CGameInfo::FUN_00405d80() != 4)
            g_unk0x00818ac8 = 1;
        else
            g_unk0x00818ac8 = 0;
    } else if (param1 == 2) {
        if (!CGameInfo::FUN_00405da0() && CGameInfo::FUN_00405d80() != 4)
            g_unk0x00818ac8 = 2;
        else
            g_unk0x00818ac8 = 0;
    } else if (param1 == 3) {
        if (CGameInfo::FUN_00405da0()) {
            g_unk0x00818ac8 = 0;
        } else {
            g_unk0x00818ac8 = 3;
            if (CGameInfo::FUN_00405d80() == 4)
                g_unk0x00818ac8 = 0;
        }
    }
}

// GLOBAL: CMR2 0x0052afc0
unsigned int g_unk0x0052afc0;

// FUNCTION: CMR2 0x004eabc0
void FUN_004eabc0(void)
{
    if (CGameInfo::FUN_00406320() || CGameInfo::FUN_00405d80() == 0 ||
        CGameInfo::FUN_00405d80() == 4) {
        g_unk0x0052afc0 = (g_unk0x0052afc0 & 0xff00ffff) |
                          ((g_unk0x0052afc0 & 0x400) << 8);
    } else if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6) {
        g_unk0x0052afc0 = (g_unk0x0052afc0 & 0xff00ffff) |
                          ((g_unk0x0052afc0 & 0xd00) << 8);
    } else {
        g_unk0x0052afc0 = (g_unk0x0052afc0 & 0xff00ffff) |
                          ((g_unk0x0052afc0 & 0xff00) << 8);
    }
}
