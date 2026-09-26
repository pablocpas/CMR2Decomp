#include "GameInfo.h"
#include "Menu.h"
#include "Graphics.h"
#include "Input.h"
#include "Frontend.h"
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
int CGameInfo::FUN_004063f0(int param1)
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
DWORD CGameInfo::SetupInputs(int unused) {
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

// GLOBAL: CMR2 0x00817780
int g_unk0x00817780;
// GLOBAL: CMR2 0x00817784
char g_unk0x00817784[5][256];
// GLOBAL: CMR2 0x00817c84
char *g_unk0x00817c84[5];

// FUNCTION: CMR2 0x004d05f0
void FUN_004d05f0(void)
{
    char **pp = g_unk0x00817c84;
    char *p = g_unk0x00817784[0];

    do {
        *p = 0;
        *pp = p;
        p += 256;
        pp++;
    } while ((int)p < (int)g_unk0x00817c84);
    g_unk0x00817780 = -1;
}

int FUN_004a19c0(DPID *pId, char *pIndex);
Unk0x005a1820 *FUN_004a1b30(BYTE index);
char FUN_004a1c50(int to, int guaranteed, int data, int size);

// GLOBAL: CMR2 0x00523bbc
char g_chatLineFormat[] = "%s > %s";

// Adds a chat line "name > text" to the ring of the last five lines and
// rebuilds g_unk0x00817c84 newest first.
// TODO: CMR2 0x004d0620 (implemented, match 91%)
void FUN_004d0620(DPID *pFrom, char *text, char local)
{
    char **pp;
    int line;
    int i;

    if (local) {
        if (++g_unk0x00817780 >= 5)
            g_unk0x00817780 = 0;
        sprintf(g_unk0x00817784[g_unk0x00817780], g_chatLineFormat, (char *)RallyData_GetRecord(0), text);
    } else {
        if (!FUN_004a19c0(pFrom, &local))
            return;
        if (++g_unk0x00817780 >= 5)
            g_unk0x00817780 = 0;
        sprintf(g_unk0x00817784[g_unk0x00817780], g_chatLineFormat, FUN_004a1b30(local)->field_0x0, text);
    }
    line = g_unk0x00817780 + 5;
    pp = g_unk0x00817c84;
    i = 5;
    do {
        *pp++ = g_unk0x00817784[line-- % 5];
    } while (--i);
}

// Sends a chat line to every player and adds it to the local log.
// FUNCTION: CMR2 0x004d0700
void FUN_004d0700(char *text)
{
    char message[0x101];

    message[0] = 0;
    strcpy(message + 1, text);
    FUN_004a1c50(0, 0, (int)message, 0x101);
    FUN_004d0620(NULL, text, 1);
}

// FUNCTION: CMR2 0x00406710
int FUN_00406710(void)
{
    return CGameInfo::m_gameInfo.field_0x397c;
}

// FUNCTION: CMR2 0x00406730
int FUN_00406730(void)
{
    return CGameInfo::m_gameInfo.field_0x3984;
}

// FUNCTION: CMR2 0x00406720
void FUN_00406720(int param1)
{
    CGameInfo::m_gameInfo.field_0x397c = param1;
}

// FUNCTION: CMR2 0x00406740
void FUN_00406740(int param1)
{
    CGameInfo::m_gameInfo.field_0x3984 = param1;
}

// FUNCTION: CMR2 0x00406750
int FUN_00406750(void)
{
    return CGameInfo::m_gameInfo.field_0x3988;
}

// FUNCTION: CMR2 0x00406760
void FUN_00406760(int param1)
{
    CGameInfo::m_gameInfo.field_0x3988 = param1;
}

// FUNCTION: CMR2 0x00406770
int FUN_00406770(void)
{
    return CGameInfo::m_gameInfo.field_0x398c;
}

// FUNCTION: CMR2 0x00406790
int FUN_00406790(void)
{
    return CGameInfo::m_gameInfo.field_0x3990;
}

// FUNCTION: CMR2 0x004067a0
void FUN_004067a0(int param1)
{
    CGameInfo::m_gameInfo.field_0x3990 = param1;
}

// FUNCTION: CMR2 0x00406780
void FUN_00406780(int param1)
{
    CGameInfo::m_gameInfo.field_0x398c = param1;
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

    g_sessionNamePtr = (LPVOID *)m_unk0x005a00b8;
    g_sessionPasswordPtr = (LPVOID *)m_unk0x005a02c0;

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
// GLOBAL: CMR2 0x0082c040
BYTE g_unk0x0082c040[4][12];
// Option records, copied from RallyData_FUN_00407610 by FUN_00502d50; each
// entry is 0x148 bytes and the block runs up to the globals at 0x82c698.
#define g_unk0x0082c070 ((BYTE *)&g_unk0x0082c040[4][0])
struct Unk0x0082d220Vec {
    int v[4];
};

struct Unk0x0082d220 {
    BYTE field_0x0[0x22c];
    Unk0x0082d220Vec field_0x22c;
    Unk0x0082d220Vec field_0x23c;
    BYTE field_0x24c[0x60];
};

// GLOBAL: CMR2 0x0082d220
Unk0x0082d220 g_unk0x0082d220[8];
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
int FUN_004ff4b0(int index)
{
    return g_unk0x00526f54[index];
}

// FUNCTION: CMR2 0x004ff4c0
int FUN_004ff4c0(int index)
{
    return g_unk0x00526f70[index];
}

// FUNCTION: CMR2 0x004ff4d0
int FUN_004ff4d0(int index)
{
    return g_unk0x00526f8c[index];
}

// FUNCTION: CMR2 0x004ff540
int FUN_004ff540(void)
{
    return g_unk0x0082a928 - g_unk0x0082a930;
}

// FUNCTION: CMR2 0x004ff5a0
int FUN_004ff5a0(int index)
{
    return g_unk0x0082a90c[index];
}

// FUNCTION: CMR2 0x005004a0
int FUN_005004a0(void)
{
    return g_unk0x0082a92c;
}

// FUNCTION: CMR2 0x005004b0
void FUN_005004b0(int value)
{
    g_unk0x0082a92c = value;
}

// FUNCTION: CMR2 0x00500520
void FUN_00500520(void)
{
    g_unk0x0082ac58 = 0;
    g_unk0x0082ac5c = 0;
}

// FUNCTION: CMR2 0x00500530
void FUN_00500530(void)
{
    if (g_unk0x0082ac58 != 0 && (unsigned int)(CMain::GetFrameDelta() - g_unk0x0082ac5c) >= 30)
        FUN_00500520();
}

// FUNCTION: CMR2 0x005011d0
int FUN_005011d0(void)
{
    return g_unk0x0082b0a8;
}

// FUNCTION: CMR2 0x005011e0
void FUN_005011e0(int value)
{
    g_unk0x0082b0a8 = value;
}

// FUNCTION: CMR2 0x005011f0
int FUN_005011f0(int index)
{
    return g_unk0x0082af90[index];
}

// FUNCTION: CMR2 0x00501200
int FUN_00501200(int index)
{
    return g_unk0x0082af78[index];
}

// FUNCTION: CMR2 0x00501210
void FUN_00501210(int index, int value)
{
    g_unk0x0082af90[index] = value;
}

// FUNCTION: CMR2 0x00501510
int FUN_00501510(void)
{
    return g_unk0x0082b1b4;
}

// FUNCTION: CMR2 0x00501d00
void FUN_00501d00(int index)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];

    p->field_0x8 = 0;
    p->field_0x4 = 0;
    p->field_0x0 = 0;
    p->field_0xc = 0;
}

// FUNCTION: CMR2 0x00501d20
void FUN_00501d20(int count)
{
    int i;

    g_unk0x0082b1bc = count;
    for (i = 0; i < g_unk0x0082b1bc; i++)
        FUN_00501d00(i);
}

// FUNCTION: CMR2 0x005021c0
int FUN_005021c0(int index)
{
    return g_unk0x0082b2c0[index].field_0xc;
}

// FUNCTION: CMR2 0x005021e0
void FUN_005021e0(void)
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
BYTE *FUN_00502210(void)
{
    return g_unk0x0082ba28;
}

// FUNCTION: CMR2 0x00502220
BYTE *FUN_00502220(void)
{
    return g_unk0x0082b488;
}

// FUNCTION: CMR2 0x00502990
BYTE FUN_00502990(int i, int j)
{
    return g_unk0x0082bf20[i][j];
}

// FUNCTION: CMR2 0x00502d40
int FUN_00502d40(int index)
{
    return g_unk0x00527098[index];
}

