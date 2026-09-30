#ifndef _GAME_INFO_H
#define _GAME_INFO_H

#include "FixedPoint.h"
#include <windows.h>

struct GameInfo0xa4SubStruct8 {
	char ident[4];
	unsigned int value;
};

struct GameInfo0xa4SubStruct12 {
	char ident[4];
	unsigned int flags;
	unsigned int value;
};

// and this is actually meant to be 0x12c4 (4804) bytes in size.
struct GameInfo0xa4
{
	GameInfo0xa4SubStruct12 firstLoop[15];
	GameInfo0xa4SubStruct12 secondLoop[120];
	GameInfo0xa4SubStruct8 rallyStageRecordTimes[88];
	short rallyStageRecordSplits[88][10];
	GameInfo0xa4SubStruct12 thirdLoop[45];
	GameInfo0xa4SubStruct8 arcadeRecordTimes[9];
	short arcadeRecordSplits[9][6];
};

// looks like this should be 0x3958 (14680) bytes in size.
struct GameInfo
{
    unsigned char empty[16];
    int magicNumber;          /* Always 0x11 */
    unsigned int field_0x14; /* first byte of this contains language */
    unsigned int field_0x18;
    unsigned int field_0x1c;
    unsigned int field_0x20;
    int screenWidth;
    int screenHeight;
    int screenColourDepth;
    unsigned int unknownGraphicsOptions; // first byte is fullscreen
    unsigned int field_0x34;
    char graphicsCardName[80];
    unsigned int field_0x88;
    unsigned int field_0x8c;
    unsigned int field_0x90;
    WORD field_0x94;
    BYTE field_0x96_padding[2];
    short field_0x98;
    short field_0x9a;
    unsigned int field_0x9c;
    unsigned int field_0xa0;
    GameInfo0xa4 field_0xa4;
    GameInfo0xa4 field_0x1368;
    GameInfo0xa4 field_0x262c;
    unsigned int field_0x38f0;
    unsigned int field_0x38f4;
    char field_0x38f8[88];
    char field_0x3950[21];
    char field_0x3965[21];
    short field_0x397a;
    int field_0x397c;
    BYTE field_0x3980;
    BYTE field_0x3981_padding[3];
    int field_0x3984;
    int field_0x3988;
    int field_0x398c;
    int field_0x3990;
    BYTE field_0x3994_padding[5]; // seems wrong that this 5 bytes, but we're a byte short without it
};

struct Unk0x0059fa20 {
    DWORD field_0x0;
    DWORD field_0x4;
    BYTE field_0x10;
};

// 0x14-byte entry of the table at 0x82b2c0.
struct Unk0x0082b2c0 {
    int field_0x0;
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int field_0x10;
};

// 16.16 value animated between start and end at 50 units/s.
struct FixInterp {
    BYTE field_0x0[0x14];
    int start;              // 0x14
    int end;                // 0x18
    int current;            // 0x1c
    int distance;           // 0x20 |end - start| * 50
    BYTE field_0x24[0x18];
    unsigned int startTime; // 0x3c
    BYTE field_0x40[0xc];
    int active;             // 0x4c
};

void FixInterp_StartToOne(FixInterp *p);
void FixInterp_StartToZero(FixInterp *p);
BYTE *FUN_00502500(void);

