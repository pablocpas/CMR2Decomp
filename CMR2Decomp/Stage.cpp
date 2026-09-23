#include "Stage.h"

// GLOBAL: CMR2 0x00542cac
char g_stageSplitCount = 0;
// GLOBAL: CMR2 0x00542cae
char g_stageLooped;
// GLOBAL: CMR2 0x00542cb0
int g_unk0x00542cb0;
// GLOBAL: CMR2 0x00542c68
int g_unk0x00542c68;
// GLOBAL: CMR2 0x00542c70
int g_stageCheckpointCount;
// GLOBAL: CMR2 0x00542c74
int g_unk0x00542c74;
// GLOBAL: CMR2 0x00542c7c
int g_unk0x00542c7c[12];

// FUNCTION: CMR2 0x004583c0
int GetStageSplitCount(void)
{
	return g_stageSplitCount;
}

// FUNCTION: CMR2 0x00458390
int FUN_00458390(void)
{
    return g_unk0x00542c68;
}

// FUNCTION: CMR2 0x004583a0
int FUN_004583a0(void)
{
    return g_unk0x00542c74;
}

// FUNCTION: CMR2 0x004583b0
int FUN_004583b0(int index)
{
    return g_unk0x00542c7c[index];
}

// FUNCTION: CMR2 0x004584c0
int FUN_004584c0(void)
{
    return g_unk0x00542cb0;
}

// Previous checkpoint, wrapping round on looped stages.
// TODO: CMR2 0x00459320 (implemented, match 61%)
int FUN_00459320(int index)
{
    index--;
    if (index == -1) {
        if (g_stageLooped != 0)
            index = g_stageCheckpointCount - 1;
        else
            index = 0;
    }
    return index;
}

// Next checkpoint, wrapping round on looped stages.
// FUNCTION: CMR2 0x00459350
int FUN_00459350(int index)
{
    index++;
    if (index == g_stageCheckpointCount && g_stageLooped != 0)
        index = 0;
    return index;
}