// FUNCTION: CMR2 0x00503930
int FUN_00503930(int index)
{
    return g_unk0x005270b4[index];
}

// FUNCTION: CMR2 0x00503940
BYTE FUN_00503940(int i, int j)
{
    return g_unk0x0082c040[i][j];
}

// FUNCTION: CMR2 0x00509d00
void FUN_00509d00(int index)
{
    Unk0x0082d220Vec *p = &g_unk0x0082d220[index].field_0x22c;

    p->v[0] = 0;
    p->v[1] = 0;
    p->v[2] = 0;
    p->v[3] = 0;
}

// FUNCTION: CMR2 0x00509d90
void FUN_00509d90(int index)
{
    Unk0x0082d220Vec *p = &g_unk0x0082d220[index].field_0x23c;

    p->v[0] = 0;
    p->v[1] = 0;
    p->v[2] = 0;
    p->v[3] = 0;
}

// FUNCTION: CMR2 0x0050a020
int FUN_0050a020(int mode, int type)
{
    if (type == 6 && mode != 11 && mode != 8 && mode != 10 && mode != 13)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x0050a050
int FUN_0050a050(int mode, int type)
{
    if (type <= 1 && mode != 11 && mode != 8 && mode != 10 && mode != 13)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x0050f1c0
void FUN_0050f1c0(void)
{
    g_pMenu0x00831778 = g_pMenu0x0083177c;
}

// FUNCTION: CMR2 0x0050f230
void FUN_0050f230(void)
{
    Menu_CallCallback2(g_pMenu0x00831778);
}

// FUNCTION: CMR2 0x0050f620
int *FUN_0050f620(void)
{
    return g_unk0x00831a90;
}

// FUNCTION: CMR2 0x0050f630
int *FUN_0050f630(void)
{
    return g_unk0x00831ab0;
}

// FUNCTION: CMR2 0x0050f640
int *FUN_0050f640(void)
{
    return g_unk0x00831aa0;
}

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
        return g_netStageBest[0];
    return g_netStageBest[index - 1];
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

bool FUN_004f48b0(void);

// Loads the credits text of the current language and splits it into
// entries of two quoted strings ("role", "name"); each quote-delimited
// string is terminated in place. Optionally registers the release callback.
// FUNCTION: CMR2 0x004f4910
void FUN_004f4910(char registerRelease)
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
    if (g_unk0x0081a734 == NULL) {
        g_unk0x0081a728 = 0;
    } else {
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
    }

    if (registerRelease)
        CGame::RegisterCallback(FUN_004f48b0, NULL);
}

// FUNCTION: CMR2 0x004f4b10
void *FUN_004f4b10(void)
{
    return g_unk0x0081a734;
}

// FUNCTION: CMR2 0x004f4b20
int FUN_004f4b20(void)
{
    return g_unk0x0081a728;
}

// FUNCTION: CMR2 0x004f4b30
void *FUN_004f4b30(void)
{
    return g_unk0x0081a72c;
}

// Frontend text files per region (index = language, see CGameInfo::FUN_004f4b40)
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

// Frontend fonts (Fonts\<name>.tga + .pcf)
// GLOBAL: CMR2 0x00525b9c
char g_strFontHel12[20] = "general\\hel_12pt";
// GLOBAL: CMR2 0x00525bb0
char g_strFontHel15[20] = "general\\hel_15pt";
// GLOBAL: CMR2 0x00525bc4
char g_strFontHel36[20] = "general\\hel_36pt";
// GLOBAL: CMR2 0x00525bd8
char g_strFontDot[20] = "general\\dot";

// Text buffers of the frontend languages and whether each one lives inside
// its archive (then it is not freed on release)
// GLOBAL: CMR2 0x0081a738
void *g_languageTexts[5];
// GLOBAL: CMR2 0x0081a74c
BYTE g_languageTextInArchive[5];

extern char g_strTxtFormat[];

// Release callback of FUN_004f4b90.
// FUNCTION: CMR2 0x004f4cc0
BYTE FUN_004f4cc0(void)
{
    BYTE i;

    for (i = 0; i < CGameInfo::m_unk0x0081a754; i++) {
        if (g_languageTextInArchive[i] == 0) {
            CFileBuffer::FreeGenericFileBuffer(g_languageTexts[i]);
            g_languageTexts[i] = NULL;
        }
        g_languageTexts[i] = NULL;
    }
    CFrontend::FUN_004a3d80();
    return 1;
}

// Loads the text file of every frontend language of this region from its
// language archive and hands them to the frontend text tables.
// TODO: CMR2 0x004f4b90 (implemented, match 78%)
BYTE FUN_004f4b90(void)
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
        g_languageTexts[i] = CGenericFileLoader::FindFile(CFrontend::FUN_004d21c0(i), CFrontend::m_stringDest,
                                                        &g_languageTextInArchive[i], NULL, 0);
    }
    if (g_languageTexts[0] == NULL)
        return 0;
    CFrontend::FUN_004a3c90(CGameInfo::m_unk0x0081a754, 0x215, (BYTE **)g_languageTexts);
    CGame::RegisterCallback(FUN_004f4cc0, NULL);
    return 1;
}

// Loads the four frontend fonts from the frontend archive.
// FUNCTION: CMR2 0x004f4d20
BYTE FUN_004f4d20(void)
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
    g_unk0x0081a728 = 0;
    return true;
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

// FUNCTION: CMR2 0x004f4db0
int FUN_004f4db0(void)
{
    return g_unk0x0081b154;
}

// Saved games list: records of 0x7f4 bytes
// FUNCTION: CMR2 0x004f4dc0
BYTE *FUN_004f4dc0(int index)
{
    return (BYTE *)g_unk0x0081b14c + 0x10 + index * 0x7f4;
}

// FUNCTION: CMR2 0x004f4de0
unsigned int FUN_004f4de0(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x30 + index * 0x7f4) >> 10 & 0xf;
}

// FUNCTION: CMR2 0x004f4e00
unsigned int FUN_004f4e00(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x7d8 + index * 0x7f4) & 0x1f;
}

// FUNCTION: CMR2 0x004f4e20
unsigned int FUN_004f4e20(int index)
{
    return *(unsigned int *)((BYTE *)g_unk0x0081b14c + 0x7d8 + index * 0x7f4) >> 5 & 0x1f;
}

// FUNCTION: CMR2 0x004f4e50
BYTE *FUN_004f4e50(int index)
{
    return (BYTE *)g_unk0x0081b14c + index * 0x7f4;
}

// FUNCTION: CMR2 0x004f4e70
void *FUN_004f4e70(int index)
{
    return g_unk0x0081b150[index];
}

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

#include <io.h>

// Formats into the buffer the first save game file name that does not exist
// yet (<install>\gamesave\gameNNNNN.rcs), starting from index 0.
// FUNCTION: CMR2 0x004f5150
void FUN_004f5150(char *pName)
{
    bool exists = true;
    int i;

    for (i = 0; exists; i++) {
        sprintf(pName, g_strSaveGameFileFormat, CInstallInfo::GetGameHDPath(), i);
        if (_access(pName, 0) == -1)
            exists = false;
    }
}

// Copies the string into CFrontend::m_stringDest and writes it back into the
// buffer in groups of four characters, one space between groups (the last
// character is dropped).
// TODO: CMR2 0x004f8a90 (implemented, match 54%)
void FUN_004f8a90(char *pText)
{
    int len;
    int i;
    int out;

    strcpy(CFrontend::m_stringDest, pText);
    len = (int)strlen(CFrontend::m_stringDest) - 1;
    i = 1;
    out = 0;
    for (; i - 1 < len; i++) {
        pText[out++] = CFrontend::m_stringDest[i - 1];
        if ((i & 3) == 0)
            pText[out++] = ' ';
    }
    pText[out] = 0;
}