class CGameInfo
{
public:
    static unsigned char GetGameLanguage(void);
    static unsigned int GetGameRegion(void);
    static void SetGameRegion(unsigned int region);
    static unsigned char FUN_00405d80(void);
    static unsigned char FUN_00405d90(void);
    static unsigned char FUN_00405d70(void);
    static void FUN_004f8a70(int index);
    static void FUN_005011a0(void);
    static int FUN_005011b0(void);
    static void FUN_00500500(void);
    static int FUN_005012c0(void);
    static void FUN_00501cc0(int index, int param2, int param3);
    static int FUN_005004c0(void);
    static int FUN_00501230(void);
    static int FUN_00505e10(BYTE param1);
    static void FUN_004a13b0(void);
    static void FUN_004a12d0(int param1);
    static void FUN_00505a60(int param1);
    static int FUN_0040a420(int index);
    static unsigned char FUN_00405da0(void);
    static unsigned char FUN_00405dc0(void);
    static unsigned char FUN_00405dd0(void);
    static unsigned char FUN_00405e00(void);
    static void FUN_00405e10(unsigned int param1);
    static unsigned int FUN_00405e40(void);
    static void FUN_00405e50(unsigned int param1);
    static unsigned int FUN_00405e70(void);
    static void FUN_00405e80(unsigned int param1);
    static unsigned int FUN_00405ea0(void);
    static unsigned int FUN_00405eb0(void);
    static void FUN_00405ec0(BYTE param1);
    static unsigned int FUN_00405ef0(void);
    static void FUN_00405f00(unsigned int param1);
    static void FUN_00405f20(BYTE param1);
    static void FUN_00405f40(BYTE param1);
    static void FUN_00405f60(BYTE param1);
    static void FUN_00405f80(BYTE param1);
    static void FUN_00405fa0(DWORD *param1, WORD param2, DWORD param3);
    static void FUN_00405fd0(unsigned int param1);
    static unsigned int *FUN_00405db0(void);
    static GameInfo0xa4 *FUN_00405fe0(void);
    static GameInfo0xa4 *FUN_00405ff0(int param1);
    static void SetFullscreen(BYTE fullscreen);
    static unsigned int FUN_00405b50(void);
    static void FUN_00405b60(unsigned int param1);
    static void FUN_00405b80(BYTE param1);
    static unsigned int FUN_00405ba0(void);
    static void FUN_00405bb0(BYTE param1);
    static void FUN_00405be0(unsigned int param1);
    static unsigned int FUN_00405c70(void);
    static void FUN_00405c80(unsigned int param1);
    static void FUN_00405cb0(unsigned int param1);
    static unsigned int FUN_00405cd0(void);
    static void FUN_00405ce0(unsigned int param1);
    static unsigned int FUN_00405d00(void);
    static void FUN_00405d20(unsigned int param1);
    static unsigned int FUN_00405d60(void);
    static unsigned int FUN_00405d10(void);
    static void FUN_00405d40(unsigned int param1);
    static unsigned int GetScreenWidth(void);
    static void SetScreenWidth(unsigned int width);
    static unsigned int GetScreenHeight(void);
    static void SetScreenHeight(unsigned int height);
    static unsigned int GetColourDepth(void);
    static void SetColourDepth(unsigned int depth);
    static char *GetGameRegionDirectory(void);
    static void FUN_004f4b40(void);
    static void FUN_00405de0(BYTE param1);
    static void FUN_00510410(void);
    static void FUN_00510570(void);
    static void FUN_00406010(GameInfo0xa4 *param1);
    static void FUN_00406560(void);
    static void FUN_00406580(void);
    static DWORD SetupInputs(int unused);
    static bool LoadGameInfo(void);
    static unsigned int IsFullscreen(void);
    static int FUN_00405ca0(void);
    static bool FUN_00406410(int param1);
    static bool FUN_004eac50(int param1);
    static void FUN_004d0590(BYTE param1);
    static void FUN_0049ea90(unsigned int param1);
    static void FUN_0049e930(unsigned int param1);
    static void FUN_004d05d0(void);
    static bool FUN_004d05a0(void);
    static void FUN_004a0c60(void);
    static unsigned int FUN_00405bd0(void);
    static unsigned int FUN_00405c00(void);
    static unsigned int FUN_00406310(void);
    static BYTE FUN_00406320(void);
    static void FUN_00406330(BYTE param1);
    static void FUN_00406340(BYTE param1);
    static unsigned int FUN_00406360(int param1);
    static void FUN_00406380(int param1, int param2);
    static int FUN_004063d0(int param1);
    static int FUN_004063f0(int param1);
    static unsigned int FUN_00406430(void);
    static unsigned int FUN_00406440(void);
    static unsigned int FUN_00406450(unsigned int **param1);
    static void FUN_00406470(void);
    static BYTE FUN_00406520(int param1, int param2);
    static void FUN_00406540(int param1, int param2, BYTE param3);
    static char *FUN_00406690(void);
    static void FUN_004066a0(char *name);
    static char *FUN_004066d0(void);
    static void FUN_004066e0(char *name);
    static unsigned char FUN_00404f20(void);

