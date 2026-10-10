// Bink 1 playback: decode sequential frames and queue the selected sound track.
#include "port/audio.h"
#include "port/gfx.h"
#include "port/movie.h"
#include "port/sys.h"
#include "video/bink_decoder.h"

#include <cstring>
#include <new>

struct Movie {
    SysFile *file = nullptr;
    Bink::Decoder decoder;
    Bink::Frame decoded;
    Bink::Info info;
    DWORD frame = 0, startTicks = 0;
    bool decodedCurrent = false, started = false;
    GfxTexture *texture = nullptr;
    AudioStream *audio = nullptr;
};

static bool ReadMovie(void *context, uint32_t offset, void *dst, uint32_t bytes)
{
    SysFile *file = static_cast<SysFile *>(context);
    return offset <= 0x7fffffff && Sys_SeekFile(file, (LONG)offset, SYS_SEEK_SET) == offset &&
           Sys_ReadFile(file, dst, bytes) == bytes;
}

extern "C" Movie *Movie_Open(const char *path, int audioTrack)
{
    SysFile *file = Sys_OpenFile(path, SYS_FILE_READ);
    if (!file)
        return nullptr;
    Movie *movie = new (std::nothrow) Movie;
    if (!movie) {
        Sys_CloseFile(file);
        return nullptr;
    }
    movie->file = file;
    DWORD size = Sys_GetFileSize(file);
    if (size > 0x7fffffff || !movie->decoder.Open({file, size, ReadMovie}, audioTrack)) {
        Sys_Log("movie %s: %s", path, movie->decoder.Error().c_str());
        Movie_Close(movie);
        return nullptr;
    }
    movie->info = movie->decoder.GetInfo();
    movie->texture = Gfx_CreateTexture(movie->info.width, movie->info.height, GFX_FORMAT_BGRX8, 1,
                                       GFX_TEXTURE_DYNAMIC);
    if (!movie->texture) {
        Sys_Log("movie %s: cannot create frame texture", path);
        Movie_Close(movie);
        return nullptr;
    }
    if (movie->info.channels) {
        movie->audio = Audio_CreateStream(movie->info.sampleRate, movie->info.channels);
        if (!movie->audio)
            Sys_Log("movie %s: audio output unavailable", path);
    }
    Sys_Log("movie %s: %ux%u, %u frames at %u/%u fps, %u Hz/%u channels", path,
            movie->info.width, movie->info.height, movie->info.frames,
            movie->info.fpsNumerator, movie->info.fpsDenominator,
            movie->info.sampleRate, movie->info.channels);
    return movie;
}

extern "C" void Movie_Close(Movie *movie)
{
    if (!movie)
        return;
    Audio_ReleaseStream(movie->audio);
    Gfx_DestroyTexture(movie->texture);
    Sys_CloseFile(movie->file);
    delete movie;
}

extern "C" DWORD Movie_GetWidth(Movie *movie) { return movie->info.width; }
extern "C" DWORD Movie_GetHeight(Movie *movie) { return movie->info.height; }
extern "C" DWORD Movie_GetFrameNumber(Movie *movie) { return movie->frame + 1; }
extern "C" DWORD Movie_GetFrameCount(Movie *movie) { return movie->info.frames; }

extern "C" void Movie_DecodeFrame(Movie *movie)
{
    if (movie->decodedCurrent || movie->frame >= movie->info.frames)
        return;
    if (!movie->decoder.Decode(movie->decoded)) {
        Sys_Log("movie frame %u: %s", movie->frame + 1, movie->decoder.Error().c_str());
        Audio_ReleaseStream(movie->audio);
        movie->audio = nullptr;
        movie->frame = movie->info.frames;
        movie->started = false;
        return;
    }
    movie->decodedCurrent = true;
    if (movie->audio && !movie->decoded.audio.empty() &&
        !Audio_QueueStream(movie->audio, movie->decoded.audio.data(), movie->decoded.audio.size())) {
        Sys_Log("movie: cannot queue audio");
        Audio_ReleaseStream(movie->audio);
        movie->audio = nullptr;
    }
    GfxLockedRect rect;
    if (Gfx_LockTexture(movie->texture, 0, 0, &rect)) {
        for (DWORD y = 0; y < movie->info.height; ++y)
            memcpy((BYTE *)rect.pixels + y * rect.pitch,
                   movie->decoded.bgrx.data() + size_t(y) * movie->info.width * 4, movie->info.width * 4);
        Gfx_UnlockTexture(movie->texture, 0, 0);
    }
}

extern "C" void Movie_NextFrame(Movie *movie)
{
    if (movie->frame < movie->info.frames)
        ++movie->frame;
    movie->decodedCurrent = false;
}

extern "C" BOOL Movie_Wait(Movie *movie)
{
    if (!movie->started)
        return FALSE;
    DWORD due = (DWORD)(uint64_t(movie->frame) * 1000 * movie->info.fpsDenominator / movie->info.fpsNumerator);
    return Sys_GetTicks() - movie->startTicks < due;
}

extern "C" void Movie_Present(Movie *movie, int x, int y, int width, int height)
{
    // The game caches its render-state setters. Leave their actual values
    // intact so the next menu can still blend its fonts and sprites.
    Gfx_PushState();
    GfxTLVertex v[4];
    float x0 = (float)x, y0 = (float)y, x1 = (float)(x + width), y1 = (float)(y + height);

    memset(v, 0, sizeof(v));
    for (int i = 0; i < 4; i++) {
        v[i].rhw = 1.0f;
        v[i].color = 0xffffffff;
    }
    v[0].sx = x0; v[0].sy = y0; v[0].tu = 0.0f; v[0].tv = 0.0f;
    v[1].sx = x1; v[1].sy = y0; v[1].tu = 1.0f; v[1].tv = 0.0f;
    v[2].sx = x0; v[2].sy = y1; v[2].tu = 0.0f; v[2].tv = 1.0f;
    v[3].sx = x1; v[3].sy = y1; v[3].tu = 1.0f; v[3].tv = 1.0f;

    Gfx_Clear(GFX_CLEAR_TARGET | GFX_CLEAR_ZBUFFER, 0xff000000, 1.0f);
    Gfx_BeginScene();
    Gfx_SetDepthTest(FALSE);
    Gfx_SetAlphaBlend(FALSE);
    Gfx_SetAlphaTest(FALSE);
    Gfx_SetCullMode(GFX_CULL_NONE);
    Gfx_SetTexture(0, movie->texture);
    Gfx_SetStageColorOp(0, GFX_TOP_SELECTARG1);
    Gfx_SetStageColorArg1(0, GFX_TA_TEXTURE);
    Gfx_SetStageAlphaOp(0, GFX_TOP_SELECTARG1);
    Gfx_SetStageAlphaArg1(0, GFX_TA_TEXTURE);
    Gfx_SetStageColorOp(1, GFX_TOP_DISABLE);
    Gfx_SetStageAlphaOp(1, GFX_TOP_DISABLE);
    Gfx_SetStageMinFilter(0, GFX_FILTER_LINEAR);
    Gfx_SetStageMagFilter(0, GFX_FILTER_LINEAR);
    Gfx_SetStageAddress(0, GFX_ADDRESS_CLAMP);
    Gfx_DrawPrimitiveTL(GFX_TRIANGLESTRIP, v, 4);
    Gfx_EndScene();
    Gfx_PresentStretched(TRUE);
    Gfx_PopState();
    if (!movie->started && movie->decodedCurrent) {
        movie->startTicks = Sys_GetTicks();
        movie->started = true;
        Audio_StartStream(movie->audio);
    }
}
