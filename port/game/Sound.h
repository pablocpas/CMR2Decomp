#ifndef _SOUND_H
#define _SOUND_H

#include <stdlib.h>
#include <string.h>
#include "port/audio.h"
#include "port/sys.h"

// GLOBAL: CMR2 0x00511c38
// IID_IDirectSound3DBuffer

// PORT: a RIFF chunk position (the fields of MMCKINFO the reader uses).
struct WaveChunk {
    DWORD ckid;
    DWORD cksize;
    DWORD fccType;
    DWORD dwDataOffset;
    DWORD dwFlags;
};

// Wave file reader, modelled on wave.c / CWaveFile from the DirectX SDK samples
// PORT: reads through Sys instead of mmio.
struct MMIOData {
    AudioWaveFormat *pBuffer;  // Offset 0x0
    SysFile *hmmio;            // Offset 0x4
    WaveChunk ck;              // Offset 0x8
    WaveChunk ckRiff;          // Offset 0x1c
    DWORD dwSize;              // Offset 0x30

    MMIOData();
    HRESULT Open(LPSTR strFileName);
    HRESULT StartDataRead(void);
    HRESULT Read(UINT cbRead, BYTE *pbDest, UINT *pcbRead);
};

HRESULT ReadMMIO(SysFile *hmmioIn, WaveChunk *pckInRIFF, AudioWaveFormat **ppwfxInfo);
HRESULT WaveOpenFile(LPSTR strFileName, SysFile **phmmioIn, AudioWaveFormat **ppwfxInfo, WaveChunk *pckInRIFF);
HRESULT WaveStartDataRead(SysFile **phmmioIn, WaveChunk *pckIn, WaveChunk *pckInRIFF, DWORD *pdwSize);
HRESULT WaveReadFile(SysFile *hmmioIn, UINT cbRead, BYTE *pbDest, WaveChunk *pckIn, UINT *cbActualRead);

// PORT: the ACM lookup functions; the decoder is built in.
int AcmFindDriver(WORD wFormatTag);
AudioWaveFormat *AcmGetDriverFormat(int hadid, WORD wFormatTag);


// One sound slot; CSound::UpdateFinishedSoundSlot stops/releases it when its buffer has
// finished playing.
struct SoundSlot {
    unsigned short id;                  // 0x00 index into CSound::m_soundSlots
    BYTE field_0x2[0x2];
    unsigned int handle;                // 0x04 id | serial << 8, see Sound_MakeHandle
    unsigned short sampleId;            // 0x08 index into g_soundBuffers
    unsigned short field_0xa;           // 0x0a frequency, clamped to [100, 100000]
    int field_0xc;                      // 0x0c volume scale
    int field_0x10;                     // 0x10 loops
    int field_0x14;                     // 0x14 3D sound, released when the slot is reset
    int field_0x18;                     // 0x18 loop start offset in bytes
    AudioBuffer *pBuffer;               // 0x1c
    AudioBuffer *field_0x20;            // 0x20 PORT: unused (3D interface of pBuffer)
    AudioBuffer *pLoopBuffer;           // 0x24 restarted while field_0x30 is set
    AudioBuffer *field_0x28;            // 0x28 PORT: unused (3D interface of pLoopBuffer)
    int field_0x2c;                     // 0x2c release pBuffer when set
    int field_0x30;                     // 0x30 looping
};

class CSound {
public:
    static void EnsureBufferPlaying(AudioBuffer *pBuffer, int flags);
    static void NoOpSoundDeviceCallback(void);
    static void StopSharedMusicBuffer(void);
    static void RunSoundDeviceCallback(void);
    static void PauseMusicStreaming(void);
    static void UpdateFinishedSoundSlot(SoundSlot *pSlot);
    static void ReleaseSoundSlotData(int index);

    static BOOL __fastcall CloseMusicStreamResources(void);
    static void OpenStreamingMusicFile(char *path);
    static void CloseMusicStreamAndClearPath(BOOL param1);
    static bool CloseADPCMDecoder(void);
    static HRESULT StopDirectSoundBuffer(void);
    static BOOL IsSoundCallSuccessful(HRESULT param1);
    static UINT __fastcall CloseMMIO(MMIOData* hhmio);
    static void __fastcall CloseAndCleanupMMIO(MMIOData* pMMIO);
    static void SetMusicStreamVolume(int volume);
    

    // GLOBAL: CMR2 0x005a23e8
    static MMIOData *m_pMMIO;

    // GLOBAL: CMR2 0x005a2720
    static BOOL m_unk0x005a2720;
    // GLOBAL: CMR2 0x005a2724
    static BOOL m_unk0x005a2724;

    // GLOBAL: CMR2 0x005a2728
    static BOOL m_unk0x005a2728;

    // GLOBAL: CMR2 0x005a272c
    static BOOL m_unk0x005a272c;

    // GLOBAL: CMR2 0x005a2730
    static BOOL m_unk0x005a2730;

    // GLOBAL: CMR2 0x005a2734
    static BOOL m_unk0x005a2734;

    // GLOBAL: CMR2 0x005a2738
    static char m_unk0x005a2738[256];    

    // GLOBAL: CMR2 0x005a2854
    static AudioBuffer* m_pDirectSoundBuffer;

    // GLOBAL: CMR2 0x00816a7c
    static int m_unk0x00816a7c;     // PORT: 1 while the music decoder is open

    // GLOBAL: CMR2 0x006e0d6c
    static SoundSlot *m_soundSlots[32];

    // GLOBAL: CMR2 0x006e0eec
    static BOOL m_unk0x006e0eec;

    // GLOBAL: CMR2 0x006e0dec
    static SoundSlot *m_soundSlotsEnd;
};

int Sound_PlaySampleWithParameters(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);

struct GenericFile;
BOOL Sound_InitDevice(int sampleRate, int channels, int bits, int unused);
BOOL Sound_LoadWave(char *name, BYTE flags, GenericFile *pFile);
BOOL Sound_Init(int sampleRate, int channels, int bits, int unused);
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);

#endif