    // GLOBAL: CMR2 0x0052afa0
    static GameInfo m_gameInfo;

    // GLOBAL: CMR2 0x0052ea52
    static BYTE m_unk0x0052ea52;

    // cached copies of bitfield values, refreshed by FUN_00406470
    // GLOBAL: CMR2 0x0052af40
    static unsigned char m_unk0x0052af40;   // byte flag, siblings are the cached bitfields below
    // GLOBAL: CMR2 0x0052af80
    static unsigned int m_unk0x0052af80;
    // GLOBAL: CMR2 0x0052af84
    static unsigned int m_unk0x0052af84;
    // GLOBAL: CMR2 0x0052af88
    static unsigned int m_unk0x0052af88;
    // GLOBAL: CMR2 0x0052af8c
    static unsigned int m_unk0x0052af8c;
    // GLOBAL: CMR2 0x0052af94
    static unsigned int m_unk0x0052af94;
    // GLOBAL: CMR2 0x0052af98
    static unsigned int m_unk0x0052af98;
    // GLOBAL: CMR2 0x0052af9c
    static unsigned int m_unk0x0052af9c;
    // GLOBAL: CMR2 0x0052e93c
    static unsigned int m_unk0x0052e93c;
    // GLOBAL: CMR2 0x0052e940
    static unsigned int m_unk0x0052e940;
    // GLOBAL: CMR2 0x0052ea44
    static unsigned int m_unk0x0052ea44;
    // GLOBAL: CMR2 0x0052ea48
    static unsigned int m_unk0x0052ea48;

    // GLOBAL: CMR2 0x0052ea54
    static unsigned int m_gameRegion;

    // GLOBAL: CMR2 0x00516124
    static char *m_gameRegionStrings[4];

    // GLOBAL: CMR2 0x00817574
    static BYTE m_unk0x00817574;    

    // GLOBAL: CMR2 0x0081a754
    static int m_unk0x0081a754;

    // GLOBAL: CMR2 0x00516184
    static char m_stringCMR[4];

    // GLOBAL: CMR2 0x005250dc
    static char m_stringGameInfoRCF[32];

    // GLOBAL: CMR2 0x0059f8d0
    static unsigned int m_unk0x0059f8d0;

    // GLOBAL: CMR2 0x00520870
    static unsigned int m_unk0x00520870;

    // GLOBAL: CMR2 0x0081777c
    static void * m_unk0x0081777c;
    
    // GLOBAL: CMR2 0x00817678
    static BOOL m_unk0x00817678;

    // GLOBAL: CMR2 0x005a00b8
    static char m_unk0x005a00b8[0x104];     // session name
    // GLOBAL: CMR2 0x005a02c0
    static char m_unk0x005a02c0[0x104];     // session password
    // GLOBAL: CMR2 0x005a03c4
    static char m_sessionNames[20][0x104];
    // GLOBAL: CMR2 0x005a0060
    static BOOL m_unk0x005a0060;
    // GLOBAL: CMR2 0x005a1814
    static HRESULT m_unk0x005a1814;

    // GLOBAL: CMR2 0x005a01bc
    static BYTE m_unk0x005a01bc;
    // GLOBAL: CMR2 0x0059fa20
    static BOOL m_unk0x0059fa20[400];

};

// DPSESSIONDESC2 of the hosted/joined session (0x50 bytes, defined in Game.cpp);
// its lpszSessionName/lpszPassword fields are set to the name buffers below.
extern BYTE g_unk0x005a0068[0x50];
#define g_sessionNamePtr (*(LPVOID **)(g_unk0x005a0068 + 0x30))       // 0x5a0098
#define g_sessionPasswordPtr (*(LPVOID **)(g_unk0x005a0068 + 0x34))   // 0x5a009c

#endif