// Reads every saved game (<install>\gamesave\*.rcs) into the saved games
// list: one 0x7f4-byte record per file plus its file name.
// TODO: CMR2 0x004f4ef0 (implemented, match 79%)
void FUN_004f4ef0(void)
{
    WIN32_FIND_DATAA find;
    char saveDir[260];
    char oldDir[260];
    HANDLE hFind;
    void *pRecord;

    FUN_004f4e80();
    g_unk0x0081b154 = 0;
    GetCurrentDirectoryA(sizeof(oldDir), oldDir);
    sprintf(saveDir, g_strSaveGameDirFormat, CInstallInfo::GetGameHDPath());
    if (SetCurrentDirectoryA(saveDir) != 0 &&
        (hFind = FindFirstFileA(g_strSaveGamePattern, &find)) != INVALID_HANDLE_VALUE) {
        do {
            pRecord = CFileBuffer::GetGenericFileBuffer(find.cFileName, TRUE);
            if (pRecord != NULL) {
                g_unk0x0081b14c = CFileBuffer::ReallocateLockedBuffer(g_unk0x0081b14c, (g_unk0x0081b154 + 1) * 0x7f4);
                memcpy((BYTE *)g_unk0x0081b14c + g_unk0x0081b154 * 0x7f4, pRecord, 0x7f4);
                g_unk0x0081b150 = (void **)CFileBuffer::ReallocateLockedBuffer(g_unk0x0081b150, g_unk0x0081b154 * 4 + 4);
                g_unk0x0081b150[g_unk0x0081b154] = CFileBuffer::AllocateLockedBuffer(0x100);
                strcpy((char *)g_unk0x0081b150[g_unk0x0081b154], find.cFileName);
                g_unk0x0081b154++;
                CFileBuffer::FreeGenericFileBuffer(pRecord);
            }
        } while (FindNextFileA(hFind, &find) != 0);
        FindClose(hFind);
    }
    SetCurrentDirectoryA(oldDir);
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

// FUNCTION: CMR2 0x004ea500
int FUN_004ea500(void)
{
    return g_unk0x00818ac8;
}

// FUNCTION: CMR2 0x004ea8e0
void FUN_004ea8e0(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 0x7f) << 3) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffffc07);
    if (param1 >= 8)
        CGameInfo::m_gameInfo.field_0x3980 = CGameInfo::m_gameInfo.field_0x14 >> 3 & 0x7f;
}

// FUNCTION: CMR2 0x004ea970
void FUN_004ea970(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7) << 20) | (CGameInfo::m_gameInfo.field_0x14 & 0xff8fffff);
}

// FUNCTION: CMR2 0x004ea990
void FUN_004ea990(int *pOut1, int *pOut2)
{
    *pOut1 = CGameInfo::m_gameInfo.field_0x14 >> 23 & 0xf;
    *pOut2 = CGameInfo::m_gameInfo.field_0x14 >> 27 & 0xf;
}

// FUNCTION: CMR2 0x004ea8a0
void FUN_004ea8a0(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7)) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffffff8U);
}

// FUNCTION: CMR2 0x004ea8c0
void FUN_004ea8c0(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 0xf) << 13) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffe1fffU);
}

// FUNCTION: CMR2 0x004ea920
BYTE FUN_004ea920(void)
{
    return CGameInfo::m_gameInfo.field_0x3980;
}

// FUNCTION: CMR2 0x004ea930
void FUN_004ea930(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 7) << 10) | (CGameInfo::m_gameInfo.field_0x14 & 0xffffe3ffU);
}

// FUNCTION: CMR2 0x004ea950
void FUN_004ea950(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x14 = ((param1 & 1) << 17) | (CGameInfo::m_gameInfo.field_0x14 & 0xfffdffffU);
}

// FUNCTION: CMR2 0x004ea9c0
void FUN_004ea9c0(unsigned int param1, unsigned int param2)
{
    CGameInfo::m_gameInfo.field_0x14 = (((param2 & 0xf) << 4 | param1 & 0xf) << 23) | (CGameInfo::m_gameInfo.field_0x14 & 0x807fffffU);
}

// GLOBAL: CMR2 0x0052af90
DWORD g_unk0x0052af90;

// FUNCTION: CMR2 0x004ea9f0
void FUN_004ea9f0(void)
{
    g_unk0x0052af90 = timeGetTime();
}

// FUNCTION: CMR2 0x004eaa00
DWORD FUN_004eaa00(void)
{
    return g_unk0x0052af90;
}

// FUNCTION: CMR2 0x004eaa10
void FUN_004eaa10(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x18 = ((param1 & 1) << 22) | (CGameInfo::m_gameInfo.field_0x18 & 0xffbfffffU);
}

// FUNCTION: CMR2 0x004eaa30
char FUN_004eaa30(void)
{
    return 1;
}

// FUNCTION: CMR2 0x004eaa40
void FUN_004eaa40(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 3) << 1) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffffff9U);
}

// FUNCTION: CMR2 0x004eaa60
void FUN_004eaa60(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 7) << 8) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffff8ffU);
}

// FUNCTION: CMR2 0x004eaa80
unsigned int FUN_004eaa80(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 8 & 7;
}

// FUNCTION: CMR2 0x004eaa90
void FUN_004eaa90(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0x1f) << 3) | (CGameInfo::m_gameInfo.field_0x1c & 0xffffff07U);
}

// FUNCTION: CMR2 0x004eaab0
unsigned int FUN_004eaab0(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 3 & 0x1f;
}

// Number of bits set.
// FUNCTION: CMR2 0x004eaac0
int FUN_004eaac0(unsigned int value)
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

// Default stage setting values from the game options.
// TODO: CMR2 0x004eaae0 (implemented, match 80%)
void FUN_004eaae0(int *pValues, int *pOut1, int *pOut2)
{
    pValues[0] = 0;
    pValues[1] = *(int *)((BYTE *)&CGameInfo::m_gameInfo + 0x88);
    pValues[2] = *(int *)((BYTE *)&CGameInfo::m_gameInfo + 0x8c);
    *pOut1 = *(int *)((BYTE *)&CGameInfo::m_gameInfo + 0x94);
    *pOut2 = *(int *)((BYTE *)&CGameInfo::m_gameInfo + 0x90);
}

// FUNCTION: CMR2 0x004eab20
void FUN_004eab20(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x18 = ((param1 & 1) << 30) | (CGameInfo::m_gameInfo.field_0x18 & 0xbfffffffU);
}

// Turns cheat `bit` on or off in the game options.
// FUNCTION: CMR2 0x004eab40
void FUN_004eab40(int bit, int value)
{
    BYTE mask = 1 << bit;

    if (value != 0) {
        *(unsigned int *)((BYTE *)&CGameInfo::m_gameInfo + 0x20) |= (unsigned int)mask << 8;
        return;
    }
    *(unsigned int *)((BYTE *)&CGameInfo::m_gameInfo + 0x20) &= (unsigned int)(BYTE)~mask << 8 | 0xffff00ff;
}

// FUNCTION: CMR2 0x004eab80
void FUN_004eab80(void)
{
    CGameInfo::m_gameInfo.field_0x20 = ((CGameInfo::m_gameInfo.field_0x20 & 0xff00) << 16) | (CGameInfo::m_gameInfo.field_0x20 & 0xffffff);
}

// FUNCTION: CMR2 0x004eaba0
void FUN_004eaba0(void)
{
    CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 >> 24) << 8 | (CGameInfo::m_gameInfo.field_0x20 & 0xffff00ffU);
}

// FUNCTION: CMR2 0x004eac80
void FUN_004eac80(BYTE param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 1) << 11) | (CGameInfo::m_gameInfo.field_0x1c & 0xfffff7ffU);
}

// FUNCTION: CMR2 0x004eaca0
int FUN_004eaca0(void)
{
    return *(int *)&CGameInfo::m_gameInfo.field_0x98;
}

// FUNCTION: CMR2 0x004eacb0
void FUN_004eacb0(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0xf) << 12) | (CGameInfo::m_gameInfo.field_0x1c & 0xffff0fffU);
}

// FUNCTION: CMR2 0x004eacd0
void FUN_004eacd0(unsigned int param1)
{
    CGameInfo::m_gameInfo.field_0x1c = ((param1 & 0xf) << 16) | (CGameInfo::m_gameInfo.field_0x1c & 0xfff0ffffU);
}

// FUNCTION: CMR2 0x004eacf0
unsigned int FUN_004eacf0(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 12 & 0xf;
}

// FUNCTION: CMR2 0x004ead00
unsigned int FUN_004ead00(void)
{
    return CGameInfo::m_gameInfo.field_0x1c >> 16 & 0xf;
}

void RallyData_FUN_00406960(BYTE param1);
void RallyData_FUN_004068e0(BYTE param1);
void RallyData_FUN_004068b0(BYTE param1);

