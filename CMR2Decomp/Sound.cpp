#include <math.h>
#include <stdlib.h>
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
void MMIOData::Open(LPSTR strFileName)
{
    if (pBuffer != NULL) {
        delete pBuffer;
        pBuffer = NULL;
    }

    if (SUCCEEDED(WaveOpenFile(strFileName, &hmmio, &pBuffer, &ckRiff)))
        StartDataRead();
}

// FUNCTION: CMR2 0x004bd920
void MMIOData::StartDataRead(void)
{
    WaveStartDataRead(&hmmio, &ck, &ckRiff, &dwSize);
}

// FUNCTION: CMR2 0x004bd940
void MMIOData::Read(UINT cbRead, BYTE *pbDest, UINT *pcbRead)
{
    WaveReadFile(hmmio, cbRead, pbDest, &ck, pcbRead);
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

// TODO: CMR2 0x004a2d30 (implemented, match 84%)
void FUN_004a2d30(void)
{
    int lo, hi;
    unsigned int v;

    CSound::FUN_004a3250(((DPSoundMethod2)(*(void ***)CSound::m_pDirectSoundBuffer)[0x10 / 4])(
        CSound::m_pDirectSoundBuffer, &lo, &hi));
    v = (unsigned int)lo / 1000u;
    g_unk0x005a2710 = (int)v;
    v -= g_unk0x005a2714;
    if ((int)v > 0)
        g_unk0x005a2718 = (int)v - 1;
    else
        g_unk0x005a2718 = (int)v + 7;
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

// Creates a PCM buffer of the given format (3D buffers use the HRTF light
// algorithm on Windows 98 and later).
// TODO: CMR2 0x004a20c0 (implemented, match 84%)
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
// TODO: CMR2 0x004a24a0 (implemented, match 85%)
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

// Applies the slot volume (scaled by the master volume) as a logarithmic
// attenuation in hundredths of a decibel.
// TODO: CMR2 0x004a25f0 (implemented, match 91%)
void FUN_004a25f0(SoundSlot *pSlot)
{
    int volume;
    int attenuation;

    volume = (int)((float)Sound_GetMasterVolume() * pSlot->field_0xc * (1.0f / 65536.0f));
    attenuation = DSBVOLUME_MIN -
                  (int)(log((double)volume) * -10.0) * abs(DSBVOLUME_MIN) / (int)(log(65536.0) * 10.0);
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
// TODO: CMR2 0x004b7950 (implemented, match 83%)
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
    } while (ppSlot < &CSound::m_soundSlots[32]);
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
