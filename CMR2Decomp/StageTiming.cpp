#include "StageTiming.h"
#include "Stage.h"
#include "TimingUtils.h"
#include "RallyTiming.h"
#include "GameInfo.h"
#include "Game.h"
#include "FileBuffer.h"
#include "SceneNode.h"
#include "Mesh.h"
#include <string.h>

// GLOBAL: CMR2 0x00541f08
StageFile g_stageFiles[7];

// GLOBAL: CMR2 0x00541f78
char g_stageSplitUnk0x00541f78[10][2];

// GLOBAL: CMR2 0x00541f8c
int g_unk0x00541f8c;

// GLOBAL: CMR2 0x00541f98
int g_unk0x00541f98;

// GLOBAL: CMR2 0x00541f9c
char g_stageDriverSlot[16];

// 10 - split id, 16 - drivers
// GLOBAL: CMR2 0x00541fac
char g_stageSplitDriverIndices[10][16];

// GLOBAL: CMR2 0x0054204c
char g_stageSplitPositions[10][16];

// GLOBAL: CMR2 0x0054218c
char g_stageSplitDriverCount[12];

// GLOBAL: CMR2 0x00542418
short g_unk0x00542418;

// GLOBAL: CMR2 0x00542604
BYTE g_unk0x00542604;

// GLOBAL: CMR2 0x00542198
int g_stageSplitTimesRaw[10][16];
// GLOBAL: CMR2 0x005420ec
char g_stageSplitTimesRawDriverIx[10][16];

// FUNCTION: CMR2 0x00455cf0
int StageTiming_GetDriverIDForPosition(int positionIx)
{
    int iSplitIx = GetStageSplitCount();

    /* This isn't the driver name, just the index in the lookup table. It looks like
       driver ix 15 is the player */
    return g_stageSplitDriverIndices[iSplitIx][positionIx];
}

// FUNCTION: CMR2 0x00455d70
int StageTiming_GetTimeForPosition(int iPosition)
{
    int stageSplitCount;
    int iTime;

    if (iPosition >= 0 && iPosition <= 15)
    {
        stageSplitCount = GetStageSplitCount();
        iTime = g_stageSplitTimesRaw[stageSplitCount][g_stageSplitTimesRawDriverIx[stageSplitCount][iPosition]];
        return ConvertRawTimeToCentiseconds(iTime);
    }

    return -1;
}

// FUNCTION: CMR2 0x00455020
void StageTiming_FreeStageFile5(void)
{
    if (g_stageFiles[5].buffer != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_stageFiles[5].buffer);
    g_stageFiles[5].buffer = NULL;
    g_stageFiles[5].loaded = FALSE;
    g_stageFiles[5].size = 0;
}

// FUNCTION: CMR2 0x00455050
void StageTiming_FreeStageFile2(void)
{
    if (g_stageFiles[2].buffer != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_stageFiles[2].buffer);
    g_stageFiles[2].buffer = NULL;
    g_stageFiles[2].loaded = FALSE;
    g_stageFiles[2].size = 0;
}

// FUNCTION: CMR2 0x00455290
StageFile *StageTiming_GetStageFile0(void)
{
    return &g_stageFiles[0];
}

// FUNCTION: CMR2 0x004552a0
StageFile *StageTiming_GetStageFile1(void)
{
    return &g_stageFiles[1];
}

// FUNCTION: CMR2 0x004552b0
StageFile *StageTiming_GetStageFile2(void)
{
    return &g_stageFiles[2];
}

// FUNCTION: CMR2 0x004552c0
StageFile *StageTiming_GetStageFile3(void)
{
    return &g_stageFiles[3];
}

// FUNCTION: CMR2 0x004552d0
StageFile *StageTiming_GetStageFile4(void)
{
    return &g_stageFiles[4];
}

// FUNCTION: CMR2 0x004552e0
StageFile *StageTiming_GetStageFile5(void)
{
    return &g_stageFiles[5];
}

// FUNCTION: CMR2 0x004552f0
StageFile *StageTiming_GetStageFile6(void)
{
    return &g_stageFiles[6];
}

// FUNCTION: CMR2 0x00455460
int StageTiming_FUN_00455460(void)
{
    return g_unk0x00541f8c;
}

