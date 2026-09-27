#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Graphics.h"
#include "Sprite.h"
#include "Game.h"
#include "Sound.h"
#include "main.h"
#include "InstallInfo.h"
#include "FileBuffer.h"

SoundSlot *CSound::m_soundSlots[32];
BOOL CSound::m_unk0x006e0eec;
SoundSlot *CSound::m_soundSlotsEnd;

BOOL CSound::m_unk0x005a2728;
BOOL CSound::m_unk0x005a272c;
BOOL CSound::m_unk0x005a2730;
MMIOData *CSound::m_pMMIO;
IDirectSoundBuffer* CSound::m_pDirectSoundBuffer;
HACMSTREAM CSound::m_unk0x00816a7c;
BOOL CSound::m_unk0x005a2720;
BOOL CSound::m_unk0x005a2724;
BOOL CSound::m_unk0x005a2734 = FALSE;
char CSound::m_unk0x005a2738[256];

// GLOBAL: CMR2 0x00520a3c
char g_strCouldNotCreateStreamingBuffer[36] = "Could not create streaming buffer";
// GLOBAL: CMR2 0x00520a60
char g_strCouldNotOpenAdpcm[44] = "Could not open Microsoft ADPCM Audio CODEC";
// GLOBAL: CMR2 0x00520a8c
char g_strCouldNotOpenMusicFile[28] = "Could not open music file";

BOOL FUN_004a2a20(void);
BOOL FUN_004bd120(void);
void FUN_004a3240(int unused);

// Opens a music file (.wav with Microsoft ADPCM data) and prepares it for
// streaming: creates the streaming buffer and the ACM decoder; on failure the
// music is stopped again. The name is remembered in m_unk0x005a2738.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a28d0
void CSound::FUN_004a28d0(char *path) {
    if (m_unk0x006e0eec == 0)
        return;

    m_unk0x005a2728 = FALSE;
    m_unk0x005a272c = FALSE;
    m_unk0x005a2730 = FALSE;
    if (m_pMMIO != NULL) {
        CloseMMIO(m_pMMIO);
        MMIOData *pMMIO = m_pMMIO;
        if (pMMIO != NULL) {
            CloseAndCleanupMMIO(pMMIO);
            delete pMMIO;
            m_pMMIO = NULL;
        }
    }

    m_pMMIO = new MMIOData();
    if (m_pMMIO->Open(path) != 0) {
        FUN_004a3240((int)g_strCouldNotOpenMusicFile);
    } else {
        m_unk0x005a2728 = TRUE;
        if (FUN_004a3250(FUN_004a2a20()) == 0) {
            FUN_004a3240((int)g_strCouldNotCreateStreamingBuffer);
        } else {
            m_unk0x005a272c = TRUE;
            if (m_unk0x005a2734 == FALSE)
                CGame::RegisterCallback((void *)FUN_004a2ac0, NULL);
            if (FUN_004bd120() == 1) {
                m_unk0x005a2730 = TRUE;
                m_unk0x005a2734 = TRUE;
                strcpy(m_unk0x005a2738, path);
            } else {
                FUN_004a3240((int)g_strCouldNotOpenAdpcm);
            }
        }
    }
    if (m_unk0x005a2730 == FALSE)
        FUN_004a2b50(FALSE);
}

// FUNCTION: CMR2 0x004a2b50
void CSound::FUN_004a2b50(BOOL param1) {
    FUN_004a2ac0();
    if (param1 == 0) {
        m_unk0x005a2734 = FALSE;
        strcpy(m_unk0x005a2738, CMain::m_logFileBlankLine);
    }
}

// FUNCTION: CMR2 0x004a2ac0
BOOL __fastcall CSound::FUN_004a2ac0(void) {
    MMIOData* pvVar1;

    if (m_unk0x005a2728 != 0) {
        if (m_unk0x005a2730 != 0) {
            FUN_004bd230();
        }

        if (m_unk0x005a272c != 0) {
            StopDirectSoundBuffer();
            if (m_pDirectSoundBuffer != NULL) {
                m_pDirectSoundBuffer->Release();
            }

            m_pDirectSoundBuffer = NULL;
        }

        CloseMMIO(m_pMMIO);

        pvVar1 = m_pMMIO;
        if (m_pMMIO != NULL) {
            CloseAndCleanupMMIO(m_pMMIO);
            delete pvVar1;
        }

        m_pMMIO = NULL;
        m_unk0x005a2728 = FALSE;
        m_unk0x005a272c = FALSE;
        m_unk0x005a2730 = FALSE;
    }

    return TRUE;
}

// Converts one 16 KB block of the compressed music stream into pDst.
// FUNCTION: CMR2 0x004bd1b0
bool FUN_004bd1b0(BYTE *pSrc, BYTE *pDst)
{
    ACMSTREAMHEADER header;

    memset(&header, 0, sizeof(header));
    header.pbSrc = pSrc;
    header.cbStruct = sizeof(header);
    header.cbSrcLength = 0x4000;
    header.pbDst = pDst;
    header.cbDstLength = 0xfe80;
    if (acmStreamPrepareHeader(CSound::m_unk0x00816a7c, &header, 0) != 0)
        return false;
    return acmStreamConvert(CSound::m_unk0x00816a7c, &header, 0) == 0;
}

// FUNCTION: CMR2 0x004bd230
bool CSound::FUN_004bd230(void) {
    return acmStreamClose(m_unk0x00816a7c, 0) == 0;
}

// FUNCTION: CMR2 0x004a2f00
HRESULT CSound::StopDirectSoundBuffer(void) {
    DWORD status;

    if (m_unk0x005a2730 != 0) {
        if (m_pDirectSoundBuffer != NULL) {
            m_pDirectSoundBuffer->GetStatus(&status);
            if ((status & DSBSTATUS_PLAYING) != 0)
                FUN_004a3250(m_pDirectSoundBuffer->Stop());
        }

        m_unk0x005a2720 = FALSE;
    }

    return 0;
}


// FUNCTION: CMR2 0x004a3250
BOOL CSound::FUN_004a3250(HRESULT param_1) {
    return param_1 >= 0;
}

// FUNCTION: CMR2 0x004bd960
MMRESULT __fastcall CSound::CloseMMIO(MMIOData* pMMIOData) {
    mmioClose(pMMIOData->hmmio, 0);
    return 0;
}

// FUNCTION: CMR2 0x004bd8c0
void __fastcall CSound::CloseAndCleanupMMIO(MMIOData* pMMIOData) {
    CloseMMIO(pMMIOData);
    
    if (pMMIOData->pBuffer != NULL) {
        delete pMMIOData->pBuffer;
        pMMIOData->pBuffer = NULL;
    }
}

// FUNCTION: CMR2 0x004a31f0
void CSound::FUN_004a31f0(int volume)
{
    LONG vol;

    if (m_unk0x005a2730 != 0) {
        if (volume == 0)
            vol = DSBVOLUME_MIN;
        else
            vol = (volume - 100) * 100 / 4;
        m_pDirectSoundBuffer->SetVolume(vol);
    }
}

