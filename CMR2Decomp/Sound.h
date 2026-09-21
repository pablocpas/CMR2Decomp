#ifndef _SOUND_H
#define _SOUND_H

#include <windows.h>
#include <MMREG.H>
#include <MMSYSTEM.H>
#include <MSACM.H>
#include <DSOUND.H>
#include <stdlib.h>
#include <string.h>

// Wave file reader, modelled on wave.c / CWaveFile from the DirectX SDK samples
struct MMIOData {
    WAVEFORMATEX *pBuffer;  // Offset 0x0
    HMMIO hmmio;            // Offset 0x4
    MMCKINFO ck;            // Offset 0x8
    MMCKINFO ckRiff;        // Offset 0x1c
    DWORD dwSize;           // Offset 0x30

    MMIOData();
    void Open(LPSTR strFileName);
    void StartDataRead(void);
    void Read(UINT cbRead, BYTE *pbDest, UINT *pcbRead);
};

// Data passed through the ACM enumeration callbacks
struct AcmFindData {
    HACMDRIVERID hadid;
    WORD wFormatTag;
};

HRESULT ReadMMIO(HMMIO hmmioIn, MMCKINFO *pckInRIFF, WAVEFORMATEX **ppwfxInfo);
HRESULT WaveOpenFile(LPSTR strFileName, HMMIO *phmmioIn, WAVEFORMATEX **ppwfxInfo, MMCKINFO *pckInRIFF);
HRESULT WaveStartDataRead(HMMIO *phmmioIn, MMCKINFO *pckIn, MMCKINFO *pckInRIFF, DWORD *pdwSize);
HRESULT WaveReadFile(HMMIO hmmioIn, UINT cbRead, BYTE *pbDest, MMCKINFO *pckIn, UINT *cbActualRead);

BOOL CALLBACK AcmFormatEnumCallback(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport);
BOOL CALLBACK AcmDriverEnumCallback(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport);
HACMDRIVERID AcmFindDriver(WORD wFormatTag);
WAVEFORMATEX *AcmGetDriverFormat(HACMDRIVERID hadid, WORD wFormatTag);


// One sound slot; CSound::FUN_004a27c0 stops/releases it when its buffer has
// finished playing.
struct SoundSlot {
    unsigned short id;                  // 0x00 index into CSound::m_soundSlots
    BYTE field_0x2[0x1a];
    IDirectSoundBuffer *pBuffer;        // 0x1c
    BYTE field_0x20[0x4];
    IDirectSoundBuffer *pLoopBuffer;    // 0x24 restarted while field_0x30 is set
    BYTE field_0x28[0x4];
    int field_0x2c;                     // 0x2c release pBuffer when set
    int field_0x30;                     // 0x30 looping
};

class CSound {
public:
    static void FUN_004a23f0(IDirectSoundBuffer *pBuffer, int flags);
    static void FUN_004a27c0(SoundSlot *pSlot);
    static void FUN_004b7620(int index);

    static BOOL __fastcall FUN_004a2ac0(void);
    static void FUN_004a2b50(BOOL param1);
    static bool FUN_004bd230(void);
    static HRESULT StopDirectSoundBuffer(void);
    static bool FUN_004a3250(HRESULT param1);
    static MMRESULT __fastcall CloseMMIO(MMIOData* hhmio);
    static void __fastcall CloseAndCleanupMMIO(MMIOData* pMMIO);
    static void FUN_004a31f0(int volume);
    

    // GLOBAL: CMR2 0x005a23e8
    static MMIOData *m_pMMIO;

    // GLOBAL: CMR2 0x005a2720
    static BOOL m_unk0x005a2720;

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
    static IDirectSoundBuffer* m_pDirectSoundBuffer;

    // GLOBAL: CMR2 0x00816a7c
    static HACMSTREAM m_unk0x00816a7c;

    // GLOBAL: CMR2 0x006e0d6c
    static SoundSlot *m_soundSlots[32];

    // GLOBAL: CMR2 0x006e0eec
    static BOOL m_unk0x006e0eec;

    // GLOBAL: CMR2 0x006e0dec
    static SoundSlot *m_soundSlotsEnd;
};

#endif