// FUNCTION: CMR2 0x00455610
void StageTiming_FUN_00455610(void)
{
    g_unk0x00542418 = 0;
}

// FUNCTION: CMR2 0x00455ab0
int StageTiming_FUN_00455ab0(int iSplit)
{
    return g_stageSplitUnk0x00541f78[iSplit][1];
}

// FUNCTION: CMR2 0x00455ac0
int StageTiming_FUN_00455ac0(int iSplit, int iIndex)
{
    return g_stageSplitUnk0x00541f78[iSplit][iIndex];
}

// FUNCTION: CMR2 0x00455ae0
BYTE StageTiming_FUN_00455ae0(void)
{
    return g_unk0x00542604;
}

// FUNCTION: CMR2 0x00455c70
int StageTiming_GetSplitDriverCount(int iSplit)
{
    return g_stageSplitDriverCount[iSplit];
}

// FUNCTION: CMR2 0x00455c80
int StageTiming_GetSplitPositionOfDriver(int iDriver, int iSplit)
{
    return g_stageSplitPositions[iSplit][g_stageDriverSlot[iDriver]];
}

// FUNCTION: CMR2 0x00455ca0
int StageTiming_GetCurrentSplitPositionOfDriver(int iDriver)
{
    int slot;
    int iSplit;

    slot = g_stageDriverSlot[iDriver];
    iSplit = GetStageSplitCount();
    return g_stageSplitPositions[iSplit][slot];
}

// FUNCTION: CMR2 0x00455cc0
int StageTiming_GetSplitDriverIDForPosition(int iPosition, int iSplit)
{
    if (iPosition >= g_stageSplitDriverCount[iSplit])
        return -1;
    return g_stageSplitDriverIndices[iSplit][iPosition];
}

// FUNCTION: CMR2 0x00455d40
int StageTiming_GetSplitTimeForPosition(int iPosition, int iSplit)
{
    return ConvertRawTimeToCentiseconds(g_stageSplitTimesRaw[iSplit][g_stageSplitTimesRawDriverIx[iSplit][iPosition]]);
}

// FUNCTION: CMR2 0x00455db0
int StageTiming_GetCurrentSplitTimeForDriver(int iDriver)
{
    int iSplit;

    iSplit = GetStageSplitCount();
    return ConvertRawTimeToCentiseconds(g_stageSplitTimesRaw[iSplit][g_stageSplitTimesRawDriverIx[iSplit][g_stageSplitPositions[iSplit][iDriver]]]);
}

// FUNCTION: CMR2 0x00455de0
int StageTiming_GetDriverSlot(int iDriver)
{
    return g_stageDriverSlot[iDriver];
}

// FUNCTION: CMR2 0x00455e60
void StageTiming_Reset(void)
{
    int iSplit;
    int i;

    memset(g_stageDriverSlot, 0, sizeof(g_stageDriverSlot));
    g_unk0x00541f98 = 16;
    for (iSplit = 0; iSplit < 10; iSplit++)
    {
        for (i = 0; i < 16; i++)
        {
            g_stageSplitDriverIndices[iSplit][i] = i;
            g_stageSplitPositions[iSplit][i] = i;
            g_stageSplitTimesRawDriverIx[iSplit][i] = i;
            g_stageSplitTimesRaw[iSplit][i] = 0;
        }
    }
}

// FUNCTION: CMR2 0x00455d10
void StageTiming_AddToOverall(void)
{
    int iSplit;

    iSplit = GetStageSplitCount();
    RallyTiming_AddStageTimes(g_stageSplitDriverIndices[iSplit], g_stageSplitTimesRawDriverIx[iSplit], g_stageSplitTimesRaw[iSplit]);
}

// FUNCTION: CMR2 0x00455df0
void StageTiming_GetSplitTimesForPositions(int iPosition1, int iPosition2, int *piTime1, int *piTime2)
{
    int iSplit;
    int iSlot1;
    int iSlot2;

    iSplit = GetStageSplitCount();
    iSlot1 = iPosition1 - CGameInfo::FUN_00405d70();
    iSlot2 = iPosition2 - CGameInfo::FUN_00405d70();
    *piTime1 = StageTiming_GetSplitTimeForPosition(g_stageSplitPositions[iSplit][iSlot1], iSplit);
    *piTime2 = StageTiming_GetSplitTimeForPosition(g_stageSplitPositions[iSplit][iSlot2], iSplit);
}