// FUNCTION: CMR2 0x004bd250
BOOL CALLBACK AcmFormatEnumCallback(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport)
{
    AcmFindData *pFind = (AcmFindData *)dwInstance;

    if (pafd->dwFormatTag == pFind->wFormatTag &&
        pafd->pwfx->nSamplesPerSec == 44100 &&
        pafd->pwfx->nChannels == 2 &&
        (pafd->pwfx->wBitsPerSample == 16 || pFind->wFormatTag == WAVE_FORMAT_ADPCM))
    {
        pFind->hadid = hadid;
        return FALSE;
    }
    return TRUE;
}

// FUNCTION: CMR2 0x004bd2b0
BOOL CALLBACK AcmDriverEnumCallback(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
    AcmFindData *pFind = (AcmFindData *)dwInstance;
    HACMDRIVER had;
    DWORD cbMaxFormat;
    WAVEFORMATEX *pwfx;
    ACMFORMATDETAILS afd;
    MMRESULT mmr;

    had = NULL;
    if (acmDriverOpen(&had, hadid, 0) == 0) {
        cbMaxFormat = 0;
        acmMetrics((HACMOBJ)had, ACM_METRIC_MAX_SIZE_FORMAT, &cbMaxFormat);
        if (cbMaxFormat < sizeof(WAVEFORMATEX))
            cbMaxFormat = sizeof(WAVEFORMATEX);

        pwfx = (WAVEFORMATEX *)malloc(cbMaxFormat);
        memset(pwfx, 0, cbMaxFormat);
        pwfx->cbSize = (WORD)(cbMaxFormat - sizeof(WAVEFORMATEX));
        pwfx->wFormatTag = pFind->wFormatTag;

        memset(&afd, 0, sizeof(afd));
        afd.cbStruct = sizeof(afd);
        afd.pwfx = pwfx;
        afd.cbwfx = cbMaxFormat;
        afd.dwFormatTag = pFind->wFormatTag;

        mmr = acmFormatEnum(had, &afd, AcmFormatEnumCallback, (DWORD)pFind, 0);
        free(pwfx);
        acmDriverClose(had, 0);

        if (pFind->hadid == NULL && mmr == 0)
            return TRUE;
    }
    return FALSE;
}

// FUNCTION: CMR2 0x004bd3c0
HACMDRIVERID AcmFindDriver(WORD wFormatTag)
{
    AcmFindData find;

    find.hadid = NULL;
    find.wFormatTag = wFormatTag;
    if (acmDriverEnum(AcmDriverEnumCallback, (DWORD)&find, 0) != 0)
        return NULL;
    return find.hadid;
}

// FUNCTION: CMR2 0x004bd400
WAVEFORMATEX *AcmGetDriverFormat(HACMDRIVERID hadid, WORD wFormatTag)
{
    HACMDRIVER had;
    DWORD cbMaxFormat;
    WAVEFORMATEX *pwfx;
    ACMFORMATDETAILS afd;
    AcmFindData find;
    MMRESULT mmr;

    had = NULL;
    if (acmDriverOpen(&had, hadid, 0) == 0) {
        cbMaxFormat = 0;
        acmMetrics((HACMOBJ)had, ACM_METRIC_MAX_SIZE_FORMAT, &cbMaxFormat);
        if (cbMaxFormat < sizeof(WAVEFORMATEX))
            cbMaxFormat = sizeof(WAVEFORMATEX);

        pwfx = (WAVEFORMATEX *)malloc(cbMaxFormat);
        memset(pwfx, 0, cbMaxFormat);
        pwfx->cbSize = (WORD)(cbMaxFormat - sizeof(WAVEFORMATEX));
        pwfx->wFormatTag = wFormatTag;

        memset(&afd, 0, sizeof(afd));
        afd.cbStruct = sizeof(afd);
        afd.pwfx = pwfx;
        afd.cbwfx = cbMaxFormat;
        afd.dwFormatTag = wFormatTag;

        find.wFormatTag = wFormatTag;
        find.hadid = NULL;
        mmr = acmFormatEnum(had, &afd, AcmFormatEnumCallback, (DWORD)&find, 0);
        acmDriverClose(had, 0);

        if (find.hadid != NULL && mmr == 0)
            return pwfx;
        free(pwfx);
    }
    return NULL;
}

// TODO: 86% - identical code, only the block layout of the two cleanup paths differs
// FUNCTION: CMR2 0x004bd520
HRESULT ReadMMIO(HMMIO hmmioIn, MMCKINFO *pckInRIFF, WAVEFORMATEX **ppwfxInfo)
{
    MMCKINFO ckIn;
    PCMWAVEFORMAT pcmWaveFormat;

    *ppwfxInfo = NULL;

    if (0 != mmioDescend(hmmioIn, pckInRIFF, NULL, 0))
        return E_FAIL;

    if (pckInRIFF->ckid != FOURCC_RIFF)
        return E_FAIL;
    if (pckInRIFF->fccType != mmioFOURCC('W', 'A', 'V', 'E'))
        return E_FAIL;

    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (0 != mmioDescend(hmmioIn, &ckIn, pckInRIFF, MMIO_FINDCHUNK))
        return E_FAIL;

    if (ckIn.cksize < (LONG)sizeof(PCMWAVEFORMAT))
        return E_FAIL;

    if (mmioRead(hmmioIn, (HPSTR)&pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
        return E_FAIL;

    if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM) {
        *ppwfxInfo = new WAVEFORMATEX;
        if (NULL == *ppwfxInfo)
            return E_FAIL;

        memcpy(*ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat));
        (*ppwfxInfo)->cbSize = 0;
    } else {
        WORD cbExtraBytes = 0L;
        if (mmioRead(hmmioIn, (CHAR *)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
            return E_FAIL;

        *ppwfxInfo = (WAVEFORMATEX *)new CHAR[sizeof(WAVEFORMATEX) + cbExtraBytes];
        if (NULL == *ppwfxInfo)
            return E_FAIL;

        memcpy(*ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat));
        (*ppwfxInfo)->cbSize = cbExtraBytes;

        if (mmioRead(hmmioIn, (CHAR *)(((BYTE *)&((*ppwfxInfo)->cbSize)) + sizeof(WORD)), cbExtraBytes) != cbExtraBytes)
            goto fail;
    }

    if (0 == mmioAscend(hmmioIn, &ckIn, 0))
        return S_OK;

fail:
    delete *ppwfxInfo;
    *ppwfxInfo = NULL;
    return E_FAIL;
}

// FUNCTION: CMR2 0x004bd6b0
HRESULT WaveOpenFile(LPSTR strFileName, HMMIO *phmmioIn, WAVEFORMATEX **ppwfxInfo, MMCKINFO *pckInRIFF)
{
    HRESULT hr;
    HMMIO hmmioIn;

    hmmioIn = mmioOpen(strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ);
    while (hmmioIn == NULL) {
        if (!CInstallInfo::ShowNoCDErrorMessage())
            return E_FAIL;
        hmmioIn = mmioOpen(strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ);
    }

    if (FAILED(hr = mmioSetBuffer(hmmioIn, NULL, 0x4000, 0)))
        return E_FAIL;

    if (FAILED(hr = ReadMMIO(hmmioIn, pckInRIFF, ppwfxInfo))) {
        mmioClose(hmmioIn, 0);
        return hr;
    }

    *phmmioIn = hmmioIn;
    return S_OK;
}

// FUNCTION: CMR2 0x004bd740
HRESULT WaveStartDataRead(HMMIO *phmmioIn, MMCKINFO *pckIn, MMCKINFO *pckInRIFF, DWORD *pdwSize)
{
    if (-1 == mmioSeek(*phmmioIn, pckInRIFF->dwDataOffset + sizeof(FOURCC), SEEK_SET))
        return E_FAIL;

    pckIn->ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (0 != mmioDescend(*phmmioIn, pckIn, pckInRIFF, MMIO_FINDCHUNK))
        return E_FAIL;

    *pdwSize = pckIn->cksize;
    return S_OK;
}

// FUNCTION: CMR2 0x004bd7b0
HRESULT WaveReadFile(HMMIO hmmioIn, UINT cbRead, BYTE *pbDest, MMCKINFO *pckIn, UINT *cbActualRead)
{
    MMIOINFO mmioinfoIn;
    UINT cbDataIn;
    int cbCopySize;
    int cbLeft;

    *cbActualRead = 0;

    if (0 != mmioGetInfo(hmmioIn, &mmioinfoIn, 0))
        return E_FAIL;

    cbDataIn = cbRead;
    if (cbDataIn > pckIn->cksize)
        cbDataIn = pckIn->cksize;

    pckIn->cksize -= cbDataIn;

    cbLeft = cbDataIn;
    while (cbLeft > 0) {
        if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead) {
            if (0 != mmioAdvance(hmmioIn, &mmioinfoIn, MMIO_READ))
                return E_FAIL;

            if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
                return E_FAIL;
        }

        cbCopySize = mmioinfoIn.pchEndRead - mmioinfoIn.pchNext;
        if (cbLeft <= cbCopySize)
            cbCopySize = cbLeft;
        cbLeft -= cbCopySize;

        memcpy(pbDest, mmioinfoIn.pchNext, cbCopySize);
        pbDest += cbCopySize;
        mmioinfoIn.pchNext += cbCopySize;
    }

    if (0 != mmioSetInfo(hmmioIn, &mmioinfoIn, 0))
        return E_FAIL;

    *cbActualRead = cbDataIn;
    return S_OK;
}

