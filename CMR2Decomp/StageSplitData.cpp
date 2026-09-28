#include "StageSplitData.h"

// Two 0x48-byte StageSplitData entries (one per car); the second one's last
// dword and the dword after it are also the per-car flags g_unk0x00536e88[2].
// GLOBAL: CMR2 0x00536dfc
BYTE g_stageSplitBlock[0x94];
