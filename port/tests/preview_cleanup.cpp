#include "port/types.h"
#include "GameInfo.h"
#include "DeformGeometry.h"

#include <cstdio>
#include <cstdlib>

extern DeformVertex **g_previewOriginalVertices[16];
extern BYTE *g_previewOriginalNodeTypes[16];
extern BYTE g_previewSessionRegistered;
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
        g_previewOriginalVertices[i] = static_cast<DeformVertex **>(calloc(1, sizeof(DeformVertex *)));
        g_previewOriginalNodeTypes[i] = static_cast<BYTE *>(calloc(1, 1));
    }
    g_previewSessionRegistered = 1;
    OptionPreview_ReleaseSessionResources();
    for (int i = 0; i < 16; ++i)
        if (g_previewOriginalVertices[i] || g_previewOriginalNodeTypes[i])
            return 1;
    if (g_previewSessionRegistered)
        return 1;
    OptionPreview_ReleaseSessionResources();
    puts("preview cleanup: all 16 slots and repeated cleanup passed");
}