// FUNCTION: CMR2 0x004bd8b0
MMIOData::MMIOData()
{
    pBuffer = NULL;
}

// FUNCTION: CMR2 0x004bd8e0
HRESULT MMIOData::Open(LPSTR strFileName)
{
    HRESULT hr;

    if (pBuffer != NULL) {
        delete pBuffer;
        pBuffer = NULL;
    }

    hr = WaveOpenFile(strFileName, &hmmio, &pBuffer, &ckRiff);
    if (SUCCEEDED(hr))
        hr = StartDataRead();
    return hr;
}

// FUNCTION: CMR2 0x004bd920
HRESULT MMIOData::StartDataRead(void)
{
    return WaveStartDataRead(&hmmio, &ck, &ckRiff, &dwSize);
}

// FUNCTION: CMR2 0x004bd940
HRESULT MMIOData::Read(UINT cbRead, BYTE *pbDest, UINT *pcbRead)
{
    return WaveReadFile(hmmio, cbRead, pbDest, &ck, pcbRead);
}

// Releases the sound data held by one slot and clears the slot.
// FUNCTION: CMR2 0x004b7620
void CSound::FUN_004b7620(int index)
{
    CFileBuffer::FreeGenericFileBuffer(m_soundSlots[index]);
    m_soundSlots[index] = NULL;
}

// Polls the buffer status and (re)starts it with the given play flags.
// FUNCTION: CMR2 0x004a23f0
void CSound::FUN_004a23f0(IDirectSoundBuffer *pBuffer, int flags)
{
    DWORD status;

    status = 0;
    FUN_004a3250(pBuffer->GetStatus(&status));
    FUN_004a3250(pBuffer->Play(0, 0, flags));
}

// Drops a finished sound slot: restarts looping sounds, otherwise releases the
// buffer once the slot stops asking for it.
// FUNCTION: CMR2 0x004a27c0
void CSound::FUN_004a27c0(SoundSlot *pSlot)
{
    DWORD status;
    IDirectSoundBuffer *pBuffer;

    status = 0;
    pBuffer = pSlot->pBuffer;
    if (pBuffer != NULL) {
        FUN_004a3250(pBuffer->GetStatus(&status));
        if (status & 1)
            return;
        if (pSlot->field_0x30 != 0) {
            FUN_004a23f0(pSlot->pLoopBuffer, 1);
            return;
        }
        if (pSlot->field_0x2c != 0) {
            pBuffer = pSlot->pBuffer;
            if (pBuffer != NULL && pBuffer->Release() == 0)
                pSlot->pBuffer = NULL;
        }
        FUN_004b7620(pSlot->id);
    }
}

// FUNCTION: CMR2 0x004a28c0
void CSound::FUN_004a28c0(void)
{
}

// FUNCTION: CMR2 0x004b7b10
void CSound::FUN_004b7b10(void)
{
    FUN_004a28c0();
}

// Stops the shared DirectSound buffer when it is still playing.
// FUNCTION: CMR2 0x004a31a0
void CSound::FUN_004a31a0(void)
{
    DWORD status;

    if (m_unk0x005a2728 != 0 && m_pDirectSoundBuffer != NULL) {
        m_pDirectSoundBuffer->GetStatus(&status);
        if (status & 1) {
            FUN_004a3250(m_pDirectSoundBuffer->Stop());
            m_unk0x005a2720 = 1;
        }
    }
}

// GLOBAL: CMR2 0x00816978
HACMDRIVERID g_unk0x00816978;

// FUNCTION: CMR2 0x004bd100
BOOL FUN_004bd100(void)
{
    HACMDRIVERID id;

    id = AcmFindDriver(2);
    g_unk0x00816978 = id;
    return id != NULL;
}

WAVEFORMATEX *AcmGetDriverFormat(HACMDRIVERID hadid, WORD wFormatTag);

// Opens the ACM stream that decodes the ADPCM music into PCM.
// FUNCTION: CMR2 0x004bd120
BOOL FUN_004bd120(void)
{
    WAVEFORMATEX *pSrc;
    WAVEFORMATEX *pDst;
    HACMDRIVER had;

    CSound::m_unk0x00816a7c = 0;
    pSrc = AcmGetDriverFormat(g_unk0x00816978, 2);
    if (pSrc == NULL)
        return FALSE;
    pDst = AcmGetDriverFormat(g_unk0x00816978, 1);
    if (pDst == NULL)
        return FALSE;
    had = 0;
    if (acmDriverOpen(&had, g_unk0x00816978, 0) != 0)
        return FALSE;
    return acmStreamOpen(&CSound::m_unk0x00816a7c, had, pSrc, pDst, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME) == 0;
}

// FUNCTION: CMR2 0x004a3160
void CSound::FUN_004a3160(void)
{
    if (m_unk0x005a2730 != 0) {
        m_unk0x005a2724 = 1;
        FUN_004a31a0();
    }
}

// GLOBAL: CMR2 0x005a2710
int g_unk0x005a2710;
// GLOBAL: CMR2 0x005a2714
int g_unk0x005a2714;
// GLOBAL: CMR2 0x005a2718
int g_unk0x005a2718;

typedef HRESULT (__stdcall *DPSoundMethod2)(void *pThis, void *a1, void *a2);