// Pushes the frontend option settings to the modules that use them.
// FUNCTION: CMR2 0x004ead10
void FUN_004ead10(void)
{
    FUN_004ea8e0((BYTE)CGameInfo::m_unk0x0052af94);
    FUN_004ea8c0((BYTE)CGameInfo::m_unk0x0052ea44);
    FUN_004ea930((BYTE)CGameInfo::m_unk0x0052af9c);
    RallyData_FUN_0040d640((BYTE)CGameInfo::m_unk0x0052af80);
    RallyData_FUN_0040d600((BYTE)CGameInfo::m_unk0x0052af84);
    RallyData_FUN_00406960((BYTE)CGameInfo::m_unk0x0052af8c);
    RallyData_FUN_0040d620((BYTE)CGameInfo::m_unk0x0052e93c);
    RallyData_FUN_004068e0((BYTE)CGameInfo::m_unk0x0052ea48);
    RallyData_FUN_004068b0((BYTE)CGameInfo::m_unk0x0052e940);
    CGameInfo::m_gameInfo.field_0x14 = ((CGameInfo::m_unk0x0052af88 & 0xf) << 4 | CGameInfo::m_unk0x0052af98 & 0xf) << 0x17
        | CGameInfo::m_gameInfo.field_0x14 & 0x807fffff;
}

// FUNCTION: CMR2 0x004eabc0
void FUN_004eabc0(void)
{
    if (CGameInfo::FUN_00406320() || CGameInfo::FUN_00405d80() == 0 ||
        CGameInfo::FUN_00405d80() == 4) {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0x400) << 8);
    } else if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6) {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0xd00) << 8);
    } else {
        CGameInfo::m_gameInfo.field_0x20 = (CGameInfo::m_gameInfo.field_0x20 & 0xff00ffff) |
                          ((CGameInfo::m_gameInfo.field_0x20 & 0xff00) << 8);
    }
}

// GLOBAL: CMR2 0x00511300
double g_unk0x00511300 = 4096.0 / (360.0 * 65536.0);   // 16.16 degrees -> sine table index
// GLOBAL: CMR2 0x0082b1b8
BYTE g_unk0x0082b1b8;
// GLOBAL: CMR2 0x0082b1b9
BYTE g_unk0x0082b1b9;
// GLOBAL: CMR2 0x0082b1ba
BYTE g_unk0x0082b1ba;
// GLOBAL: CMR2 0x0082b1bb
BYTE g_unk0x0082b1bb;


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


// Prepara el descriptor de sesion 0x5a0068 y crea la sesion de DirectPlay.
// TODO: CMR2 0x004a13b0 (implemented, match 58%)
void CGameInfo::FUN_004a13b0(void)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    if (m_unk0x005a1814 != 0)
        return;
    FUN_004a0c60();
    memset(g_unk0x005a0068, 0, 0x50);
    *(int *)(g_unk0x005a0068 + 0x18) = g_unk0x00511cd8[0];
    *(int *)(g_unk0x005a0068) = 0x50;
    *(int *)(g_unk0x005a0068 + 0x1c) = g_unk0x00511cd8[2];
    *(int *)(g_unk0x005a0068 + 0x20) = g_unk0x00511cd8[1];
    *(int *)(g_unk0x005a0068 + 0x24) = g_unk0x00511cd8[3];
    g_sessionNamePtr = (LPVOID *)&m_unk0x005a00b8;
    pDP = CGame::GetDirectPlay();
    if (pDP == NULL)
        return;
    hr = ((DPMethod5GI)(*(void ***)pDP)[0x34 / 4])(pDP, (DWORD)g_unk0x005a0068, 0,
                                                (DWORD)Session_EnumCallback, 0, 0x20);
    if (hr > (HRESULT)0x887700aa) {
        if (hr == (HRESULT)0x8877015e)
            return;
        if (hr == (HRESULT)0x887700cb)
            return;
        if (hr != 0)
            return;
        m_unk0x005a1814 = 1;
        return;
    }
    if (hr == (HRESULT)0x887700aa || hr <= (HRESULT)0x8877005a ||
        hr == (HRESULT)0x88770082)
        return;
}

// Variante de FUN_004a13b0 que ademas guarda el parametro en 0x5a009c y usa
// 0x51 como tamano inicial.
// TODO: CMR2 0x004a12d0 (implemented, match 65%)
void CGameInfo::FUN_004a12d0(int param1)
{
    IDirectPlay4A *pDP;
    HRESULT hr;

    if (m_unk0x005a1814 != 0)
        return;
    FUN_004a0c60();
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
        return;
    hr = ((DPMethod5GI)(*(void ***)pDP)[0x34 / 4])(pDP, (DWORD)g_unk0x005a0068, 0,
                                                  (DWORD)Session_EnumCallback, 0, 0x51);
    if (hr > (HRESULT)0x887700aa) {
        if (hr == (HRESULT)0x8877015e)
            return;
        if (hr != 0)
            return;
        m_unk0x005a1814 = 1;
        return;
    }
    if (hr == (HRESULT)0x887700aa || hr <= (HRESULT)0x8877005a ||
        hr == (HRESULT)0x88770082)
        return;
}



// Cambia el modo activo 0x82ca1c (intercambiando 0x3c con el modo anterior) y
// reinicia el temporizador.
// TODO: CMR2 0x00505a60 (implemented, match 51%)
void CGameInfo::FUN_00505a60(int param1)
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
        tmp = *(int *)((char *)pNew + 0x3c);
        *(int *)((char *)pNew + 0x3c) = *(int *)((char *)pOld + 0x3c);
        *(int *)((char *)pOld + 0x3c) = tmp;
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
BYTE g_unk0x0082b668[0x40];

// FUNCTION: CMR2 0x00502500
BYTE *FUN_00502500(void)
{
    return g_unk0x0082b668;
}

// GLOBAL: CMR2 0x0082b848
BYTE g_unk0x0082b848[0x40];

