#include "port/types.h"
#include "GameInfo.h"

#include <cstdio>
#include <cstdlib>

extern BYTE **g_unk0x00831198[16];
extern BYTE *g_unk0x0082d1dc[16];
extern BYTE g_unk0x00831318;
extern BYTE OptionPreview_ReleaseSessionResources(void);

int main()
{
    // Exercise the real callback on an empty session and on every supported
    // slot, including the unused slots traversed when leaving the garage.
    CGameInfo::m_gameInfo.field_0x14 = 0;
    CGameInfo::SetPreviewMode(1);
    OptionPreview_ReleaseSessionResources();
    for (int i = 0; i < 16; ++i) {
        // With no source meshes, cleanup still owns these top-level arrays.
        g_unk0x00831198[i] = static_cast<BYTE **>(calloc(1, sizeof(BYTE *)));
        g_unk0x0082d1dc[i] = static_cast<BYTE *>(calloc(1, 1));
    }
    g_unk0x00831318 = 1;
    OptionPreview_ReleaseSessionResources();
    for (int i = 0; i < 16; ++i)
        if (g_unk0x00831198[i] || g_unk0x0082d1dc[i])
            return 1;
    if (g_unk0x00831318)
        return 1;
    OptionPreview_ReleaseSessionResources();
    puts("preview cleanup: all 16 slots and repeated cleanup passed");
}
