// Audio on SDL3: see port/audio.h. Every buffer is mixed in software into one
// float stereo stream, with DirectSound's volume and pan laws.

#include "platform/platform.h"
#include "port/audio.h"
#include "port/sys.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <algorithm>

namespace {

struct AudioData {
    BYTE *bytes;
    DWORD size;
    DWORD sampleRate;
    int bits;
    int channels;
    int refs;
};

}

struct AudioBuffer {
    AudioData *data;
    bool playing = false;
    bool loop = false;
    double position = 0.0;      // in frames
    DWORD frequency;
    float leftGain = 1.0f, rightGain = 1.0f;
    LONG volume = 0, pan = 0;
};

struct AudioStream {
    SDL_AudioStream *converter;
    bool playing = false;
};

namespace {

SDL_AudioStream *s_stream;
SDL_Mutex *s_lock;
int s_outputRate = 44100;
std::vector<AudioBuffer *> s_buffers;
std::vector<AudioStream *> s_movieStreams;
std::vector<float> s_mix;
std::vector<float> s_movieMix;

// DirectSound's laws: volume and pan in hundredths of a decibel.
float DecibelGain(LONG hundredths)
{
    if (hundredths <= -10000)
        return 0.0f;
    return powf(10.0f, hundredths / 2000.0f);
}

void UpdateGains(AudioBuffer *b)
{
    float volume = DecibelGain(b->volume);
    float left = b->pan > 0 ? DecibelGain(-b->pan) : 1.0f;
    float right = b->pan < 0 ? DecibelGain(b->pan) : 1.0f;
    b->leftGain = volume * left;
    b->rightGain = volume * right;
}

inline float SampleAt(const AudioData *d, DWORD frame, int channel)
{
    int ch = channel < d->channels ? channel : 0;
    if (d->bits == 16) {
        const Sint16 *p = (const Sint16 *)d->bytes;
        return p[frame * d->channels + ch] / 32768.0f;
    }
    return (d->bytes[frame * d->channels + ch] - 128) / 128.0f;
}

void MixBuffer(AudioBuffer *b, float *out, int frames)
{
    AudioData *d = b->data;
    DWORD frameBytes = (DWORD)(d->bits / 8 * d->channels);
    DWORD total = frameBytes ? d->size / frameBytes : 0;
    double step = (double)b->frequency / s_outputRate;

    if (total == 0) {
        b->playing = false;
        return;
    }
    for (int i = 0; i < frames; i++) {
        if (b->position >= total) {
            if (!b->loop) {
                b->playing = false;
                b->position = 0.0;
                return;
            }
            b->position = fmod(b->position, (double)total);
        }
        DWORD f0 = (DWORD)b->position;
        DWORD f1 = f0 + 1 < total ? f0 + 1 : (b->loop ? 0 : f0);
        float t = (float)(b->position - f0);
        float l = SampleAt(d, f0, 0) * (1.0f - t) + SampleAt(d, f1, 0) * t;
        float r = SampleAt(d, f0, 1) * (1.0f - t) + SampleAt(d, f1, 1) * t;
        out[i * 2] += l * b->leftGain;
        out[i * 2 + 1] += r * b->rightGain;
        b->position += step;
    }
}

void SDLCALL Feed(void *, SDL_AudioStream *stream, int additional, int)
{
    int frames = additional / (int)(2 * sizeof(float));
    if (frames <= 0)
        return;
    s_mix.assign((size_t)frames * 2, 0.0f);
    s_movieMix.resize((size_t)frames * 2);
    SDL_LockMutex(s_lock);
    for (AudioBuffer *b : s_buffers)
        if (b->playing)
            MixBuffer(b, s_mix.data(), frames);
    for (AudioStream *movie : s_movieStreams) {
        if (!movie->playing)
            continue;
        int bytes = SDL_GetAudioStreamData(movie->converter, s_movieMix.data(), frames * 2 * sizeof(float));
        for (int i = 0; i < bytes / (int)sizeof(float); ++i)
            s_mix[i] += s_movieMix[i];
    }
    SDL_UnlockMutex(s_lock);
    for (float &v : s_mix)
        v = v > 1.0f ? 1.0f : (v < -1.0f ? -1.0f : v);
    SDL_PutAudioStreamData(stream, s_mix.data(), frames * 2 * (int)sizeof(float));
}

struct Lock {
    Lock() { if (s_lock) SDL_LockMutex(s_lock); }
    ~Lock() { if (s_lock) SDL_UnlockMutex(s_lock); }
};

} // namespace

extern "C" BOOL Audio_Init(int sampleRate, int channels)
{
    if (s_stream != NULL)
        return TRUE;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        Sys_Log("audio: %s", SDL_GetError());
        return FALSE;
    }
    s_lock = SDL_CreateMutex();
    s_outputRate = sampleRate > 0 ? sampleRate : 44100;
    SDL_AudioSpec spec = { SDL_AUDIO_F32, 2, s_outputRate };
    s_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, Feed, NULL);
    if (s_stream == NULL) {
        Sys_Log("audio: %s", SDL_GetError());
        return FALSE;
    }
    SDL_ResumeAudioStreamDevice(s_stream);
    return TRUE;
}