// FUNCTION: CMR2 0x004a2d30
void FUN_004a2d30(void)
{
    int lo, hi;
    unsigned int v;

    CSound::FUN_004a3250(((DPSoundMethod2)(*(void ***)CSound::m_pDirectSoundBuffer)[0x10 / 4])(
        CSound::m_pDirectSoundBuffer, &lo, &hi));
    v = (unsigned int)lo / 0xfe80u;
    g_unk0x005a2710 = (int)v;
    v -= g_unk0x005a2714;
    if ((int)v > 0)
        g_unk0x005a2718 = (int)v - 1;
    else
        g_unk0x005a2718 = (int)v + 7;
}

extern int g_unk0x005a271c;

// Decodes `count` 16 KB blocks of the music file into pDst (0xfe80 bytes of
// PCM each). At the end of the file the block is padded to the ADPCM block size
// and the file is rewound, so the music loops.
// FUNCTION: CMR2 0x004a2d90
HRESULT FUN_004a2d90(BYTE *pDst, int count)
{
    BYTE buffer[0x4000];
    UINT read;
    BYTE *pOut;
    UINT pos;
    UINT pad;
    UINT blocks;
    HRESULT hr;
    int i;

    for (i = 0; i < count; i++) {
        pOut = pDst + i * 0xfe80;
        hr = CSound::m_pMMIO->Read(0x4000, buffer, &read);
        if (hr != 0)
            return hr;
        if (read < 0x4000) {
            pos = read;
            do {
                pad = 0x800 - (read & 0x7ff);
                memset(buffer + read, 0, pad);
                pos += pad;
                FUN_004bd1b0(buffer, pOut);
                blocks = (UINT)(read * (1.0f / 2048.0f));
                CSound::FUN_004a3250(CSound::m_pMMIO->StartDataRead());
                CSound::FUN_004a3250(CSound::m_pMMIO->Read(0x4000 - pos, buffer + pos, &read));
                FUN_004bd1b0(buffer, pDst + (blocks + 0xfe80) * i);
                pos += read;
            } while (pos < 0x4000);
        } else {
            FUN_004bd1b0(buffer, pOut);
        }
        g_unk0x005a2714++;
        if (g_unk0x005a2714 == 8)
            g_unk0x005a2714 = 0;
    }
    return 0;
}

// Rewinds the music file and fills the whole streaming buffer from the start.
// FUNCTION: CMR2 0x004a2c70
HRESULT FUN_004a2c70(int unused)
{
    void *pAudio1 = NULL;
    void *pAudio2 = NULL;
    DWORD bytes1;
    DWORD bytes2;

    if (CSound::m_pDirectSoundBuffer == NULL)
        return E_FAIL;
    CSound::m_pMMIO->StartDataRead();
    CSound::m_pDirectSoundBuffer->SetCurrentPosition(0);
    FUN_004a2d30();
    CSound::FUN_004a3250(CSound::m_pDirectSoundBuffer->Lock(0, g_unk0x005a271c, &pAudio1, &bytes1, &pAudio2, &bytes2, 0));
    CSound::FUN_004a3250(FUN_004a2d90((BYTE *)pAudio1, 8));
    CSound::FUN_004a3250(CSound::m_pDirectSoundBuffer->Unlock(pAudio1, bytes1, pAudio2, bytes2));
    return 0;
}

// Restores the streaming buffer if it was lost, then refills it.
// FUNCTION: CMR2 0x004a2f70
HRESULT FUN_004a2f70(int param1)
{
    DWORD status;
    HRESULT hr;

    if (CSound::m_pDirectSoundBuffer != NULL) {
        hr = CSound::m_pDirectSoundBuffer->GetStatus(&status);
        if (hr < 0)
            return hr;
        if (status & DSBSTATUS_BUFFERLOST) {
            do {
                if (CSound::m_pDirectSoundBuffer->Restore() == DSERR_BUFFERLOST)
                    Sleep(10);
            } while (CSound::m_pDirectSoundBuffer->Restore() != 0);
            hr = FUN_004a2c70(param1);
            if (hr < 0)
                return hr;
        }
    }
    return 0;
}

// GLOBAL: CMR2 0x00520aa8
char g_strCouldNotPlayMusicFile[28] = "Could not play music file";
// GLOBAL: CMR2 0x00520ac4
char g_strCouldNotFillMusicBuffer[28] = "Could not fill music buffer";
// GLOBAL: CMR2 0x00520ae0
char g_strCouldNotRestoreMusicBuffer[32] = "Could not restore music buffer";

void FUN_004a3240(int unused);

// Starts playing the opened music from the beginning, looping.
// FUNCTION: CMR2 0x004a2bd0
HRESULT FUN_004a2bd0(int param1)
{
    if (CSound::m_unk0x005a2730 != 0) {
        g_unk0x005a2710 = 0;
        g_unk0x005a2714 = 0;
        CSound::m_unk0x005a2720 = FALSE;
        CSound::m_unk0x005a2724 = FALSE;
        if (CSound::m_pDirectSoundBuffer == NULL)
            return E_FAIL;
        if (CSound::FUN_004a3250(FUN_004a2f70(param1)) == 0)
            FUN_004a3240((int)g_strCouldNotRestoreMusicBuffer);
        if (CSound::FUN_004a3250(FUN_004a2c70(param1)) == 0)
            FUN_004a3240((int)g_strCouldNotFillMusicBuffer);
        if (CSound::FUN_004a3250(CSound::m_pDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING)) == 0)
            FUN_004a3240((int)g_strCouldNotPlayMusicFile);
    }
    return 0;
}

// One chunk (0xfe80 bytes) as a 16.16 fraction; the original's constant block.
// GLOBAL: CMR2 0x00511420
extern const float g_unk0x00511420 = 1.0f / 0xfe80;

// Refills the part of the streaming buffer that has already been played.

// FUNCTION: CMR2 0x004a3050
HRESULT FUN_004a3050(int unused)
{
    void *pAudio1 = NULL;
    void *pAudio2 = NULL;
    DWORD bytes2;
    DWORD bytes1;

    FUN_004a2d30();
    if (g_unk0x005a2718 > 0) {
        if (CSound::m_pDirectSoundBuffer->Lock(g_unk0x005a2714 * 0xfe80, g_unk0x005a2718 * 0xfe80,
                                               &pAudio1, &bytes1, &pAudio2, &bytes2, 0) == 0) {
            if (pAudio1 != NULL)
                CSound::FUN_004a3250(FUN_004a2d90((BYTE *)pAudio1, (UINT)(bytes1 * g_unk0x00511420)));
            if (pAudio2 != NULL)
                CSound::FUN_004a3250(FUN_004a2d90((BYTE *)pAudio2, (UINT)(bytes2 * g_unk0x00511420)));
            CSound::FUN_004a3250(CSound::m_pDirectSoundBuffer->Unlock(pAudio1, bytes1, pAudio2, bytes2));
        }
    }
    return 0;
}

