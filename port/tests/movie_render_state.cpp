#include "bink_fixture.h"
#include "port/audio.h"
#include "port/movie.h"
#include "port/sys.h"
#include "render/gfx_internal.h"

using namespace BinkTest;
struct SysFile { size_t position = 0; };
static std::vector<uint8_t> data;
static GfxDrawState drawnState;
static bool stretched;

extern "C" SysFile *Sys_OpenFile(const char *, int) { return new SysFile; }
extern "C" void Sys_CloseFile(SysFile *file) { delete file; }
extern "C" DWORD Sys_GetFileSize(SysFile *) { return data.size(); }
extern "C" DWORD Sys_SeekFile(SysFile *file, LONG offset, int) { file->position = offset; return offset; }
extern "C" DWORD Sys_ReadFile(SysFile *file, void *dst, DWORD count) {
    count = std::min(count, DWORD(data.size() - file->position));
    std::memcpy(dst, data.data() + file->position, count); file->position += count; return count;
}
extern "C" DWORD Sys_GetTicks() { return 0; }
extern "C" void Sys_GetDesktopSize(int *w, int *h) { *w = 640; *h = 480; }
extern "C" void Sys_Log(const char *, ...) {}
int Platform_GetSettingInt(const char *, int value) { return value; }
extern "C" AudioStream *Audio_CreateStream(DWORD, int) { return nullptr; }
extern "C" BOOL Audio_QueueStream(AudioStream *, const float *, DWORD) { return FALSE; }
extern "C" void Audio_StartStream(AudioStream *) {}
extern "C" void Audio_ReleaseStream(AudioStream *) {}

struct CaptureExecutor : GfxExecutor {
    bool SetVideoMode(int, int, bool) override { return true; }
    void Execute(GfxFrame &frame, std::vector<GfxTexture *> &, GfxReadback &, bool, bool, bool stretch) override {
        stretched = stretch;
        for (const auto &command : frame.commands)
            if (command.type == GFX_CMD_DRAW)
                drawnState = frame.states[command.state];
    }
    void DestroyTexture(GfxTexture *) override {}
    void ReadTexture(GfxTexture *) override {}
};
GfxExecutor *Gfx_CreateGpuExecutor() { return new CaptureExecutor; }

int main()
{
    data = Container(8, 8, {Video(8, 8, 6, 81)});
    Check(Gfx_SetVideoMode(640, 480, FALSE), "renderer init");
    auto *menuTexture = Gfx_CreateTexture(8, 8, GFX_FORMAT_BGRA8, 1, 0);
    Gfx_SetTexture(0, menuTexture);
    Gfx_SetAlphaBlend(TRUE);
    Gfx_SetAlphaTest(TRUE);
    Gfx_SetSrcBlend(GFX_BLEND_SRCALPHA);
    Gfx_SetDestBlend(GFX_BLEND_INVSRCALPHA);
    Gfx_SetCullMode(GFX_CULL_CW);
    Gfx_SetStageColorOp(0, GFX_TOP_MODULATE);
    Gfx_SetStageAlphaOp(0, GFX_TOP_MODULATE);
    Gfx_SetStageColorOp(1, GFX_TOP_MODULATE);
    Gfx_SetStageAlphaOp(1, GFX_TOP_MODULATE);
    Movie *movie = Movie_Open("test.bik", -1);
    Check(movie != nullptr, "movie open");
    Movie_DecodeFrame(movie);
    for (int i = 0; i < 2; ++i) {
        Movie_Present(movie, 0, 0, 640, 480);
        Check(stretched && !drawnState.alphaBlend && !drawnState.alphaTest && !drawnState.depthTest &&
              drawnState.stages[0].texture != menuTexture, "movie pass uses its own state");
        // Draw the menu without resetting the cached state, exactly as at boot.
        GfxTLVertex vertices[3] = {};
        Gfx_DrawPrimitiveTL(GFX_TRIANGLELIST, vertices, 3);
        Gfx_Present(FALSE);
        Check(!stretched && drawnState.alphaBlend && drawnState.alphaTest && drawnState.depthTest &&
              drawnState.cull == GFX_CULL_CW && drawnState.srcBlend == GFX_BLEND_SRCALPHA &&
              drawnState.destBlend == GFX_BLEND_INVSRCALPHA, "menu transparency and depth survive movies");
        Check(drawnState.stages[0].texture == menuTexture &&
              drawnState.stages[0].colorOp == GFX_TOP_MODULATE && drawnState.stages[0].alphaOp == GFX_TOP_MODULATE &&
              drawnState.stages[1].colorOp == GFX_TOP_MODULATE && drawnState.stages[1].alphaOp == GFX_TOP_MODULATE &&
              drawnState.stages[0].address == GFX_ADDRESS_WRAP &&
              drawnState.stages[0].minFilter == GFX_FILTER_POINT && drawnState.stages[0].magFilter == GFX_FILTER_POINT,
              "menu texture stages and sampling survive movies");
    }
    Movie_Close(movie);
    Gfx_Shutdown();
    puts("movie render state: menu rendering survives movie presentation");
}