// FUNCTION: CMR2 0x00502510
BYTE *FUN_00502510(void)
{
    return g_unk0x0082b848;
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
// GLOBAL: CMR2 0x0052af58
BYTE g_unk0x0052af58[2];

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

void FUN_00404ef0(void);
void FUN_004a0ba0(void);
void FUN_004a3180(void);
unsigned int RallyData_FUN_00407e70(void);

// TODO: CMR2 0x00401850 (implemented, match 83%)
void FUN_00401850(Menu *pMenu, int param)
{
    if ((BYTE)RallyData_FUN_00407e70())
        FUN_004a3180();
    FUN_00404ef0();
}

// FUNCTION: CMR2 0x00402bb0
void FUN_00402bb0(Menu *pMenu, char param)
{
    if (param == 0) {
        pMenu->cursor = (char)Menu_FindItem(pMenu, 7);
        g_unk0x0052af6c = pMenu->cursor;
        return;
    }
    g_unk0x0052af6c = pMenu->cursor;
}

// TODO: CMR2 0x00402bf0 (implemented, match 19%, registers only)
void FUN_00402bf0(Menu *pMenu)
{
    if (pMenu->cursor == 2) {
        pMenu->cursor = ((g_unk0x0052af6c >= 2) - 1 & 2) + 1;
        g_unk0x0052af6c = pMenu->cursor;
        return;
    }
    g_unk0x0052af6c = pMenu->cursor;
}

// TODO: CMR2 0x00402c30 (implemented, match 50%)
void FUN_00402c30(Menu *pMenu)
{
    FUN_00404ef0();
}

// FUNCTION: CMR2 0x00402c40
void FUN_00402c40(void)
{
    Menu_Init(&g_menu0x00529ce8, 0, 0, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00529ce8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x00529ce8, NULL, NULL, (MenuCallback)FUN_00402c30, NULL);
}

// Callback 0 of the network options menu: loads the two switches.
// FUNCTION: CMR2 0x00402eb0
void FUN_00402eb0(Menu *pMenu, char param)
{
    if (param == 0) {
        if (CGameInfo::FUN_00405dc0() == 0)
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 0;
        else
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 1;
        if ((BYTE)CGameInfo::FUN_00405eb0()) {
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
            return;
        }
        pMenu->items[Menu_FindItem(pMenu, 4)].max = 0;
    }
}

// FUNCTION: CMR2 0x00402f70
void FUN_00402f70(Menu *pMenu, int param)
{
    Menu_SetNextAction((int)&g_menu0x0052ad60);
}

// FUNCTION: CMR2 0x00402f80
void FUN_00402f80(Menu *pMenu)
{
    Menu_SetNextAction((int)&g_menu0x0052ad60);
}

// FUNCTION: CMR2 0x00403360
void FUN_00403360(Menu *pMenu, int param)
{
    pMenu->cursor = pMenu->itemCount - 1;
}

// FUNCTION: CMR2 0x004035d0
void FUN_004035d0(Menu *pMenu, int param)
{
    g_unk0x0052af58[1] = pMenu->cursor;
}

// GLOBAL: CMR2 0x0052a0b8
short g_unk0x0052a0b8;

unsigned short FUN_0040bbc0(unsigned short slot);

// Returns 1 when the slot's device pressed "back" (button 0x400, or the
// joystick button mapped to action 9, which is then remembered).
// TODO: CMR2 0x00404e10 (implemented, match 89%)
int FUN_00404e10(unsigned short slot)
{
    DeviceInfo *pDev = CInput::FUN_0049ead0(FUN_0040bbc0(slot));

    if (pDev->field_0x0 == 1 || pDev->field_0x0 == 2) {
        if ((pDev->field_0x8 & 0x400) != 0)
            return 1;
    } else {
        if ((pDev->field_0x8 & CInput::GetButtonMapping(slot, 9) & 0xffff) != 0) {
            g_unk0x0052a0b8 = CInput::GetButtonMapping(slot, 9);
            return 1;
        }
        if ((CInput::FUN_0049ead0(0)->field_0x8 & 0x400) != 0)
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
void FUN_00403200(Menu *pMenu)
{
    g_unk0x0052aa5c = (int)(pMenu->items[2].max << 19) / (pMenu->items[2].min - 1) + 0x80000;
    g_unk0x0052a86c = (short)((int)(pMenu->items[3].max * 0x11c) / (pMenu->items[3].min - 1));
    g_unk0x0052aa54 = (int)(pMenu->items[0].max * 0xcccd) / (pMenu->items[0].min - 1) + 0x13333;
    g_unk0x0052aa58 = (int)(pMenu->items[1].max * 0x50000) / (pMenu->items[1].min - 1) + 0x50000;
}

bool FUN_004174d0(void);

// Callback of the sound options menu: applies the three volumes.
// FUNCTION: CMR2 0x00401380
void FUN_00401380(Menu *pMenu)
{
    int rate;

    CGameInfo::FUN_00405e10(pMenu->items[Menu_FindItem(pMenu, 0)].max * 10);
    CGameInfo::FUN_00405e50(pMenu->items[Menu_FindItem(pMenu, 1)].max * 10);
    if (FUN_004174d0())
        CGameInfo::FUN_00405e80(pMenu->items[Menu_FindItem(pMenu, 2)].max * 10);
    rate = (int)(CGameInfo::FUN_00405e70() << 16) / 100;
    CInput::FUN_0049ffc0(rate / 4);
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

BYTE FUN_0041b370(void);
void RallyData_FUN_00408990(BYTE index, BYTE *pValue);
void RallyData_MarkTyresChanged(int index);
int RallyData_FUN_00411880(void);

// Callback 0 of the sound options menu: loads the three volumes.
// FUNCTION: CMR2 0x004012d0
void FUN_004012d0(Menu *pMenu, int param)
{
    g_unk0x00529ecc = CGameInfo::FUN_00405e40();
    g_unk0x0052a488 = CGameInfo::FUN_00405e70();
    g_unk0x0052ad50 = CGameInfo::FUN_00405ea0();
    pMenu->items[Menu_FindItem(pMenu, 0)].max = g_unk0x00529ecc / 10;
    pMenu->items[Menu_FindItem(pMenu, 1)].max = g_unk0x0052a488 / 10;
    if (FUN_004174d0())
        pMenu->items[Menu_FindItem(pMenu, 2)].max = g_unk0x0052ad50 / 10;
}

// Callback 0 of the car setup menu: loads the four switches from the driver's
// setting byte (resetting them to on).
// FUNCTION: CMR2 0x00404130
void FUN_00404130(Menu *pMenu, int param)
{
    g_unk0x005298f8 = (g_unk0x005298f8 & 0xfc) | 0x3c;
    RallyData_FUN_00408990(FUN_0041b370() + g_unk0x0052af58[1], &g_unk0x005298f8);
    pMenu->items[0].max = (g_unk0x005298f8 >> 2) & 1;
    pMenu->items[1].max = (g_unk0x005298f8 >> 3) & 1;
    pMenu->items[2].max = (g_unk0x005298f8 >> 4) & 1;
    pMenu->items[3].max = (g_unk0x005298f8 >> 5) & 1;
    if (RallyData_FUN_00411880() == 0) {
        if (FUN_004174d0())
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 2;
    } else if (FUN_004174d0()) {
        pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
    }
}

// Callback of the car setup menu: encodes the selected switches and tyres.
// TODO: CMR2 0x00404d00 (implemented, match 71%)
void FUN_00404d00(Menu *pMenu)
{
    g_unk0x005298f8 = (g_unk0x005298f8 & ~4) | ((pMenu->items[0].max & 1) << 2);
    g_unk0x005298f8 = (g_unk0x005298f8 & ~8) | ((pMenu->items[1].max & 1) << 3);
    g_unk0x005298f8 = (g_unk0x005298f8 & ~0x20) | ((pMenu->items[3].max & 1) << 5);

    if (FUN_004174d0()) {
        if (RallyData_FUN_00411880()) {
            if (pMenu->items[Menu_FindItem(pMenu, 4)].max == 0)
                g_unk0x005298f8 = (g_unk0x005298f8 & ~1) | 2;
            else
                g_unk0x005298f8 &= ~3;
        } else {
            BYTE bits = 0xfe - pMenu->items[Menu_FindItem(pMenu, 4)].max;
            g_unk0x005298f8 ^= (bits ^ g_unk0x005298f8) & 3;
        }
    }

    if (RallyData_FUN_00411880()) {
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
    RallyData_FUN_00408990(FUN_0041b370() + g_unk0x0052af58[1], &g_unk0x005298f8);
}

// Callback 1 of the car setup menu: stores the setting byte (or the other
// one when cancelling) and applies the switches.
// FUNCTION: CMR2 0x00404c50
void FUN_00404c50(Menu *pMenu, char cancel)
{
    if (cancel != 0) {
        RallyData_FUN_00408990(FUN_0041b370() + g_unk0x0052af58[1], &g_unk0x0052a850);
        return;
    }
    RallyData_FUN_00408990(FUN_0041b370() + g_unk0x0052af58[1], &g_unk0x005298f8);
    CGameInfo::FUN_00405f40((g_unk0x005298f8 >> 2) & 1);
    CGameInfo::FUN_00405f20((g_unk0x005298f8 >> 3) & 1);
    CGameInfo::FUN_00405f60((g_unk0x005298f8 >> 4) & 1);
    CGameInfo::FUN_00405f80((g_unk0x005298f8 >> 5) & 1);
    CGameInfo::FUN_00405f00(g_unk0x005298f8 & 3);
    RallyData_MarkTyresChanged((FUN_0041b370() & 0xff) + g_unk0x0052af58[1]);
}

// Callback 2 of the network options menu.
// FUNCTION: CMR2 0x00403880
void FUN_00403880(Menu *pMenu)
{
    FUN_00403200(pMenu);
}

void FUN_00427c10(void);

// Callback 1 of the sound options menu: applies the volumes (or restores the
// ones the menu started with when cancelled).
// TODO: CMR2 0x00401420 (implemented, match 77%)
void FUN_00401420(Menu *pMenu, char cancel)
{
    int rate;

    if (cancel == 0) {
        CGameInfo::FUN_00405e10(pMenu->items[Menu_FindItem(pMenu, 0)].max * 10);
        CGameInfo::FUN_00405e50(pMenu->items[Menu_FindItem(pMenu, 1)].max * 10);
        if (FUN_004174d0())
            CGameInfo::FUN_00405e80(pMenu->items[Menu_FindItem(pMenu, 2)].max * 10);
    } else {
        CGameInfo::FUN_00405e10(g_unk0x00529ecc);
        CGameInfo::FUN_00405e50(g_unk0x0052a488);
        CGameInfo::FUN_00405e80(g_unk0x0052ad50);
    }
    rate = (int)(CGameInfo::FUN_00405e70() << 16) / 100;
    CInput::FUN_0049ffc0(rate / 4);
    FUN_00427c10();
}

BYTE *RallyData_FUN_00408a00(BYTE index);

// Callback 0 of the car setup menu: loads the switches from the driver's
// setting byte (keeping a copy to restore on cancel).
// FUNCTION: CMR2 0x00404b80
void FUN_00404b80(Menu *pMenu, int param)
{
    g_unk0x0052a850 = *RallyData_FUN_00408a00(FUN_0041b370() + g_unk0x0052af58[1]);
    g_unk0x005298f8 = *RallyData_FUN_00408a00(FUN_0041b370() + g_unk0x0052af58[1]);
    pMenu->items[0].max = (g_unk0x005298f8 >> 2) & 1;
    pMenu->items[1].max = (g_unk0x005298f8 >> 3) & 1;
    pMenu->items[2].max = (g_unk0x005298f8 >> 4) & 1;
    pMenu->items[3].max = (g_unk0x005298f8 >> 5) & 1;
    if (FUN_004174d0()) {
        if (RallyData_FUN_00411880()) {
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

void FUN_00447d20(unsigned int index, FixVector *pOffset);
void FUN_00447e20(unsigned int index, short value);
void FUN_00447ec0(unsigned int index, int value);
void RallyData_FUN_00408bd0(int *pPos, short heading, int value, BYTE index);

// Callback 1 of the camera options menu: applies the chosen offset (and
// stores it for the driver), or restores the old one when cancelled.
// TODO: CMR2 0x004037c0 (implemented, match 77%)
void FUN_004037c0(Menu *pMenu, char cancel)
{
    if (cancel != 0) {
        FUN_00447d20(g_unk0x0052af58[1], &g_unk0x0052af60);
        FUN_00447ec0(g_unk0x0052af58[1], g_unk0x00529914);
        FUN_00447e20(g_unk0x0052af58[1], g_unk0x0052a48c);
        return;
    }
    FUN_00447ec0(g_unk0x0052af58[1], g_unk0x0052aa5c);
    FUN_00447d20(g_unk0x0052af58[1], &g_unk0x0052aa50);
    FUN_00447e20(g_unk0x0052af58[1], g_unk0x0052a86c);
    CGameInfo::FUN_00405fa0((DWORD *)&g_unk0x0052aa50, g_unk0x0052a86c, g_unk0x0052aa5c);
    RallyData_FUN_00408bd0((int *)&g_unk0x0052aa50, g_unk0x0052a86c, g_unk0x0052aa5c,
                           (BYTE)((FUN_0041b370() & 0xff) + g_unk0x0052af58[1]));
}

// FUNCTION: CMR2 0x00404ea0
void FUN_00404ea0(BYTE param1)
{
    g_unk0x0052af58[0] = param1;
    CGameInfo::m_unk0x0052af40 = 1;
    g_unk0x0052ad54 = 0;
    CGameInfo::FUN_0049ea90(1);
    g_unk0x0052af41 = 1;
    g_pMenu0x0052af44 = NULL;
    g_pMenu0x0052af48 = &g_menu0x0052ad60;
    FUN_004a0ba0();
}

// FUNCTION: CMR2 0x00404f10
void FUN_00404f10(void)
{
    g_unk0x0052af41 = 0;
}

// FUNCTION: CMR2 0x00404f20
unsigned char CGameInfo::FUN_00404f20(void)
{
    return m_unk0x0052af40;
}

// FUNCTION: CMR2 0x00404f30
BYTE FUN_00404f30(void)
{
    return g_unk0x0052af41;
}

// FUNCTION: CMR2 0x004054a0
void FUN_004054a0(void)
{
    Menu_CallCallback2(g_pMenu0x0052af44);
}

// FUNCTION: CMR2 0x004054b0
int FUN_004054b0(unsigned int param1)
{
    if (CGameInfo::FUN_00404f20() && g_unk0x0052af58[1] == param1 && g_pMenu0x0052af44 == &g_menu0x0052aa70)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x004055e0
int FUN_004055e0(void)
{
    return g_unk0x0052aa60;
}

// FUNCTION: CMR2 0x004055f0
int FUN_004055f0(void)
{
    return g_unk0x0052aa68;
}

// FUNCTION: CMR2 0x00405600
int FUN_00405600(void)
{
    return g_unk0x0052af4c;
}

// FUNCTION: CMR2 0x00405610
int FUN_00405610(void)
{
    return g_unk0x0052af50;
}

// Text buffer for the truncated draw below (256 bytes up to the next global).
// GLOBAL: CMR2 0x0082b1c0
char g_unk0x0082b1c0[0x100];

// Draws the record's text; in mode 1 it truncates the string at the 16.16
// fraction of its length and draws the remainder separately.
// FUNCTION: CMR2 0x00501f80
void FUN_00501f80(int index, int font1, int font2, char *text, int x, int y,
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
        count = FixMulShift32(pRec->field_0x0, len << 16);
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
    edge[3] = pRect[3] + 1;
    edge[2] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
    edge[0] = pRect[0];
    edge[1] = pRect[3] + pRect[1];
    edge[2] = pRect[2];
    edge[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
    edge[0] = pRect[0];
    edge[1] = pRect[1];
    edge[3] = pRect[3];
    edge[2] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 2);
}

void FUN_00423010(int view, int start);
extern BYTE g_unk0x0052af58[2];

// TODO: CMR2 0x00404ef0 (implemented, match 70%)
void FUN_00404ef0(void)
{
    CGameInfo::FUN_0049ea90(0);
    FUN_00423010(g_unk0x0052af58[1], 0);
    CGameInfo::m_unk0x0052af40 = 0;
}

// Callback 0 of the camera options menu: turns the current values into the
// four slider positions.
// TODO: CMR2 0x00403110 (implemented, match 45%)
void FUN_00403110(Menu *pMenu)
{
    int distance = (pMenu->items[2].min - 1) * (g_unk0x0052aa5c - 0x80000);
    int angle = (pMenu->items[3].min - 1) * g_unk0x0052a86c;
    int height = (pMenu->items[0].min - 1) * (g_unk0x0052aa54 - 0x13333);
    int depth = (pMenu->items[1].min - 1) * (g_unk0x0052aa58 - 0x50000);

    pMenu->items[2].max = (BYTE)((unsigned int)(FixDiv(distance, 0x80000) + 0x8000) >> 16);
    pMenu->items[3].max = (BYTE)((unsigned int)(FixDiv(angle, 0x11c) + 0x8000) >> 16);
    pMenu->items[0].max = (BYTE)((unsigned int)(FixDiv(height, 0xcccd) + 0x8000) >> 16);
    pMenu->items[1].max = (BYTE)((unsigned int)(FixDiv(depth, 0x50000) + 0x8000) >> 16);
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

// Loads the in-race menu textures (arrows, curtain and, in championships,
// the round box).
// FUNCTION: CMR2 0x004054f0
void FUN_004054f0(void)
{
    bool loaded;

    sprintf(CFrontend::m_stringDest, g_strAr640ATga, CInstallInfo::GetGameCDPath());
    g_unk0x0052aa60 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest,
                                                     &loaded, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strAr640DTga, CInstallInfo::GetGameCDPath());
    g_unk0x0052aa68 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest,
                                                     &loaded, 0, 0, 0);
    if (CGameInfo::FUN_00405d80() == 4) {
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

void FUN_00447cf0(FixVector *pOut, unsigned int index);
int FUN_00447ea0(unsigned int index);
short FUN_00447e00(unsigned int index);
BYTE *RallyData_FUN_00408930(BYTE index);

// Callback 0 of the camera options menu: remembers the current camera and
// shows it on the sliders.
// TODO: CMR2 0x00403700 (implemented, match 77%)
void FUN_00403700(Menu *pMenu, char unused)
{
    BYTE index;

    memcpy(g_unk0x00529900, RallyData_FUN_00408930(FUN_0041b370() + g_unk0x0052af58[1]), sizeof(g_unk0x00529900));
    index = g_unk0x0052af58[1];
    memcpy(g_unk0x0052a858, g_unk0x00529900, sizeof(g_unk0x0052a858));
    FUN_00447cf0(&g_unk0x0052af60, index);
    g_unk0x00529914 = FUN_00447ea0(g_unk0x0052af58[1]);
    g_unk0x0052a48c = FUN_00447e00(g_unk0x0052af58[1]);
    g_unk0x0052aa50.x = g_unk0x0052af60.x;
    g_unk0x0052aa54 = g_unk0x0052af60.y;
    g_unk0x0052aa58 = g_unk0x0052af60.z;
    g_unk0x0052aa5c = g_unk0x00529914;
    g_unk0x0052a86c = g_unk0x0052a48c;
    FUN_00403110(pMenu);
    g_unk0x0052ad54 = 1;
}

// Item callback of the camera menu "default" item.
// TODO: CMR2 0x004036c0 (implemented, match 75%)
void FUN_004036c0(Menu *pMenu, char unused)
{
    g_unk0x0052aa5c = 0xe0000;
    g_unk0x0052a86c = 0x2d;
    g_unk0x0052aa54 = 0x18000;
    g_unk0x0052aa58 = 0x68000;
    FUN_00403110(pMenu);
}

// ===== Agent3 batch 1: GameInfo option records (>= 0x4f0000) =====

struct Unk0x0052ebc0;
extern struct Unk0x0052ebc0 *RallyData_FUN_00407610(int index);

// Draws the four one pixel edges of the option record's box (x, y, w, h);
// the vertical edges start one pixel inside the horizontal ones.
// TODO: CMR2 0x0050cb30 (implemented, match 82%)
void FUN_0050cb30(short *pRect, BYTE *pColour)
{
    short edge[4];

    edge[0] = pRect[0];
    edge[1] = pRect[1];
    edge[2] = pRect[2];
    edge[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[1] = pRect[3] + pRect[1] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[3] = pRect[3];
    edge[1] = pRect[1];
    edge[0] = pRect[0];
    edge[2] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
    edge[0] = pRect[2] + pRect[0] - 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, edge, pColour, 3);
}

// Returns the byte at column type of the 7-byte option record index,
// sign extended; 0 for an unknown column.
// FUNCTION: CMR2 0x00502c60
int FUN_00502c60(int index, int type)
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
void FUN_00501d50(int index, short *pBar, int direction, int unused)
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
// TODO: CMR2 0x005028a0 (implemented, match 83%)
int FUN_005028a0(int index, int type)
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
int FUN_004ff550(void)
{
    int i;
    int found;

    found = 0;
    for (i = 0; i < (char)FUN_00502500()[6]; i++) {
        if (g_unk0x0082a90c[i] != 3) {
            found = 1;
            break;
        }
    }
    if (found) {
        if (CGameInfo::FUN_00405d80() != 0) {
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
// TODO: CMR2 0x005020a0 (implemented, match 77%)
void FUN_005020a0(int index, int font1, int font2, char *text, int x, int y,
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
            width = Font_GetTextWidth(font1, (BYTE *)g_unk0x0082b1c0);
            for (i = count, j = 0; i < len; i++, j++)
                g_unk0x0082b1c0[j] = text[i];
            g_unk0x0082b1c0[len - count] = 0;
            Font_DrawText(font2, g_unk0x0082b1c0, width + x, y, pColour2, flags);
        }
    }
}

// Option record value as a percentage 0..100: picks the byte selected by
// type, letting a byte that follows it win when it is larger, and a later
// byte win when the value is still smaller.
// FUNCTION: CMR2 0x00502df0
unsigned int FUN_00502df0(int index, int type, int dynamic)
{
    BYTE *p;
    int value;
    int other;

    value = 0;
    if (dynamic != 0)
        p = (BYTE *)RallyData_FUN_00407610(index);
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
void FUN_00501f00(int index, short *pBar)
{
    Unk0x0082b2c0 *p = &g_unk0x0082b2c0[index];

    if (p->field_0xc == 1) {
        pBar[2] = (short)FixMulShift32(pBar[2] << 16, p->field_0x0);
        pBar[3] = (short)FixMulShift32(pBar[3] << 16, p->field_0x0);
        return;
    }
    if (p->field_0xc == 0) {
        pBar[3] = 0;
        pBar[2] = 0;
    }
}

// Address of the byte set by the option menu (0x82b1b8).
// FUNCTION: CMR2 0x00501ab0
BYTE *FUN_00501ab0(void)
{
    return &g_unk0x0082b1b8;
}

// True when the option slot is enabled: mode 2/4 are checked against their
// selectors, everything else is accepted.
// FUNCTION: CMR2 0x00501280
int FUN_00501280(int mode, int index)
{
    int result = 1;

    if (mode == 2) {
        if (CFrontend::FUN_0040ee80(index) == 0)
            result = 0;
    } else if (mode == 4) {
        if (CFrontend::FUN_0040ee70(index) == 0)
            result = 0;
    }
    return result;
}

// Whether the option slot has been raised above its base value.
// FUNCTION: CMR2 0x00502630
int FUN_00502630(int index, int type)
{
    int value;

    value = FUN_005011f0(index);
    if (g_unk0x00527098[type] <= value)
        return 1;
    return g_unk0x0082bf20[index][type] != 0;
}

// Whether the option slot value differs from its default, per column.
// FUNCTION: CMR2 0x00502b10
int FUN_00502b10(int index, int type)
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
int FUN_00502fc0(int index, int type)
{
    int value;

    value = FUN_005011f0(index);
    if (g_unk0x005270b4[type] <= value)
        return FUN_00502df0(index, type, 1) != 0;
    return g_unk0x0082c040[index][type] != 0;
}

// Finds the index of the entry with the given id in the list at 0x3c, or -1.
// FUNCTION: CMR2 0x00508f60
int FUN_00508f60(unsigned int id, int param2)
{
    int *pEntry;
    int count;
    int i;

    count = *(char *)(param2 + 0x26a);
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
// TODO: CMR2 0x00503b70 (implemented, match 65%)
int FUN_00503b70(Unk0x0082c6c8 *p, short *pX, short *pY)
{
    if (p->field_0x1c != 0x10000) {
        *pX = (short)FixMulShift32(FixDiv((*pX - g_unk0x0082c9ec[0]) << 16, g_unk0x0082c9ec[2] << 16),
                                   *(short *)((BYTE *)p + 0x10) << 16) + *(short *)((BYTE *)p + 0xc);
        *pY = (short)FixMulShift32(FixDiv((*pY - g_unk0x0082c9ec[1]) << 16, g_unk0x0082c9ec[3] << 16),
                                   *(short *)((BYTE *)p + 0x12) << 16) + *(short *)((BYTE *)p + 0xe);
    }
    *pX = (short)((*pX * (int)g_pGraphics->resX) / 0x280);
    *pY = (short)((*pY * (int)g_pGraphics->resY) / 0x1e0);
    return 0;
}

// Re-runs the option-menu callback stored in the global (0x82b1b4).
// FUNCTION: CMR2 0x00501390
BYTE FUN_00501390(void)
{
    SceneNode_Destroy((SceneNode *)g_unk0x0082b1b4);
    return 1;
}
// Releases the three option menu textures and clears their handles.
// FUNCTION: CMR2 0x0050f480
int FUN_0050f480(void)
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
// TODO: CMR2 0x00509d30 (implemented, match 86%)
void FUN_00509d30(int index)
{
    BYTE *p;
    int *pOut;
    int i;

    pOut = g_unk0x0082d220[index].field_0x23c.v;
    p = (BYTE *)RallyData_FUN_00407610(index);
    for (i = 0; i < 4; i++)
        pOut[i] = FixDiv(p[0x108 + i] << 16, 0xff0000);
}

// Sets the fade/shape values of the option record's sky colours (field_0x22c).
// TODO: CMR2 0x00509be0 (implemented, match 81%)
void FUN_00509be0(int index)
{
    BYTE *p;
    int *pColour;
    int i;

    pColour = g_unk0x0082d220[index].field_0x22c.v;
    p = (BYTE *)RallyData_FUN_00407610(index);
    for (i = 0; i < 4; i++)
        pColour[i] = FixDiv(p[0x10e + i] << 16, 0xff0000);
    pColour[0] = FixMul(pColour[0], FixMul(0x3333, 0xffff0000));
    pColour[1] = FixMul(pColour[1], FixMul(0x3333, 0xffff0000));
    pColour[2] = FixMul(pColour[2], FixMul(0x3333, 0x8000));
    pColour[3] = FixMul(pColour[3], FixMul(0x3333, 0xffff8000));
}

// Applies the option menu's fade to the stage meshes of the given category:
// each mesh in the record's list takes the target value for its bit.
void FUN_0049c440(Mesh *pMesh, int mask, int value);
void FUN_0049c4b0(Mesh *pMesh, int mask, int value);
// TODO: CMR2 0x00508fa0 (implemented, match 84%)
void FUN_00508fa0(int index, int param2, BYTE param3){
    BYTE *pRecord = (BYTE *)&g_unk0x0082d220[index];
    int target;
    int mask;
    int i;

    if (param2 == 0) {
        target = 0;
        mask = 1;
    } else if (param2 == 1 || param3 == 4 || param3 == 5) {
        target = 3;
        mask = 4;
    } else {
        target = 5;
        mask = 7;
    }
    switch (param3) {
    case 0:
        i = FUN_00508f60(7, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 0x100, target);
            FUN_0049c4b0(*(Mesh **)(pRecord + i * 4), 0x100, mask);
            return;
        }
        break;
    case 1:
        i = FUN_00508f60(0xc, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 0x20, target);
            FUN_0049c4b0(*(Mesh **)(pRecord + i * 4), 0x20, mask);
        }
        i = FUN_00508f60(7, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 0x20, target);
            FUN_0049c4b0(*(Mesh **)(pRecord + i * 4), 0x20, mask);
            return;
        }
        break;
    case 2:
        i = FUN_00508f60(7, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 0x40, target);
            FUN_0049c4b0(*(Mesh **)(pRecord + i * 4), 0x40, mask);
            return;
        }
        break;
    case 3:
        i = FUN_00508f60(7, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 0x80, target);
            FUN_0049c4b0(*(Mesh **)(pRecord + i * 4), 0x80, mask);
            return;
        }
        break;
    case 4:
        i = FUN_00508f60(0xe, (int)pRecord);
        if (i >= 0) {
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 4, target);
            return;
        }
        break;
    case 5:
        i = FUN_00508f60(0xe, (int)pRecord);
        if (i >= 0)
            FUN_0049c440(*(Mesh **)(pRecord + i * 4), 8, target);
        break;
    }
}

// Chooses the highlight value for the selected option item and stores it at
// +0x1e, together with the option's current value at +0x1f.
// (CFrontend::FUN_004a3d80)

// GLOBAL: CMR2 0x00831880
int g_unk0x00831880;
// GLOBAL: CMR2 0x00831884
BYTE g_unk0x00831884;

// TODO: CMR2 0x004ff5b0 (implemented, match 62%)
void FUN_004ff5b0(void)
{
    BYTE *pMode;
    int index;
    BYTE value;

    index = Menu_FindItem((Menu *)FUN_00502500(), 1);
    pMode = FUN_00502500();
    value = pMode[0x1f + index * 0x14];
    switch (value) {
    case 0:
        FUN_00502510()[0x1e] = 7;
        break;
    case 1:
        FUN_00502510()[0x1e] = 5;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        FUN_00502510()[0x1e] = 0xb;
        break;
    }
    FUN_00502510()[0x1f] = (BYTE)FUN_005028a0(CGameInfo::FUN_005011b0(), value);
}

// Frees the option menu sound buffer and stops the streaming sound.
// TODO: CMR2 0x0050f340 (implemented, match 34%)
int FUN_0050f340(void)
{
    if (g_unk0x00831880 != 0) {
        if (g_unk0x00831884 == 0)
            CFileBuffer::FreeGenericFileBuffer((void *)g_unk0x00831880);
        g_unk0x00831880 = 0;
    }
    CFrontend::FUN_004a3d80();
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
void FUN_00502230(int unused, int unused2)
{
    g_unk0x0082ac60 = 1;
}

// Marks the given item selected and requests a refresh.
// FUNCTION: CMR2 0x004ffa50
void FUN_004ffa50(char *pItem, int unused)
{
    pItem[7] = 1;
    g_unk0x0082ac60 = 1;
}

// Restarts the option records and arms their countdown.
// FUNCTION: CMR2 0x00500110
void FUN_00500110(int unused, int unused2)
{
    FUN_005021e0();
    g_unk0x0082ac4c = 1;
}

// Rebuilds the option records once the refresh request is consumed.
// FUNCTION: CMR2 0x004ffa70
void FUN_004ffa70(int unused, int unused2)
{
    g_unk0x0082aa40 = FUN_004f4db0();
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
void FUN_00500190(BYTE *pItem, int unused)
{
    int index;

    index = Menu_FindItem((Menu *)pItem, 5);
    if (pItem[index * 0x14 + 0x1f] == 0) {
        FUN_005021e0();
        g_unk0x0082ac50 = 1;
    }
}

// Draws the option menu's status line centred on the screen.
// FUNCTION: CMR2 0x0050e740
void FUN_0050e740(int unused)
{
    FUN_00501f80(4, 0, 0, g_unk0x0082a93c, (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                 g_unk0x00527380, g_unk0x0052738c, 0x12);
}

// Fills pRect with `count` vertical strips shading from colour 0 to colour 1
// over the first half and from colour 1 to colour 2 over the second half
// (pColours: three RGBA colours; count must be at least 2). The strip widths
// are rounded so that together they cover the rectangle exactly.
// TODO: CMR2 0x0050ee50 (implemented, match 41%)
void FUN_0050ee50(short *pRect, int unused, unsigned int count, BYTE *pColours)
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
    d12.z = (pColours[10] << 16) - c1.z;
    half = (count & 0xff) >> 1;
    n = count & 0xff;
    for (i = 0, t = 0; i < n; i++, t += 0x10000) {
        strip[2] = FixMulShift32(t + 0x10000, FixDiv(pRect[2] << 16, n << 16)) + pRect[0] - strip[0];
        if (i <= half) {
            FixVecScale(&c, &d01, FixDiv(i << 16, half << 16));
            c.x += c0.x;
            c.y += c0.y;
            c.z += c0.z;
        } else {
            FixVecScale(&c, &d12, FixDiv(t - (half << 16), half << 16));
            c.x += c1.x;
            c.y += c1.y;
            c.z += c1.z;
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

BYTE *RallyData_FUN_00407630(int index);

// GLOBAL: CMR2 0x00529670
char g_strFontGeneralHel15pt[17] = "general\\hel_15pt";
// GLOBAL: CMR2 0x00529684
char g_strFontGeneralHel12pt[17] = "general\\hel_12pt";
// GLOBAL: CMR2 0x00529698
char g_strFontGeneralDot[12] = "general\\dot";
// GLOBAL: CMR2 0x005296ac
char g_strFontGeneralHandel[15] = "general\\handel";

// Starts the fade of the option menu once more than 50 frames have passed.
// FUNCTION: CMR2 0x00501350
void FUN_00501350(int param1, int unused)
{
    unsigned int delta = CMain::GetFrameDelta() - g_unk0x0082b0a4;

    if (delta > 0x32) {
        if (g_unk0x0082a938 != 0) {
            CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, 0, 2, 2);
            return;
        }
        CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, 0, 0, 2);
    }
}

// Copies the option records back into the rally data (undo of FUN_00502d50).
// FUNCTION: CMR2 0x00502db0
void FUN_00502db0(void)
{
    int *pSrc;
    int *pDst;
    int i;
    int j;

    i = 0;
    if ((char)CGameInfo::FUN_00405d70() != 0) {
        pSrc = (int *)g_unk0x0082c070;
        do {
            pDst = (int *)RallyData_FUN_00407610(i);
            i++;
            memcpy(pDst, pSrc, 0x148);
            pSrc += 0x52;
        } while (i < (int)(CGameInfo::FUN_00405d70() & 0xff));
    }
}

// Copies the default option values from the global table into each record.
// TODO: CMR2 0x005029b0 (implemented, match 55%)
void FUN_005029b0(void)
{
    BYTE *pDst;
    char *pSrc;
    int i;

    i = 0;
    pSrc = (char *)&g_unk0x0082bee8[0][5];
    do {
        pDst = (BYTE *)RallyData_FUN_00407630(i);
        pDst[4] = pSrc[-1];
        pDst[5] = pSrc[0];
        pDst[1] = pSrc[-4];
        pDst[6] = pSrc[1];
        pDst[3] = pSrc[-2];
        pDst[2] = pSrc[-3];
        i++;
        pDst[0] = pSrc[-5];
        pSrc += 7;
    } while ((int)pSrc < 0x82bf09);
}

// Applies the selected option: advances the menu when its value is set, or
// starts the fade otherwise.
// TODO: CMR2 0x005000b0 (implemented, match 54%)
void FUN_005000b0(int unused, int unused2)
{
    BYTE *pMode;
    int index;
    BYTE value;

    FUN_004ff5b0();
    index = Menu_FindItem((Menu *)FUN_00502500(), 1);
    pMode = FUN_00502500();
    value = pMode[0x1f + index * 0x14];
    if (FUN_00502630(CGameInfo::FUN_005011b0(), value) != 0) {
        Menu_SetNextAction((int)FUN_00502510());
        return;
    }
    CGameInfo::FUN_00500500();
}

// Loads the four option menu fonts.
// FUNCTION: CMR2 0x0050f370
void FUN_0050f370(void)
{
    Font_InitTable(4);
    Font_Load(g_strFontGeneralHel15pt, (GenericFile *)FUN_0050f620(), 0);
    Font_Load(g_strFontGeneralHel12pt, (GenericFile *)FUN_0050f620(), 1);
    Font_Load(g_strFontGeneralDot, (GenericFile *)FUN_0050f620(), 2);
    Font_Load(g_strFontGeneralHandel, (GenericFile *)FUN_0050f620(), 3);
}