extern "C" AudioStream *Audio_CreateStream(DWORD sampleRate, int channels)
{
    if (!sampleRate || (channels != 1 && channels != 2))
        return NULL;
    // Boot movies can precede the game's sound initialisation.
    if (!s_stream && !Audio_Init(44100, 2))
        return NULL;
    SDL_AudioSpec input = { SDL_AUDIO_F32, channels, (int)sampleRate };
    SDL_AudioSpec output = { SDL_AUDIO_F32, 2, s_outputRate };
    SDL_AudioStream *converter = SDL_CreateAudioStream(&input, &output);
    if (!converter)
        return NULL;
    AudioStream *movie = new AudioStream{converter};
    Lock lock;
    s_movieStreams.push_back(movie);
    return movie;
}

extern "C" BOOL Audio_QueueStream(AudioStream *stream, const float *samples, DWORD count)
{
    if (!stream || count > 0x7fffffff / sizeof(float))
        return FALSE;
    Lock lock;
    return SDL_PutAudioStreamData(stream->converter, samples, count * sizeof(float));
}

extern "C" void Audio_StartStream(AudioStream *stream)
{
    if (!stream)
        return;
    Lock lock;
    stream->playing = true;
}

extern "C" void Audio_ReleaseStream(AudioStream *stream)
{
    if (!stream)
        return;
    Lock lock;
    auto it = std::find(s_movieStreams.begin(), s_movieStreams.end(), stream);
    if (it != s_movieStreams.end())
        s_movieStreams.erase(it);
    SDL_DestroyAudioStream(stream->converter);
    delete stream;
}

extern "C" void Audio_Shutdown(void)
{
    if (s_stream != NULL) {
        SDL_DestroyAudioStream(s_stream);
        s_stream = NULL;
    }
    // Buffers the game did not release keep their data until exit.
}

extern "C" AudioBuffer *Audio_CreateBuffer(DWORD sampleRate, int bits, int channels, DWORD bytes)
{
    if ((bits != 8 && bits != 16) || channels < 1 || channels > 2 || bytes == 0)
        return NULL;
    AudioData *d = new AudioData;
    d->bytes = (BYTE *)calloc(1, bytes);
    d->size = bytes;
    d->sampleRate = sampleRate;
    d->bits = bits;
    d->channels = channels;
    d->refs = 1;
    if (bits == 8)
        memset(d->bytes, 0x80, bytes);

    AudioBuffer *b = new AudioBuffer;
    b->data = d;
    b->frequency = sampleRate;
    Lock lock;
    s_buffers.push_back(b);
    return b;
}

extern "C" AudioBuffer *Audio_DuplicateBuffer(AudioBuffer *source)
{
    if (source == NULL)
        return NULL;
    Lock lock;
    AudioBuffer *b = new AudioBuffer;
    b->data = source->data;
    b->data->refs++;
    b->frequency = source->frequency;
    b->volume = source->volume;
    b->pan = source->pan;
    UpdateGains(b);
    s_buffers.push_back(b);
    return b;
}

extern "C" void Audio_ReleaseBuffer(AudioBuffer *buffer)
{
    if (buffer == NULL)
        return;
    Lock lock;
    for (size_t i = 0; i < s_buffers.size(); i++) {
        if (s_buffers[i] == buffer) {
            s_buffers.erase(s_buffers.begin() + i);
            break;
        }
    }
    if (--buffer->data->refs == 0) {
        free(buffer->data->bytes);
        delete buffer->data;
    }
    delete buffer;
}

extern "C" DWORD Audio_GetBufferSize(AudioBuffer *buffer)
{
    return buffer ? buffer->data->size : 0;
}

extern "C" void Audio_GetBufferFormat(AudioBuffer *buffer, DWORD *sampleRate, int *bits, int *channels)
{
    *sampleRate = buffer->data->sampleRate;
    *bits = buffer->data->bits;
    *channels = buffer->data->channels;
}

extern "C" BOOL Audio_LockBuffer(AudioBuffer *buffer, DWORD offset, DWORD bytes, void **p1, DWORD *n1, void **p2,
                                 DWORD *n2)
{
    if (buffer == NULL)
        return FALSE;
    AudioData *d = buffer->data;
    offset %= d->size;
    if (bytes > d->size)
        bytes = d->size;
    *p1 = d->bytes + offset;
    if (offset + bytes <= d->size) {
        *n1 = bytes;
        *p2 = NULL;
        *n2 = 0;
    } else {
        *n1 = d->size - offset;
        *p2 = d->bytes;
        *n2 = bytes - *n1;
    }
    return TRUE;
}

extern "C" void Audio_UnlockBuffer(AudioBuffer *)
{
    // The mixer reads the data in place, as DirectSound's software buffers.
}

extern "C" void Audio_Play(AudioBuffer *buffer, BOOL loop)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->loop = loop != 0;
    buffer->playing = true;
}

extern "C" void Audio_Stop(AudioBuffer *buffer)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->playing = false;
}