// Per-frame music update: keeps the streaming buffer filled while it plays,
// or restarts it after a pause.
// FUNCTION: CMR2 0x004a2fe0
void FUN_004a2fe0(void)
{
    DWORD status;

    if (CSound::m_unk0x005a2724 == 0 && CSound::m_unk0x005a2730 != 0) {
        if (CSound::m_unk0x005a2720 == 0) {
            CSound::m_pDirectSoundBuffer->GetStatus(&status);
            if (status & DSBSTATUS_PLAYING) {
                FUN_004a3050(1);
                CSound::m_unk0x005a2720 = FALSE;
                return;
            }
        } else {
            CSound::FUN_004a3250(CSound::m_pDirectSoundBuffer->Play(0, 0, DSBPLAY_LOOPING));
        }
        CSound::m_unk0x005a2720 = FALSE;
    }
}

// FUNCTION: CMR2 0x004a2430
int FUN_004a2430(SoundSlot *pSlot)
{
    DWORD status1 = 0;
    DWORD status2 = 0;
    int result;

    CSound::FUN_004a3250(pSlot->pBuffer->GetStatus(&status1));
    if (pSlot->pLoopBuffer != NULL)
        CSound::FUN_004a3250(pSlot->pLoopBuffer->GetStatus(&status2));
    result = 1;
    if ((status1 & 1) == 0) {
        if (pSlot->field_0x30 == 0)
            result = 0;
    }
    return result;
}

// Per-sample 3D interfaces and buffers loaded by the sound bank.
// GLOBAL: CMR2 0x005a1fc8
IDirectSound3DBuffer *g_sound3DBuffers[200];
// GLOBAL: CMR2 0x005a23ec
IDirectSoundBuffer *g_soundBuffers[200];
// GLOBAL: CMR2 0x005a283c
BOOL g_unk0x005a283c;

extern IDirectSound *g_unk0x005a2844;
void FUN_004a2690(SoundSlot *pSlot);
BOOL FUN_004b75c0(void);
int Sound_GetMasterVolume(void);
SoundSlot *Sound_GetSlot(int index);

// The original imported DirectSoundCreate from DSOUND.dll (the SilentPatch exe
// routes it through SPCMR2.dll ordinal 1).
#pragma comment(lib, "third_party/dx7sdk-7001/lib/dsound.lib")

// 3D sound enabled (primary buffer with CTRL3D and a listener)
// GLOBAL: CMR2 0x005a2838
BOOL g_sound3DEnabled;
// Speaker configuration read from DirectSound (DSSPEAKER_*)
// GLOBAL: CMR2 0x00520890
DWORD g_soundSpeakerConfig = DSSPEAKER_STEREO;

extern IDirectSoundBuffer *g_unk0x005a2848;
extern int g_unk0x005a284c;
BOOL FUN_004bd100(void);

// Creates the DirectSound device and sets the format of the primary buffer
// (16-bit PCM at sampleRate; mono when the speakers are mono). bits and
// unused are ignored: the original always uses 16 bits.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a1d60
BOOL Sound_InitDevice(int sampleRate, int channels, int bits, int unused)
{
    IDirectSoundBuffer *pPrimary;
    WAVEFORMATEX format;
    DSBUFFERDESC desc;

    pPrimary = NULL;
    if (!CSound::FUN_004a3250(DirectSoundCreate(NULL, &g_unk0x005a2844, NULL)))
        return FALSE;
    if (!CSound::FUN_004a3250(g_unk0x005a2844->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DSSCL_PRIORITY)))
        return FALSE;

    if (FUN_004b75c0() && (g_unk0x005a2844->GetSpeakerConfig(&g_soundSpeakerConfig), (BYTE)g_soundSpeakerConfig == DSSPEAKER_MONO))
        channels = 1;
    if (channels == 1)
        g_sound3DEnabled = FALSE;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = g_unk0x005a283c ? DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE : DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;
    if (g_sound3DEnabled)
        desc.dwFlags |= DSBCAPS_CTRL3D;
    desc.dwBufferBytes = 0;
    desc.lpwfxFormat = NULL;

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = (WORD)channels;
    format.nSamplesPerSec = sampleRate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = (WORD)(channels * 2);
    format.nAvgBytesPerSec = format.nBlockAlign * sampleRate;
    format.cbSize = 0;

    if (!CSound::FUN_004a3250(g_unk0x005a2844->CreateSoundBuffer(&desc, &pPrimary, NULL)))
        return FALSE;
    if (!CSound::FUN_004a3250(pPrimary->SetFormat(&format)))
        return FALSE;
    if (g_sound3DEnabled) {
        if (!CSound::FUN_004a3250(pPrimary->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&g_unk0x005a2848)))
            return FALSE;
        g_unk0x005a284c = 1;
    }

    FUN_004bd100();
    CSound::m_unk0x005a2734 = FALSE;
    strcpy(CSound::m_unk0x005a2738, CMain::m_logFileBlankLine);
    CSound::m_pMMIO = NULL;
    if (pPrimary != NULL)
        pPrimary->Release();
    return TRUE;
}

// GLOBAL: CMR2 0x00520a34
char g_strWave[8] = "WAVE";

int FUN_004b7780(void);
BOOL FUN_004a20c0(IDirectSound *pDS, IDirectSoundBuffer **ppBuffer, DWORD rate, int bits, int channels, int is3D,
                  DWORD size);
BOOL FUN_004a2210(IDirectSoundBuffer *pBuffer, DWORD offset, void *pData, DWORD size);

// Loads a .wav from pFile into the next free sample slot: creates its buffer
// (a 3D one when flags & 1 and 3D sound is on) and copies the PCM data.
// The file buffer is freed unless it lives inside the archive.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a1f50
BOOL Sound_LoadWave(char *name, BYTE flags, GenericFile *pFile)
{
    BYTE *pWave;
    BYTE inArchive;
    int channels, bits;
    DWORD rate;
    BYTE *pData;
    int slot;

    pWave = (BYTE *)CGenericFileLoader::FindFile(pFile, name, &inArchive, NULL, 0);
    if (pWave == NULL)
        return FALSE;

    if (strncmp((char *)pWave + 8, g_strWave, 4) == 0) {
        channels = *(WORD *)(pWave + 0x16);
        rate = *(DWORD *)(pWave + 0x18);
        bits = *(WORD *)(pWave + 0x22);
        pData = pWave + 0x2c;
        if ((flags & 1) == 0 || !g_sound3DEnabled) {
            if (!FUN_004a20c0(g_unk0x005a2844, &g_soundBuffers[FUN_004b7780()], rate, bits, channels, 0,
                              *(DWORD *)(pWave + 0x28)))
                return FALSE;
            FUN_004a2210(g_soundBuffers[FUN_004b7780()], 0, pData, *(DWORD *)(pWave + 0x28));
        } else {
            if (!FUN_004a20c0(g_unk0x005a2844, &g_soundBuffers[FUN_004b7780()], rate, bits, channels, 1,
                              *(DWORD *)(pWave + 0x28)))
                return FALSE;
            slot = FUN_004b7780();
            if (CSound::FUN_004a3250(g_soundBuffers[slot]->QueryInterface(IID_IDirectSound3DBuffer,
                                                                            (LPVOID *)&g_sound3DBuffers[FUN_004b7780()]))) {
                if (!FUN_004a2210(g_soundBuffers[FUN_004b7780()], 0, pData, *(DWORD *)(pWave + 0x28)))
                    return FALSE;
                CSound::FUN_004a3250(g_sound3DBuffers[FUN_004b7780()]->SetMode(DS3DMODE_NORMAL, DS3D_DEFERRED));
            }
        }
    }
    if (inArchive == 0)
        CFileBuffer::FreeGenericFileBuffer(pWave);
    return TRUE;
}

