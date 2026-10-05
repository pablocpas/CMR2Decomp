#ifndef _STAGE_H
#define _STAGE_H

struct Unk0x542ae8 {
    void *pBuffer;
    void *field_0x4;
    void *field_0x8;
};
struct StageArchiveTables {
    Unk0x542ae8 archives[32];
    int driverCount;
    int secondaryCount;
};
typedef char StageArchiveSizeCheck[sizeof(StageArchiveTables) == 0x188 ? 1 : -1];
extern StageArchiveTables g_stageArchiveTables;
#define g_unk0x00542ae8 (g_stageArchiveTables.archives)
#define g_unk0x00542c68 (g_stageArchiveTables.driverCount)
#define g_unk0x00542c6c (g_stageArchiveTables.secondaryCount)

int Stage_GetSplitPositionFixed(int index);
int GetStageSplitCount(void);

#endif
