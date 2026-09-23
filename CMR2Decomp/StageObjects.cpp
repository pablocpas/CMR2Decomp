#include <windows.h>
#include <string.h>

// Accessors of the stage object tables (0x460bf0-0x4789b0)

extern void *g_unk0x00547ac8;
extern void *g_unk0x00543ecc;
extern int g_unk0x0058cf7c;
extern BYTE *g_unk0x0058c94c;
extern unsigned int g_unk0x0058ca6c;

int FUN_0046d2a0(int *p);

// GLOBAL: CMR2 0x00543f28
int g_unk0x00543f28[8 * 4];
// GLOBAL: CMR2 0x00547b80
BYTE g_unk0x00547b80;
// GLOBAL: CMR2 0x00548110
BYTE g_unk0x00548110[8][8];
// GLOBAL: CMR2 0x00588758
int *g_unk0x00588758;
// GLOBAL: CMR2 0x00588760
char g_unk0x00588760;
// GLOBAL: CMR2 0x00588761
BYTE g_unk0x00588761;
// GLOBAL: CMR2 0x00588864
int g_unk0x00588864;
// GLOBAL: CMR2 0x00588868
int g_unk0x00588868;
// GLOBAL: CMR2 0x00588970
int g_unk0x00588970[16];
// GLOBAL: CMR2 0x00588ba4
BYTE g_unk0x00588ba4[16];
// GLOBAL: CMR2 0x00588bb4
int g_unk0x00588bb4[16];
// GLOBAL: CMR2 0x00588cd4
int g_unk0x00588cd4[16 * 2];
// GLOBAL: CMR2 0x00588d38
int g_unk0x00588d38;
// GLOBAL: CMR2 0x00589438
int g_unk0x00589438;
// GLOBAL: CMR2 0x0058943c
int g_unk0x0058943c;
// GLOBAL: CMR2 0x00589440
int g_unk0x00589440;
// GLOBAL: CMR2 0x00589444
int g_unk0x00589444;
// GLOBAL: CMR2 0x0058cf68
int g_unk0x0058cf68;
// GLOBAL: CMR2 0x0058cf80
BYTE g_unk0x0058cf80[0x100];
// GLOBAL: CMR2 0x0058d2a0
int g_unk0x0058d2a0[12];
// GLOBAL: CMR2 0x0058d3b0
int g_unk0x0058d3b0[64];
// GLOBAL: CMR2 0x0058d6a8
int g_unk0x0058d6a8[2];
// GLOBAL: CMR2 0x0058d6b0
int g_unk0x0058d6b0[7];
// GLOBAL: CMR2 0x0058da30
int g_unk0x0058da30[8];

struct Block0x309 { int data[0x309]; };
struct Block0x134 { int data[0x134]; };
struct Block6 { int data[6]; };

// FUNCTION: CMR2 0x00460bf0
BYTE FUN_00460bf0(int index)
{
    return *((BYTE *)g_unk0x00547ac8 + index * 0x178);
}

// FUNCTION: CMR2 0x00460c10
int FUN_00460c10(int index)
{
    return *(int *)((BYTE *)g_unk0x00547ac8 + 0x54 + index * 0x178);
}

// FUNCTION: CMR2 0x00460c80
int FUN_00460c80(BYTE *pCar)
{
    return *(int *)((BYTE *)g_unk0x00543ecc + 8 + (signed char)pCar[0xb1a] * 0xc);
}

// FUNCTION: CMR2 0x00463270
int *FUN_00463270(int i, int j)
{
    return &g_unk0x00543f28[j + i * 4];
}

// FUNCTION: CMR2 0x00463ce0
void FUN_00463ce0(BYTE value)
{
    g_unk0x00547b80 = value;
    if (value > 7)
        g_unk0x00547b80 = 6;
}

// FUNCTION: CMR2 0x00464af0
BYTE *FUN_00464af0(int index)
{
    return g_unk0x00548110[index];
}

// FUNCTION: CMR2 0x00465ea0
int FUN_00465ea0(int value)
{
    if (value < 0)
        return 0;
    if (value > 0xff)
        value = 0xff;
    return value;
}

// FUNCTION: CMR2 0x00465f80
void FUN_00465f80(void)
{
    g_unk0x00588760 = 0xff;
    g_unk0x00588864 = -1;
}

// FUNCTION: CMR2 0x00466080
void FUN_00466080(void)
{
    FUN_0046d2a0(g_unk0x00588758);
}

// FUNCTION: CMR2 0x00466090
int FUN_00466090(void)
{
    if (g_unk0x00588758 != NULL)
        return g_unk0x00588758[1];
    return 0;
}

// FUNCTION: CMR2 0x004660e0
void FUN_004660e0(BYTE value)
{
    g_unk0x00588761 = value;
}

// FUNCTION: CMR2 0x004660f0
int FUN_004660f0(void)
{
    return g_unk0x00588760;
}