// Creates a PCM buffer of the given format (3D buffers use the HRTF light
// algorithm on Windows 98 and later).
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The 3D algorithm and null GUIDs come from the SDK; the original keeps them at
// these addresses (same values).
// GLOBAL: CMR2 0x00511ca8
// DS3DALG_HRTF_LIGHT
// GLOBAL: CMR2 0x00513ea8
// GUID_NULL

// FUNCTION: CMR2 0x004a20c0
BOOL FUN_004a20c0(IDirectSound *pDS, IDirectSoundBuffer **ppBuffer, DWORD rate, int bits, int channels, int is3D,
                  DWORD size)
{
    PCMWAVEFORMAT format = {0};
    DSBUFFERDESC desc;

    format.wf.nSamplesPerSec = rate;
    format.wf.nBlockAlign = (WORD)channels * bits / 8;
    format.wf.nAvgBytesPerSec = format.wf.nBlockAlign * rate;
    memset(&desc, 0, sizeof(desc));
    format.wf.wFormatTag = WAVE_FORMAT_PCM;
    format.wf.nChannels = channels;
    format.wBitsPerSample = bits;
    desc.dwSize = sizeof(desc);
    if (g_unk0x005a283c && is3D)
        desc.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
    else
        desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
    if (channels == 2)
        desc.dwFlags |= DSBCAPS_CTRLPAN;
    if (is3D) {
        desc.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
        if (FUN_004b75c0())
            desc.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
        else
            desc.guid3DAlgorithm = GUID_NULL;
    }
    desc.dwBufferBytes = size;
    desc.lpwfxFormat = (LPWAVEFORMATEX)&format;
    return CSound::FUN_004a3250(pDS->CreateSoundBuffer(&desc, ppBuffer, NULL)) != 0;
}

// Copies size bytes of data into the buffer at the given offset.
// FUNCTION: CMR2 0x004a2210
BOOL FUN_004a2210(IDirectSoundBuffer *pBuffer, DWORD offset, void *pData, DWORD size)
{
    void *p1;
    DWORD n1;
    void *p2;
    DWORD n2;

    if (pBuffer->Lock(offset, size, &p1, &n1, &p2, &n2, 0) == DS_OK) {
        if (p1 != NULL)
            memcpy(p1, pData, n1);
        if (p2 != NULL)
            memcpy(p2, (BYTE *)pData + n1, n2);
        if (pBuffer->Unlock(p1, n1, p2, n2) == DS_OK)
            return TRUE;
    }
    return FALSE;
}

// Builds the looping buffer of the slot from the part of the sample after
// the loop start.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a24a0
void FUN_004a24a0(SoundSlot *pSlot)
{
    DSBCAPS caps = {0};
    WAVEFORMATEX format = {0};
    void *p1;
    DWORD n1;
    void *p2;
    DWORD n2;

    caps.dwSize = sizeof(caps);
    CSound::FUN_004a3250(pSlot->pBuffer->GetCaps(&caps));
    CSound::FUN_004a3250(pSlot->pBuffer->GetFormat(&format, sizeof(format), NULL));
    CSound::FUN_004a3250(pSlot->pBuffer->Lock(0, caps.dwBufferBytes, &p1, &n1, &p2, &n2, 0));
    if (FUN_004a20c0(g_unk0x005a2844, &pSlot->pLoopBuffer, format.nSamplesPerSec, format.wBitsPerSample,
                     format.nChannels, pSlot->field_0x14, n1 - pSlot->field_0x18))
        FUN_004a2210(pSlot->pLoopBuffer, 0, (BYTE *)p1 + pSlot->field_0x18, n1 - pSlot->field_0x18);
    CSound::FUN_004a3250(pSlot->pBuffer->Unlock(p1, n1, p2, n2));
    if (pSlot->field_0x14 != 0 &&
        CSound::FUN_004a3250(pSlot->pLoopBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void **)&pSlot->field_0x28)))
        ((IDirectSound3DBuffer *)pSlot->field_0x28)->SetMode(DS3DMODE_NORMAL, DS3D_DEFERRED);
}

// The scale factors of the logarithmic attenuation, from the original's block.
// GLOBAL: CMR2 0x00511418
extern const float g_unk0x00511418 = 1.0f / 65536.0f;
// GLOBAL: CMR2 0x00511410
extern const double g_unk0x00511410 = -10.0;
// GLOBAL: CMR2 0x005113c0
extern const double g_unk0x005113c0 = 10.0;

// Applies the slot volume (scaled by the master volume) as a logarithmic
// attenuation in hundredths of a decibel.
// FUNCTION: CMR2 0x004a25f0
void FUN_004a25f0(SoundSlot *pSlot)
{
    int volume;
    int attenuation;

    volume = (int)((float)Sound_GetMasterVolume() * pSlot->field_0xc * g_unk0x00511418);
    attenuation = DSBVOLUME_MIN -
                  (int)(log((double)volume) * g_unk0x00511410) * abs(10000) / (int)(log(CGraphics::m_65536) * g_unk0x005113c0);
    CSound::FUN_004a3250(pSlot->pBuffer->SetVolume(attenuation));
    if (pSlot->pLoopBuffer != NULL)
        CSound::FUN_004a3250(pSlot->pLoopBuffer->SetVolume(attenuation));
}

// Gives the slot a buffer for its sample (a duplicate when the sample is
// already playing), sets it up and starts it.
// FUNCTION: CMR2 0x004a22c0
int FUN_004a22c0(SoundSlot *pSlot)
{
    IDirectSoundBuffer *pSource;
    SoundSlot *pOther;
    DWORD status;
    int flags;
    int shared;
    int i;

    i = 0;
    flags = 0;
    status = 0;
    pSource = g_soundBuffers[pSlot->sampleId];
    if (pSource == NULL)
        return 0;
    if (pSlot->field_0x10 != 0 && pSlot->field_0x18 == 0)
        flags = DSBPLAY_LOOPING;
    CSound::FUN_004a3250(pSource->GetStatus(&status));
    shared = 0;
    for (i = 0; i < 32; i++) {
        pOther = Sound_GetSlot(i);
        if (pOther != NULL && pOther->sampleId == pSlot->sampleId && pOther->id != pSlot->id) {
            shared = 1;
            break;
        }
    }
    if ((status & DSBSTATUS_PLAYING) || shared) {
        CSound::FUN_004a3250(g_unk0x005a2844->DuplicateSoundBuffer(g_soundBuffers[pSlot->sampleId], &pSlot->pBuffer));
        pSlot->field_0x2c = 1;
    } else {
        pSlot->pBuffer = g_soundBuffers[pSlot->sampleId];
    }
    if (pSlot->field_0x14 != 0) {
        if (pSlot->field_0x2c != 0)
            pSlot->pBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void **)&pSlot->field_0x20);
        else
            pSlot->field_0x20 = (IDirectSoundBuffer *)g_sound3DBuffers[pSlot->sampleId];
    }
    if (pSlot->field_0x10 != 0 && pSlot->field_0x18 != 0)
        FUN_004a24a0(pSlot);
    FUN_004a25f0(pSlot);
    FUN_004a2690(pSlot);
    CSound::FUN_004a23f0(pSlot->pBuffer, flags);
    return 1;
}

