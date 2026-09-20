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


class CSound {
public:
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
};

#endif
