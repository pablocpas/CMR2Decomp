#include "Sound.h"
#include "main.h"
#include "InstallInfo.h"

BOOL CSound::m_unk0x005a2728;
BOOL CSound::m_unk0x005a272c;
BOOL CSound::m_unk0x005a2730;
MMIOData *CSound::m_pMMIO;
IDirectSoundBuffer* CSound::m_pDirectSoundBuffer;
HACMSTREAM CSound::m_unk0x00816a7c;
BOOL CSound::m_unk0x005a2720;
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
bool CSound::FUN_004a3250(HRESULT param_1) {
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