// FUNCTION: CMR2 0x004a2690
void FUN_004a2690(SoundSlot *pSlot)
{
    if (pSlot->field_0xa < 100)
        pSlot->field_0xa = 100;
    if (pSlot->field_0xa > 100000)
        pSlot->field_0xa = 34464;
    CSound::FUN_004a3250(pSlot->pBuffer->SetFrequency(pSlot->field_0xa));
    if (pSlot->pLoopBuffer != NULL)
        CSound::FUN_004a3250(pSlot->pLoopBuffer->SetFrequency(pSlot->field_0xa));
}

// GLOBAL: CMR2 0x005a271c
int g_unk0x005a271c;
// GLOBAL: CMR2 0x005a2844
IDirectSound *g_unk0x005a2844;


// Stops the slot's buffers and releases the looping ones.
// FUNCTION: CMR2 0x004a26f0
void FUN_004a26f0(SoundSlot *pSlot)
{
    DWORD status;

    if (pSlot->pBuffer == NULL)
        return;
    if (pSlot->field_0x30 != 0) {
        CSound::FUN_004a3250(pSlot->pLoopBuffer->Stop());
        status = 0;
        pSlot->pLoopBuffer->GetStatus(&status);
        if ((status & 1) == 0 && pSlot->pLoopBuffer != NULL) {
            if (pSlot->pLoopBuffer->Release() == 0)
                pSlot->pLoopBuffer = NULL;
        }
    }
    CSound::FUN_004a3250(pSlot->pBuffer->Stop());
    pSlot->pBuffer->SetCurrentPosition(0);
    if (pSlot->field_0x2c != 0 && pSlot->pBuffer != NULL) {
        if (pSlot->pBuffer->Release() == 0)
            pSlot->pBuffer = NULL;
    }
    if (pSlot->field_0x14 != 0) {
        if (pSlot->field_0x2c != 0 && pSlot->field_0x20 != NULL) {
            if (pSlot->field_0x20->Release() == 0)
                pSlot->field_0x20 = NULL;
        }
        if (pSlot->field_0x28 != NULL) {
            if (pSlot->field_0x28->Release() == 0)
                pSlot->field_0x28 = NULL;
        }
    }
}

int Sound_FindFreeSlot(void);
unsigned int Sound_MakeHandle(unsigned int index, unsigned short serial);
int Sound_FindHandle(unsigned int handle);
void Sound_SetPan(unsigned int handle, unsigned short pan);

// Whether the system is Windows 98 / NT 5 or later.
// FUNCTION: CMR2 0x004b75c0
BOOL FUN_004b75c0(void)
{
    OSVERSIONINFOA info;

    info.dwOSVersionInfoSize = sizeof(info);
    if (GetVersionExA(&info) &&
        (info.dwMajorVersion > 4 || (info.dwMajorVersion == 4 && info.dwMinorVersion > 0)))
        return TRUE;
    return FALSE;
}

// Sets the volume of a playing sound.
// FUNCTION: CMR2 0x004b79a0
void FUN_004b79a0(unsigned int handle, int volume)
{
    int index;

    index = Sound_FindHandle(handle);
    if (index != -1 && CSound::m_soundSlots[index]->field_0xc != volume) {
        CSound::m_soundSlots[index]->field_0xc = volume;
        FUN_004a25f0(CSound::m_soundSlots[index]);
    }
}

// Starts a sound: takes a free slot, fills it and plays it. Returns the slot
// handle, or -1 when no slot is free or the sample cannot be played.
// FUNCTION: CMR2 0x004b7790
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D)
{
    int index;
    unsigned int handle;

    index = Sound_FindFreeSlot();
    if (index == -1 || id >= 200)
        return -1;
    handle = Sound_MakeHandle(index, id);
    memset(CSound::m_soundSlots[index] = (SoundSlot *)CFileBuffer::AllocateLockedBuffer(sizeof(SoundSlot)), 0,
           sizeof(SoundSlot));
    CSound::m_soundSlots[index]->sampleId = id;
    CSound::m_soundSlots[index]->id = index;
    CSound::m_soundSlots[index]->handle = handle;
    CSound::m_soundSlots[index]->field_0x10 = loops;
    CSound::m_soundSlots[index]->field_0x14 = is3D;
    CSound::m_soundSlots[index]->field_0x18 = loopStart;
    CSound::m_soundSlots[index]->field_0x30 = loopStart != 0;
    CSound::m_soundSlots[index]->field_0xc = volume;
    CSound::m_soundSlots[index]->field_0xa = frequency;
    if (!FUN_004a22c0(CSound::m_soundSlots[index])) {
        CFileBuffer::FreeGenericFileBuffer(CSound::m_soundSlots[index]);
        CSound::m_soundSlots[index] = NULL;
        return -1;
    }
    FUN_004b79a0(handle, volume);
    Sound_SetPan(handle, frequency);
    return handle;
}

// GLOBAL: CMR2 0x005210f4
int g_soundMasterVolume = 0x10000;
// GLOBAL: CMR2 0x006e0ef0
int g_unk0x006e0ef0;

int FUN_004a2430(SoundSlot *pSlot);
int Sound_FindHandle(unsigned int handle);
void FUN_004a26f0(SoundSlot *pSlot);
void FUN_004a2690(SoundSlot *pSlot);

// FUNCTION: CMR2 0x004b7610
SoundSlot *Sound_GetSlot(int index)
{
    return CSound::m_soundSlots[index];
}

void Sound_FreeAll(void);
void FUN_004a1d10(int sample);

// Frees every loaded sound and releases the samples from index first on.
// GLOBAL: CMR2 0x005210f8
char g_strFailedToLoad[20] = "Failed to load \"%s\"";

int FUN_004b7ae0(void);

// Starts the sound system: clears the sound slots, creates the DirectSound
// device and registers the shutdown callback.
// FUNCTION: CMR2 0x004b7650
BOOL Sound_Init(int sampleRate, int channels, int bits, int unused)
{
    int i;

    if (CSound::m_unk0x006e0eec != 0)
        return FALSE;
    g_soundMasterVolume = 0x10000;
    for (i = 0; i < 32; i++)
        CSound::m_soundSlots[i] = NULL;
    if (!Sound_InitDevice(sampleRate, channels, bits, unused))
        return FALSE;
    CGame::RegisterCallback(FUN_004b7ae0, NULL);
    CSound::m_unk0x006e0eec = 1;
    return TRUE;
}

// Loads a sample into the next slot (at most 200).
// FUNCTION: CMR2 0x004b76c0
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile)
{
    char message[260];

    if (CSound::m_unk0x006e0eec == 0 || (unsigned int)g_unk0x006e0ef0 >= 200)
        return FALSE;
    if (Sound_LoadWave(name, flags, pFile)) {
        g_unk0x006e0ef0++;
        return TRUE;
    }
    // the original formats an error message nobody reads
    sprintf(message, g_strFailedToLoad, name);
    return FALSE;
}

// FUNCTION: CMR2 0x004b7740
void FUN_004b7740(int first)
{
    int count = g_unk0x006e0ef0;

    if (CSound::m_unk0x006e0eec) {
        Sound_FreeAll();
        for (; first < g_unk0x006e0ef0; first++) {
            FUN_004a1d10(first);
            count--;
        }
        g_unk0x006e0ef0 = count;
    }
}