// FUNCTION: CMR2 0x00456250
void StageTiming_RebuildSplitPositions(void)
{
    int aiDriverForSlot[16];
    char *pIndices;
    int iSplit;
    int s;
    int i;
    int d;

    iSplit = GetStageSplitCount();
    for (i = 0; i < g_unk0x00541f98; i++)
    {
        pIndices = g_stageSplitDriverIndices[iSplit];
        d = pIndices[i];
        aiDriverForSlot[g_stageSplitTimesRawDriverIx[iSplit][i]] = d;
        g_stageSplitPositions[iSplit][d] = i;
    }
    for (i = g_unk0x00541f98; i < 16; i++)
    {
        g_stageSplitDriverIndices[iSplit][i] = i;
        g_stageSplitPositions[iSplit][i] = i;
    }
    for (s = 1; s < iSplit; s++)
    {
        pIndices = g_stageSplitDriverIndices[s];
        for (i = 0; i < g_unk0x00541f98; i++)
        {
            d = aiDriverForSlot[g_stageSplitTimesRawDriverIx[s][i]];
            pIndices[i] = d;
            g_stageSplitPositions[s][d] = i;
        }
        for (i = g_unk0x00541f98; i < 16; i++)
        {
            pIndices[i] = i;
            g_stageSplitPositions[s][i] = i;
        }
    }
}

// Sets every vertex of a mesh-backed scene node to a shade of grey.
// FUNCTION: CMR2 0x00456d20
void StageTiming_FUN_00456d20(SceneNode *pNode, BYTE colour)
{
    BYTE rgb[4];

    rgb[0] = colour;
    rgb[1] = colour;
    rgb[2] = colour;
    rgb[3] = 0;
    if (pNode->type == 0 && pNode->pObject != 0)
        Mesh_SetVertexColours((Mesh *)pNode->pObject, rgb);
}

// GLOBAL: CMR2 0x00588e80
void *g_unk0x00588e80[8];
// GLOBAL: CMR2 0x00588d3c
int g_unk0x00588d3c;
// GLOBAL: CMR2 0x00588d14
int g_unk0x00588d14;

// Releases the eight stage buffers.
// FUNCTION: CMR2 0x0046c500
BOOL FUN_0046c500(void)
{
    void **p;

    p = g_unk0x00588e80;
    do {
        if (*p != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*p);
            *p = NULL;
        }
        p++;
    } while ((int)p < 0x588ea0);
    g_unk0x00588d3c = 0;
    g_unk0x00588d14 = 0;
    return TRUE;
}

// 12-byte entry of the table released by FUN_00456b70.
struct Unk0x542ae8 {
    void *pBuffer;
    void *field_0x4;
    void *field_0x8;
};

// GLOBAL: CMR2 0x00542ae8
Unk0x542ae8 g_unk0x00542ae8[32];

// TODO: CMR2 0x00456b70 (implemented, match below 90%)
bool FUN_00456b70(void)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (g_unk0x00542ae8[i].pBuffer != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00542ae8[i].pBuffer);
            g_unk0x00542ae8[i].pBuffer = NULL;
        }
        g_unk0x00542ae8[i].pBuffer = NULL;
        g_unk0x00542ae8[i].field_0x4 = NULL;
        g_unk0x00542ae8[i].field_0x8 = NULL;
    }
    return true;
}

// GLOBAL: CMR2 0x0053d1da
BYTE g_unk0x0053d1da[0x100];
// GLOBAL: CMR2 0x0053d1b0
int g_unk0x0053d1b0;
// GLOBAL: CMR2 0x0053e190
int g_unk0x0053e190[0x400];
// GLOBAL: CMR2 0x0053d1b8
int g_unk0x0053d1b8[0x100];
// GLOBAL: CMR2 0x0053e18c
BYTE g_unk0x0053e18c;

// TODO: CMR2 0x00448630 (implemented, match below 90%)
void FUN_00448630(int index)
{
    int value;

    g_unk0x0053d1da[index] = 1;
    value = g_unk0x0053d1b0;
    g_unk0x0053e190[GetStageSplitCount() + index * 9] = value;
    g_unk0x0053d1b8[index] = value + 0x4650;
    g_unk0x0053e18c++;
}

