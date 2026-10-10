#include "bink_fixture.h"
#include "port/audio.h"
#include "port/gfx.h"
#include "port/movie.h"
#include "port/sys.h"
#include "render/presentation.h"

using namespace BinkTest;
struct SysFile { size_t position = 0; };
struct GfxTexture { int width, height, pitch; std::vector<uint8_t> pixels; };
struct AudioStream {};
static std::vector<uint8_t> data;
static GfxTexture *texture;
static DWORD ticks;
static int files, textures, streams, queues, starts;
static bool failTexture;
static std::vector<float> queued;
static int stretchedPresents;
static int savedStates;
static GfxTLVertex drawn[4];

extern "C" SysFile *Sys_OpenFile(const char *, int) { ++files; return new SysFile; }
extern "C" void Sys_CloseFile(SysFile *file) { --files; delete file; }
extern "C" DWORD Sys_GetFileSize(SysFile *) { return data.size(); }
extern "C" DWORD Sys_SeekFile(SysFile *file, LONG offset, int) {
    if (offset < 0 || size_t(offset) > data.size()) return 0xffffffff;
    file->position = offset; return offset;
}
extern "C" DWORD Sys_ReadFile(SysFile *file, void *dst, DWORD count) {
    count = std::min(count, DWORD(data.size() - file->position));
    std::memcpy(dst, data.data() + file->position, count); file->position += count; return count;
}
extern "C" DWORD Sys_GetTicks() { return ticks; }
extern "C" void Sys_Log(const char *, ...) {}
extern "C" GfxTexture *Gfx_CreateTexture(int width, int height, int, int, DWORD) {
    if (failTexture) return nullptr;
    ++textures;
    texture = new GfxTexture{width, height, width * 4 + 16, {}};
    texture->pixels.assign(texture->pitch * height, 0xab);
    return texture;
}
extern "C" void Gfx_DestroyTexture(GfxTexture *t) { if (t) { --textures; delete t; texture = nullptr; } }
extern "C" BOOL Gfx_LockTexture(GfxTexture *t, int, int, GfxLockedRect *out) {
    out->pixels = t->pixels.data(); out->pitch = t->pitch; out->width = t->width; out->height = t->height;
    return TRUE;
}
extern "C" void Gfx_UnlockTexture(GfxTexture *, int, int) {}
extern "C" AudioStream *Audio_CreateStream(DWORD rate, int channels) {
    Check(rate == 8000 && channels == 2, "movie audio format");
    ++streams; return new AudioStream;
}
extern "C" BOOL Audio_QueueStream(AudioStream *, const float *samples, DWORD count) {
    ++queues; queued.insert(queued.end(), samples, samples + count); return TRUE;
}
extern "C" void Audio_StartStream(AudioStream *) { ++starts; }
extern "C" void Audio_ReleaseStream(AudioStream *stream) { if (stream) { --streams; delete stream; } }
extern "C" void Gfx_Clear(DWORD, DWORD, float) {}
extern "C" void Gfx_BeginScene() {}
extern "C" void Gfx_EndScene() {}
extern "C" void Gfx_Present(BOOL) {}
extern "C" void Gfx_PresentStretched(BOOL) { ++stretchedPresents; }
extern "C" void Gfx_PushState() { ++savedStates; }
extern "C" void Gfx_PopState() { --savedStates; }
extern "C" void Gfx_SetDepthTest(BOOL) {}
extern "C" void Gfx_SetAlphaBlend(BOOL) {}
extern "C" void Gfx_SetAlphaTest(BOOL) {}
extern "C" void Gfx_SetCullMode(int) {}
extern "C" void Gfx_SetTexture(int, GfxTexture *) {}
extern "C" void Gfx_SetStageColorOp(int, int) {}
extern "C" void Gfx_SetStageColorArg1(int, int) {}
extern "C" void Gfx_SetStageAlphaOp(int, int) {}
extern "C" void Gfx_SetStageAlphaArg1(int, int) {}
extern "C" void Gfx_SetStageMinFilter(int, int) {}
extern "C" void Gfx_SetStageMagFilter(int, int) {}
extern "C" void Gfx_SetStageAddress(int, int) {}
extern "C" void Gfx_DrawPrimitiveTL(int primitive, const GfxTLVertex *vertices, DWORD count) {
    Check(primitive == GFX_TRIANGLESTRIP && count == 4, "movie quad");
    std::copy(vertices, vertices + 4, drawn);
}

