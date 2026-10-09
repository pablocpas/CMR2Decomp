#ifndef _STAGE_OBJECT_COUNT_H
#define _STAGE_OBJECT_COUNT_H

// 0x547acc is accessed as a BYTE by writers and guards, and as a masked
// DWORD by the ramp walkers. Both views must share an allocated DWORD.
union StageObjectCount {
    unsigned int packed;
    unsigned char value;
};
extern StageObjectCount g_stageObjectCount;
#define g_unk0x00547acc (g_stageObjectCount.value)

#endif