// GLOBAL: CMR2 0x0053aba8
void *g_unk0x0053aba8[64];
// GLOBAL: CMR2 0x00428790
BYTE g_unk0x00428790[1];
// GLOBAL: CMR2 0x0053c9a4
void *g_unk0x0053c9a4;
// GLOBAL: CMR2 0x0053bd68
int g_unk0x0053bd68;

// Allocates one 0xc24-byte block per slot and registers the 0x428790 callback.
// FUNCTION: CMR2 0x004287c0
void FUN_004287c0(int size)
{
    BYTE *pBuffer;
    int i;

    pBuffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(size * 0xc24);
    g_unk0x0053c9a4 = pBuffer;
    g_unk0x0053bd68 = size;
    for (i = 0; i < size; i++) {
        g_unk0x0053aba8[i] = pBuffer;
        pBuffer += 0xc24;
    }
    CGame::RegisterCallback(g_unk0x00428790, NULL);
}

// Registers the mesh-backed node in the per-slot tables.
// FUNCTION: CMR2 0x00466e90
void FUN_00466e90(SceneNode *pNode, int *pSlot)
{
    int index;
    void *pObject;
    int count;

    index = (pNode->flags & 0xff) - 5;
    pObject = pNode->pObject;
    if (pObject == NULL)
        return;
    if (pNode->type != 0)
        return;
    pSlot[index + 0xf] = (int)pNode;
    pSlot[index] = (int)pObject;
    count = Mesh_GetField0x10((Mesh *)pObject);
    pSlot[index + 0x108] = count;
    if (pSlot[index] == 0)
        return;
    if (count <= 0)
        return;
    pSlot[0x117]++;
}

// GLOBAL: CMR2 0x00592734
void *g_unk0x00592734;
// GLOBAL: CMR2 0x00592738
int g_unk0x00592738;
// GLOBAL: CMR2 0x00494b10
BYTE g_unk0x00494b10[1];

// FUNCTION: CMR2 0x00494b50
void FUN_00494b50(int count)
{
    BYTE *pBuffer;

    pBuffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(count * 0x2a4);
    g_unk0x00592734 = pBuffer;
    memset(pBuffer, 0, count * 0x2a4);
    g_unk0x00592738 = count;
    CGame::RegisterCallback(g_unk0x00494b10, NULL);
}

// GLOBAL: CMR2 0x00547ac8
void *g_unk0x00547ac8;
// GLOBAL: CMR2 0x00543ecc
void *g_unk0x00543ecc;
// GLOBAL: CMR2 0x00543eb8
void *g_unk0x00543eb8;
// GLOBAL: CMR2 0x00547acc
BYTE g_unk0x00547acc;
// GLOBAL: CMR2 0x0045e560
BYTE g_unk0x0045e560[1];

// FUNCTION: CMR2 0x0045e5b0
void FUN_0045e5b0(int count)
{
    g_unk0x00547ac8 = CFileBuffer::AllocateLockedBuffer(count * 376);
    g_unk0x00543ecc = CFileBuffer::AllocateLockedBuffer(count * 12);
    g_unk0x00543eb8 = CFileBuffer::AllocateLockedBuffer(count * 44);
    g_unk0x00547acc = (BYTE)count;
    CGame::RegisterCallback(g_unk0x0045e560, NULL);
}

// FUNCTION: CMR2 0x00465ec0
void FUN_00465ec0(SceneNode *pNode, int alpha, BYTE checkFlag)
{
    BYTE rgb[4];
    Mesh *pMesh;

    rgb[0] = 0x80;
    rgb[1] = 0x80;
    rgb[2] = 0x80;
    rgb[3] = 0;
    if (pNode->type != 0)
        return;
    pMesh = (Mesh *)pNode->pObject;
    if (pMesh == NULL)
        return;
    *(int *)((char *)pMesh + 0x30) |= 0x40008;
    Mesh_SetVertexAlpha(pMesh, (BYTE)alpha);
    if (checkFlag != 0)
        Mesh_SetVertexColours(pMesh, rgb);
}