// FUNCTION: CMR2 0x004b7780
int FUN_004b7780(void)
{
    return g_unk0x006e0ef0;
}

// FUNCTION: CMR2 0x004b78a0
int Sound_IsPlaying(unsigned int handle)
{
    int index = Sound_FindHandle(handle);
    if (index != -1)
        return FUN_004a2430(CSound::m_soundSlots[index]);
    return 0;
}

// FUNCTION: CMR2 0x004b78d0
void Sound_Free(unsigned int handle)
{
    int index = Sound_FindHandle(handle);
    if (index != -1) {
        FUN_004a26f0(CSound::m_soundSlots[index]);
        CFileBuffer::FreeGenericFileBuffer(CSound::m_soundSlots[index]);
        CSound::m_soundSlots[index] = NULL;
    }
}

// FUNCTION: CMR2 0x004b7910
void Sound_FreeAll(void)
{
    SoundSlot **pp = CSound::m_soundSlots;

    do {
        if (*pp != NULL) {
            FUN_004a26f0(*pp);
            CFileBuffer::FreeGenericFileBuffer(*pp);
            *pp = NULL;
        }
        pp++;
    } while ((int)pp < (int)&CSound::m_soundSlotsEnd);
}

// FUNCTION: CMR2 0x004b7940
int FUN_004b7940(void)
{
    return g_unk0x006e0ef0;
}

// FUNCTION: CMR2 0x004b7990
int Sound_GetMasterVolume(void)
{
    return g_soundMasterVolume;
}

// FUNCTION: CMR2 0x004b79e0
void Sound_SetPan(unsigned int handle, unsigned short pan)
{
    int index = Sound_FindHandle(handle);
    if (index != -1) {
        CSound::m_soundSlots[index]->field_0xa = pan;
        FUN_004a2690(CSound::m_soundSlots[index]);
    }
}

// FUNCTION: CMR2 0x004b7a10
int Sound_GetVolume(unsigned int handle)
{
    int index = Sound_FindHandle(handle);
    if (index != -1)
        return CSound::m_soundSlots[index]->field_0xc;
    return 0;
}

// FUNCTION: CMR2 0x004b7a70
int Sound_FindFreeSlot(void)
{
    int i;
    SoundSlot **pp;

    for (i = 0, pp = CSound::m_soundSlots; (int)pp < (int)&CSound::m_soundSlotsEnd; pp++, i++) {
        if (*pp == NULL)
            return i;
    }
    return -1;
}

// FUNCTION: CMR2 0x004b7a90
unsigned int Sound_MakeHandle(unsigned int index, unsigned short serial)
{
    return (serial & 0xffff) << 8 | index & 0x3f;
}

// FUNCTION: CMR2 0x004b7ab0
int Sound_FindHandle(unsigned int handle)
{
    if ((int)handle >= 0 && CSound::m_soundSlots[handle & 0x3f] != NULL &&
        CSound::m_soundSlots[handle & 0x3f]->handle == handle)
        return handle & 0x3f;
    return -1;
}

// FUNCTION: CMR2 0x004a1d00
IDirectSound *FUN_004a1d00(void)
{
    return g_unk0x005a2844;
}

// FUNCTION: CMR2 0x004a3180
void FUN_004a3180(void)
{
    if (CSound::m_unk0x005a2730 != 0)
        CSound::m_unk0x005a2724 = 0;
}

// FUNCTION: CMR2 0x004a3240
void FUN_004a3240(int unused)
{
}


// Sets the master volume (0..1) and re-applies it to every sound slot.
// FUNCTION: CMR2 0x004b7950
void Sound_SetMasterVolume(int volume)
{
    SoundSlot **ppSlot;

    if (volume >= 0 && volume <= 0x10000)
        g_soundMasterVolume = volume;
    ppSlot = CSound::m_soundSlots;
    do {
        if (*ppSlot != NULL)
            FUN_004a25f0(*ppSlot);
        ppSlot++;
    } while ((int)ppSlot < (int)&CSound::m_soundSlotsEnd);
}

// Releases the (3D) buffers of a sample.
// FUNCTION: CMR2 0x004a1d10
void FUN_004a1d10(int sample)
{
    if (g_soundBuffers[sample] != NULL) {
        if (g_soundBuffers[sample]->Release() == 0)
            g_soundBuffers[sample] = NULL;
    }
    if (g_sound3DBuffers[sample] != NULL) {
        if (g_sound3DBuffers[sample]->Release() == 0)
            g_sound3DBuffers[sample] = NULL;
    }
}

// GLOBAL: CMR2 0x005a2848
IDirectSoundBuffer *g_unk0x005a2848;
// GLOBAL: CMR2 0x005a284c
int g_unk0x005a284c;

int FUN_004b7780(void);

// Releases every sample buffer, the primary buffer and DirectSound itself.
// FUNCTION: CMR2 0x004a2830
void FUN_004a2830(void)
{
    int i;

    for (i = 0; i < FUN_004b7780(); i++) {
        if (g_soundBuffers[i] != NULL && g_soundBuffers[i]->Release() == 0)
            g_soundBuffers[i] = NULL;
        if (g_sound3DBuffers[i] != NULL && g_sound3DBuffers[i]->Release() == 0)
            g_sound3DBuffers[i] = NULL;
    }
    if (g_unk0x005a2848 != NULL && g_unk0x005a2848->Release() == 0)
        g_unk0x005a2848 = NULL;
    if (g_unk0x005a2844 != NULL && g_unk0x005a2844->Release() == 0)
        g_unk0x005a2844 = NULL;
    g_unk0x005a284c = 0;
}

// Creates the shared 16-bit stereo 44.1 kHz streaming buffer.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a2a20
BOOL FUN_004a2a20(void)
{
    WAVEFORMATEX format;
    DSBUFFERDESC desc;

    memset(&desc, 0, sizeof(desc));
    format.cbSize = 0;
    desc.lpwfxFormat = &format;
    g_unk0x005a271c = 0x7f400;
    desc.dwBufferBytes = 0x7f400;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME;
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 44100;
    format.nBlockAlign = 4;
    format.nAvgBytesPerSec = 176400;
    format.wBitsPerSample = 16;
    return CSound::FUN_004a3250(g_unk0x005a2844->CreateSoundBuffer(&desc, &CSound::m_pDirectSoundBuffer, NULL)) != 0;
}

void FUN_004a2830(void);

// Shuts the sound system down (registered callback of 0x4b7650).
// FUNCTION: CMR2 0x004b7ae0
int FUN_004b7ae0(void)
{
    Sound_FreeAll();
    Sound_SetMasterVolume(0);
    FUN_004a2830();
    g_unk0x006e0ef0 = 0;
    CSound::m_unk0x006e0eec = 0;
    return 1;
}

// Not analysed yet: when the slot at 0x5a2734 exists it re-enables the 3D
// listener (FUN_004a28d0 on the buffer at 0x5a2738), re-applies the volume
// from FUN_00405e40() through FUN_004a31f0 and calls FUN_004a2bd0(1).
// Called right after the Direct3D device is created.
// STUB: CMR2 0x004a2ba0
void FUN_004a2ba0(void)
{
}