// FUNCTION: CMR2 0x0046b400
void FUN_0046b400(int value, int index)
{
    g_unk0x00588970[index] = value;
}

// FUNCTION: CMR2 0x0046b4c0
int FUN_0046b4c0(BYTE *pCar)
{
    return g_unk0x00588970[(signed char)pCar[0xb1a]];
}

// FUNCTION: CMR2 0x0046b740
void FUN_0046b740(int i, int value, int j)
{
    g_unk0x00588cd4[j + i * 2] = value;
}

// FUNCTION: CMR2 0x0046b760
void FUN_0046b760(int index, int reset)
{
    if (reset != 0) {
        g_unk0x00588bb4[index] = 0;
        return;
    }
    g_unk0x00588bb4[index] = 1;
}

// FUNCTION: CMR2 0x0046bd20
int FUN_0046bd20(int i, int j)
{
    return g_unk0x00588cd4[j + i * 2];
}

// FUNCTION: CMR2 0x0046bd40
BYTE FUN_0046bd40(int index)
{
    return g_unk0x00588ba4[index];
}

// FUNCTION: CMR2 0x0046bfb0
void FUN_0046bfb0(Block0x309 *pSrc, Block0x309 *pDst)
{
    *pDst = *pSrc;
}

// FUNCTION: CMR2 0x0046c180
void FUN_0046c180(Block0x134 *pSrc, Block0x134 *pDst)
{
    *pDst = *pSrc;
}

// FUNCTION: CMR2 0x0046c220
void FUN_0046c220(Block6 *pSrc, Block6 *pDst)
{
    *pDst = *pSrc;
}

// FUNCTION: CMR2 0x0046d2a0
int FUN_0046d2a0(int *p)
{
    if (p != NULL && p[1] != 0) {
        p[1] = 0;
        p[2] = 0;
        p[5] = 0;
        return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x0046d500
int FUN_0046d500(void)
{
    return g_unk0x00588d38;
}

// FUNCTION: CMR2 0x0046f4c0
void FUN_0046f4c0(int *pOut)
{
    *pOut = g_unk0x00589438;
}

// FUNCTION: CMR2 0x0046f4d0
void FUN_0046f4d0(int *pOut)
{
    *pOut = g_unk0x0058943c;
}

// FUNCTION: CMR2 0x0046f4e0
void FUN_0046f4e0(int *pOut1, int *pOut2)
{
    *pOut1 = g_unk0x00589440;
    *pOut2 = g_unk0x00589444;
}

// FUNCTION: CMR2 0x00471bd0
unsigned int FUN_00471bd0(BYTE **pOut)
{
    *pOut = g_unk0x0058c94c;
    return g_unk0x0058ca6c;
}

// FUNCTION: CMR2 0x004728b0
void FUN_004728b0(void)
{
    g_unk0x0058cf68 = 1;
}

// FUNCTION: CMR2 0x004728c0
void FUN_004728c0(void)
{
    g_unk0x0058cf68 = 0;
}

// FUNCTION: CMR2 0x004728d0
int FUN_004728d0(void)
{
    return g_unk0x0058cf68;
}

// FUNCTION: CMR2 0x00472ca0
void FUN_00472ca0(void)
{
    g_unk0x0058cf7c = 0;
}

// FUNCTION: CMR2 0x00473680
int FUN_00473680(unsigned int *p)
{
    if ((*p & 0x1f) != 0x1f && (*p & 0x3e0) != 0x3e0)
        return 0;
    return 1;
}

// FUNCTION: CMR2 0x00475f70
BYTE *FUN_00475f70(void)
{
    return g_unk0x0058cf80;
}

// FUNCTION: CMR2 0x004764e0
void FUN_004764e0(BYTE *p)
{
    g_unk0x0058d3b0[p[2]] = 1;
}

// FUNCTION: CMR2 0x00476520
int FUN_00476520(BYTE index)
{
    return g_unk0x0058d6a8[index];
}

// FUNCTION: CMR2 0x00477a90
void FUN_00477a90(void)
{
    memset(g_unk0x0058d6b0, 0xff, sizeof(g_unk0x0058d6b0));
    memset(g_unk0x0058d2a0, 0xff, sizeof(g_unk0x0058d2a0));
}

// FUNCTION: CMR2 0x00478170
void FUN_00478170(int index)
{
    g_unk0x0058da30[index] = 0;
}

// FUNCTION: CMR2 0x00478190
void FUN_00478190(int index, int seconds)
{
    g_unk0x0058da30[index] += seconds * 100;
}

// FUNCTION: CMR2 0x004781c0
int FUN_004781c0(int index)
{
    return g_unk0x0058da30[index];
}

// FUNCTION: CMR2 0x004789b0
void FUN_004789b0(BYTE *pCar)
{
    *(int *)(pCar + 0xa74) = *(int *)(pCar + 0xa78);
}