int main() {
    auto audio = Audio(false), video = Video(9, 9, 6, 81);
    std::vector<uint8_t> packet(4); Put32(packet, 0, audio.size());
    packet.insert(packet.end(), audio.begin(), audio.end()); packet.insert(packet.end(), video.begin(), video.end());
    data = Container(9, 9, {packet, packet}, 0x2000);
    Movie *movie = Movie_Open("test.bik", 7);
    Check(movie && files == 1 && textures == 1 && streams == 1, "movie open resources");
    ticks = 0xfffffff0; // First present, not opening, establishes the playback clock.
    Movie_DecodeFrame(movie); Movie_DecodeFrame(movie);
    Check(queues == 1 && starts == 0 && queued.size() == 960, "duplicate decode/audio start");
    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 9; ++x) Check(texture->pixels[y * texture->pitch + x * 4] == 76, "texture pixels");
        for (int x = 36; x < texture->pitch; ++x) Check(texture->pixels[y * texture->pitch + x] == 0xab, "texture pitch padding");
    }
    Movie_Present(movie, 0, 0, 1920, 1080);
    Check(starts == 1, "audio starts with first presentation");
    Check(savedStates == 0, "movie balances render-state preservation");
    Check(stretchedPresents == 1 && drawn[0].sx == 0 && drawn[0].sy == 0 &&
          drawn[3].sx == 1920 && drawn[3].sy == 1080 && drawn[3].tu == 1 && drawn[3].tv == 1,
          "movie stretches to the requested rectangle");
    PresentationRect rect = GetPresentationRect(640, 480, 1920, 1080, true);
    Check(rect.x == 0 && rect.y == 0 && rect.width == 1920 && rect.height == 1080,
          "movie fills widescreen output with a 4:3 back buffer");
    rect = GetPresentationRect(640, 480, 1920, 1080, false);
    Check(rect.x == 240 && rect.y == 0 && rect.width == 1440 && rect.height == 1080,
          "game presentation after movies retains aspect ratio");
    rect = GetPresentationRect(640, 480, 900, 1600, true);
    Check(rect.x == 0 && rect.y == 0 && rect.width == 900 && rect.height == 1600,
          "movie fills resized portrait window");
    Movie_NextFrame(movie);
    Check(Movie_GetFrameNumber(movie) == 2 && Movie_GetFrameCount(movie) == 2, "last frame accessible");
    Check(Movie_Wait(movie), "fractional frame pacing");
    ticks += 33; // Includes millisecond clock wrap.
    Check(!Movie_Wait(movie), "wrapped playback clock");
    Movie_DecodeFrame(movie); Movie_Present(movie, 0, 0, 640, 480);
    Check(queues == 2 && starts == 1, "second frame audio/clock continuity");
    Movie_NextFrame(movie); ticks += 33;
    Check(Movie_GetFrameNumber(movie) > Movie_GetFrameCount(movie) && !Movie_Wait(movie), "movie completion");
    Movie_Close(movie);
    Check(files == 0 && textures == 0 && streams == 0, "close discards movie resources/audio");
    movie = Movie_Open("test.bik", 7); Movie_DecodeFrame(movie); Movie_Close(movie);
    Check(files == 0 && textures == 0 && streams == 0, "skip releases queued sound");
    uint32_t second = uint32_t(data[60]) | uint32_t(data[61]) << 8 |
                      uint32_t(data[62]) << 16 | uint32_t(data[63]) << 24;
    Put32(data, second & ~1u, 0xffffffff);
    movie = Movie_Open("test.bik", 7);
    Movie_DecodeFrame(movie); Movie_Present(movie, 0, 0, 640, 480); Movie_NextFrame(movie);
    Movie_DecodeFrame(movie);
    Check(Movie_GetFrameNumber(movie) > Movie_GetFrameCount(movie) && !Movie_Wait(movie) && streams == 0,
          "corrupt packet must stop audio/playback immediately");
    Movie_Close(movie);
    failTexture = true;
    Check(!Movie_Open("test.bik", 7) && files == 0 && textures == 0 && streams == 0, "texture failure cleanup");
    data[0] = 0;
    Check(!Movie_Open("test.bik", 7) && files == 0, "invalid header cleanup");
    Movie_Close(nullptr);
    std::puts("Movie texture, audio queue, pacing, final frame and cleanup tests passed");
}