extern "C" BOOL Audio_IsPlaying(AudioBuffer *buffer)
{
    if (buffer == NULL)
        return FALSE;
    Lock lock;
    return buffer->playing;
}

extern "C" DWORD Audio_GetPlayPosition(AudioBuffer *buffer)
{
    if (buffer == NULL)
        return 0;
    Lock lock;
    return (DWORD)buffer->position * (DWORD)(buffer->data->bits / 8 * buffer->data->channels);
}

extern "C" void Audio_SetPlayPosition(AudioBuffer *buffer, DWORD position)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->position = (double)(position / (DWORD)(buffer->data->bits / 8 * buffer->data->channels));
}

extern "C" void Audio_SetVolume(AudioBuffer *buffer, LONG volume)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->volume = volume < AUDIO_VOLUME_MIN ? AUDIO_VOLUME_MIN : (volume > 0 ? 0 : volume);
    UpdateGains(buffer);
}

extern "C" void Audio_SetPan(AudioBuffer *buffer, LONG pan)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->pan = pan < AUDIO_PAN_LEFT ? AUDIO_PAN_LEFT : (pan > AUDIO_PAN_RIGHT ? AUDIO_PAN_RIGHT : pan);
    UpdateGains(buffer);
}

extern "C" void Audio_SetFrequency(AudioBuffer *buffer, DWORD frequency)
{
    if (buffer == NULL)
        return;
    Lock lock;
    buffer->frequency = frequency == 0 ? buffer->data->sampleRate : frequency;
}

// ---- MS-ADPCM ---------------------------------------------------------------------

extern "C" DWORD Audio_DecodeMsAdpcm(const AudioWaveFormat *format, const BYTE *src, DWORD srcBytes, BYTE *dst,
                                     DWORD dstBytes)
{
    static const int adaptation[16] = { 230, 230, 230, 230, 307, 409, 512, 614,
                                        768, 614, 512, 409, 307, 230, 230, 230 };
    static const short standardCoefs[7][2] = { { 256, 0 }, { 512, -256 }, { 0, 0 }, { 192, 64 },
                                               { 240, 0 }, { 460, -208 }, { 392, -232 } };
    short coefs[32][2];
    int coefCount = 7;
    int channels = format->nChannels;
    DWORD blockAlign = format->nBlockAlign;
    DWORD written = 0;

    memcpy(coefs, standardCoefs, sizeof(standardCoefs));
    if (format->cbSize >= 4) {
        const BYTE *extra = (const BYTE *)format + sizeof(AudioWaveFormat);
        WORD count = (WORD)(extra[2] | extra[3] << 8);
        if (count >= 7 && count <= 32 && format->cbSize >= 4 + count * 4) {
            coefCount = count;
            for (int i = 0; i < count; i++) {
                coefs[i][0] = (short)(extra[4 + i * 4] | extra[5 + i * 4] << 8);
                coefs[i][1] = (short)(extra[6 + i * 4] | extra[7 + i * 4] << 8);
            }
        }
    }
    if (channels < 1 || channels > 2 || blockAlign < (DWORD)(7 * channels))
        return 0;

    Sint16 *out = (Sint16 *)dst;
    DWORD outMax = dstBytes / 2;
    for (DWORD offset = 0; offset + blockAlign <= srcBytes; offset += blockAlign) {
        const BYTE *p = src + offset;
        int predictor[2], delta[2], s1[2], s2[2];
        for (int c = 0; c < channels; c++)
            predictor[c] = p[c] < coefCount ? p[c] : 0;
        p += channels;
        for (int c = 0; c < channels; c++, p += 2)
            delta[c] = (Sint16)(p[0] | p[1] << 8);
        for (int c = 0; c < channels; c++, p += 2)
            s1[c] = (Sint16)(p[0] | p[1] << 8);
        for (int c = 0; c < channels; c++, p += 2)
            s2[c] = (Sint16)(p[0] | p[1] << 8);
        // The header's two samples come out oldest first.
        for (int c = 0; c < channels && written < outMax; c++)
            out[written++] = (Sint16)s2[c];
        for (int c = 0; c < channels && written < outMax; c++)
            out[written++] = (Sint16)s1[c];
        const BYTE *end = src + offset + blockAlign;
        int channel = 0;
        for (; p < end && written < outMax; p++) {
            for (int half = 0; half < 2 && written < outMax; half++) {
                int nibble = half == 0 ? p[0] >> 4 : p[0] & 0xf;
                int signedNibble = nibble >= 8 ? nibble - 16 : nibble;
                int c = channel;
                int prediction = (s1[c] * coefs[predictor[c]][0] + s2[c] * coefs[predictor[c]][1]) / 256;
                int sample = prediction + signedNibble * delta[c];
                sample = sample > 32767 ? 32767 : (sample < -32768 ? -32768 : sample);
                out[written++] = (Sint16)sample;
                s2[c] = s1[c];
                s1[c] = sample;
                delta[c] = delta[c] * adaptation[nibble] / 256;
                if (delta[c] < 16)
                    delta[c] = 16;
                channel = (channel + 1) % channels;
            }
        }
    }
    return written * 2;
}
