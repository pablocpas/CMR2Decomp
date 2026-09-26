#include "FileBuffer.h"
#include "GenericFileLoader.h"
#include "Graphics.h"
#include "InstallInfo.h"
#include "main.h"
#include "BFL.h"

#include <stdio.h>
#include "zlib/zlib.h"

char CFileBuffer::m_unk0x00520f1c[4] = "rb"; // gzopen mode
int CFileBuffer::m_unk0x0066461c;
int CFileBuffer::m_unk0x00664620;

// FUNCTION: CMR2 0x004aad70
void *CFileBuffer::AllocateLockedBuffer(size_t iSize)
{
    HANDLE handle;
    void *buffer;

    handle = GlobalAlloc(0x42, iSize);
    buffer = GlobalLock(handle);
    return buffer;
}

// FUNCTION: CMR2 0x004aad90
void *CFileBuffer::ReallocateLockedBuffer(void *buffer, size_t iSize)
{
    HGLOBAL handle;
    UINT flags;

    if (buffer != NULL)
    {
        handle = GlobalHandle(buffer);
        GlobalUnlock(handle);
        flags = 0;
        handle = GlobalReAlloc(GlobalHandle(buffer), iSize, flags);
        return GlobalLock(handle);
    }

    handle = AllocateLockedBuffer(iSize);
    return handle;
}

// FUNCTION: CMR2 0x004aa220
void *CFileBuffer::GetGenericFileBuffer(char *fileName, BOOL isLocalFile)
{
    // isLocalFile is basically is it on the HDD? this function is called by loadnetworkleaderboards,loadcontrollerconfig,etc. with param2 as 1
    void *unk0x004bdee0, *lpBuffer;
    BFLHeader pFileHeaderOut;
    DWORD fileAttributes, fileSize, fileSizeRead;
    size_t iHeaderSize;
    bool bIsBFL;
    HANDLE hFile;
    char _fileName[MAX_PATH];
    // Graphics *pGraphics;
    IDirectDraw7 *pDD7;

    lpBuffer = NULL;
    bIsBFL = FALSE;
    sprintf(_fileName, fileName);

    unk0x004bdee0 = gzopen(_fileName, m_unk0x00520f1c);
    if (!unk0x004bdee0)
    {
        if (!isLocalFile && g_pGraphics && g_pGraphics->pDD7 != NULL)
        {
            // an install/CD file: ask for the CD until it can be opened
            g_pGraphics->pDD7->FlipToGDISurface();
            ShowCursor(TRUE);

            do
            {
                if (CInstallInfo::ShowNoCDErrorMessage())
                    unk0x004bdee0 = gzopen(_fileName, m_unk0x00520f1c);
            } while (!unk0x004bdee0);

            ShowCursor(FALSE);
            ShowWindow(CMain::m_hWndList[CMain::m_hWndIx], SW_RESTORE);
        }
        else
        {
            if (g_pGraphics && g_pGraphics->pDD7)
                g_pGraphics->pDD7->FlipToGDISurface();

            // make sure file exists
            fileAttributes = GetFileAttributesA(_fileName);
            if (fileAttributes == -1)
                return NULL;

            // retry counter; the original discards the retry's result and
            // carries on with the failed handle (gzread then fails and it
            // falls back to CreateFileA below)
            if (++m_unk0x0066461c >= 50)
                return NULL;
            GetGenericFileBuffer(fileName, isLocalFile);
        }
    }

    // check if its a BFL
    iHeaderSize = gzread(unk0x004bdee0, &pFileHeaderOut, 8);
    if (iHeaderSize == 8U && pFileHeaderOut.ident[0] == 0x43 && pFileHeaderOut.ident[1] == 0x4d && pFileHeaderOut.ident[2] == 0x50 && pFileHeaderOut.ident[3] == 0x52)
    {
        if (pFileHeaderOut.archiveSize == INVALID_FILE_SIZE)
            return NULL;

        lpBuffer = AllocateLockedBuffer(pFileHeaderOut.archiveSize);
        CGenericFileLoader::m_fileSize = pFileHeaderOut.archiveSize;
        gzread(unk0x004bdee0, lpBuffer, pFileHeaderOut.archiveSize);
        bIsBFL = true;
    }

    gzclose(unk0x004bdee0);

    if (!bIsBFL)
    {
        hFile = CreateFileA(_fileName, GENERIC_READ, 1, NULL, 3, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            if (g_pGraphics && g_pGraphics->pDD7)
                g_pGraphics->pDD7->FlipToGDISurface();

            fileAttributes = GetFileAttributesA(_fileName);
            if (fileAttributes == -1)
                return NULL;

            // another retry counter?
            // i swear this needs to return but the original asm doesn't have a huge RET block here
            // whereas the new ASM does if this returns. returning here makes way more sense though
            if (50 > ++m_unk0x00664620)
                return GetGenericFileBuffer(_fileName, isLocalFile);
            else
                return NULL;
        }

        fileSize = GetFileSize(hFile, NULL);
        CGenericFileLoader::m_fileSize = fileSize;

        if (fileSize == INVALID_FILE_SIZE)
            return NULL;

        lpBuffer = AllocateLockedBuffer(fileSize);
        ReadFile(hFile, lpBuffer, fileSize, &fileSizeRead, NULL);
        if (fileSizeRead != fileSize)
            return NULL;

        CloseHandle(hFile);
    }

    return lpBuffer;
}

// FUNCTION: CMR2 0x004aade0
void CFileBuffer::FreeGenericFileBuffer(void *param1)
{
    HANDLE handle;

    handle = GlobalHandle(param1);
    GlobalUnlock(handle);
    handle = GlobalHandle(param1);
    GlobalFree(handle);
}

// GLOBAL: CMR2 0x00531650
int g_unk0x00531650;
// One value per player, cleared by FUN_004eadb0
// GLOBAL: CMR2 0x00531654
int g_unk0x00531654[4];
// GLOBAL: CMR2 0x00531764
BYTE *g_unk0x00531764;
// CMR2 0x00818cd0: generic file load flag, cleared together with the load counter
// GLOBAL: CMR2 0x00818cd0
BYTE g_unk0x00818cd0;

// GLOBAL: CMR2 0x00525258
char g_strPpsPathFormat[32] = "%s\\pps\\%s%.2d%.2d%.2d%.2d.pps";
// GLOBAL: CMR2 0x00818acc
char g_ppsPath[260];

extern BYTE g_saveData[];

// Builds the path of a player profile save: <hd>\pps\<name><date>.pps.
// The packed dword after the 4-byte name holds the date fields.
// FUNCTION: CMR2 0x004eb2e0
char *FUN_004eb2e0(char *pName)
{
    unsigned int packed = *(unsigned int *)(pName + 4);

    // the original passes one value more than the format uses
    sprintf(g_ppsPath, g_strPpsPathFormat, CInstallInfo::GetGameHDPath(), pName,
            packed >> 0x10 & 0x1f, packed >> 8 & 0xf, packed & 0xff, packed >> 0xc & 0xf,
            packed >> 0x16 & 0x3f);
    return g_ppsPath;
}

// Writes a 0x650-byte player profile to its .pps file.
// TODO: CMR2 0x004eb340 (implemented, match 43%)
BYTE FUN_004eb340(int unused, BYTE *pProfile)
{
    CInstallInfo::WriteFileToDisk(FUN_004eb2e0((char *)pProfile + 0x10), 0, pProfile, 0x650);
    return 1;
}

// FUNCTION: CMR2 0x004eb450
BYTE *FUN_004eb450(int index)
{
    return g_unk0x00531764 + index * 12;
}

// Saves the profiles whose name has just been edited, clearing the dirty flag
// of the ones written; returns whether all of them were saved. Profiles with
// bit 0x200000 set are left alone.
// TODO: CMR2 0x004eb3e0 (implemented, match 58%)
bool FUN_004eb3e0(void)
{
    bool saved = true;
    int i = 0;
    BYTE *pProfile = g_saveData + 0x638;

    do {
        if ((*(unsigned int *)(pProfile + 4) & 0x200000) == 0 && *pProfile != 0 &&
            g_saveData[0x620 + i] != 0) {
            BYTE result = FUN_004eb340(0, pProfile - 0x10);
            if (result != 0)
                g_saveData[0x620 + i] = 0;
            if (saved && result != 0)
                saved = true;
            else
                saved = false;
        }
        pProfile += 0x650;
        i++;
    } while ((int)pProfile < (int)(g_saveData + 0x1f78));
    return saved;
}

// FUNCTION: CMR2 0x004eb440
int FUN_004eb440(void)
{
    return g_unk0x00531650;
}

// Saves every player profile marked dirty (unless it has flag 0x200000).
// TODO: CMR2 0x004eb470 (implemented, match 52%)
void FUN_004eb470(void)
{
    BYTE *pProfile;
    int i;

    i = 0;
    pProfile = g_saveData + 0x628;
    do {
        if ((*(unsigned int *)(pProfile + 0x14) & 0x200000) == 0 && g_saveData[0x620 + i] != 0) {
            FUN_004eb340(0, pProfile);
            g_saveData[0x620 + i] = 0;
        }
        pProfile += 0x650;
        i++;
    } while (pProfile < g_saveData + 0x1f68);
}

// FUNCTION: CMR2 0x004eb4b0
void FUN_004eb4b0(char *param1, int param2)
{
    CFileBuffer::GetGenericFileBuffer(param1, 1);
}

// Resets record `index` of the 0x531350 table to category 0xf, clearing bit 0x2000.
// TODO: CMR2 0x004ebe80 (implemented, match 41%)
void FUN_004ebe80(int index)
{
    unsigned int *pRecord = (unsigned int *)(g_saveData + 0x1f70 + index * 0x30);
    unsigned int value = *pRecord;

    if ((value & 0x3c0000) != 0x3c0000)
        *pRecord = value & 0xffffdfff | 0x3c0000;
}

// FUNCTION: CMR2 0x004ebec0
void FUN_004ebec0(void)
{
    int i;

    i = 0;
    do {
        FUN_004ebe80(i);
        i++;
    } while (i < 0x10);
}

void RallyData_ValidateIndex(int index);
void FUN_004ec260(int player);

// Gives record `index` a profile (category) unless it has one: `profile`
// if given; otherwise the first free profile no earlier record uses; else
// the free profile with the lowest age, or when they all have the same age
// the one unused for longest.
// TODO: CMR2 0x004eb860 (implemented, match 51%)
void FUN_004eb860(int index, int profile)
{
    unsigned int *pRecord;
    unsigned int flags;
    unsigned int age;
    unsigned int minAge;
    unsigned int firstAge;
    unsigned int best;
    unsigned int now;
    int free[4];
    int count;
    int chosen;
    int c;
    int i;
    BYTE *p;
    BYTE *pRec;
    char allSame;

    RallyData_ValidateIndex(index);
    pRecord = (unsigned int *)(g_saveData + 0x1f70 + index * 0x30);
    if ((*pRecord & 0x3c0000) != 0x3c0000)
        return;
    if (profile != -1) {
        flags = *(unsigned int *)(g_saveData + 0x67c + profile * 0x650);
        *pRecord = ((flags & 0x40) << 1 | profile & 0xf) << 0x12 | *pRecord & 0xfdc3ffc0 | flags & 0x1f;
        return;
    }
    c = 0;
    for (p = g_saveData + 0x63c; p < g_saveData + 0x1f7c; p += 0x650, c++) {
        if (p[-4] == 0 || (*(unsigned int *)p & 0x200000))
            break;
    }
    if (p < g_saveData + 0x1f7c && c != -1) {
        for (i = 0; i < index; i++) {
            if ((*(unsigned int *)(g_saveData + 0x1f70 + i * 0x30) >> 0x12 & 0xf) == (unsigned int)c)
                c = -1;
        }
        if (c != -1) {
            chosen = c;
            goto assign;
        }
    }
    count = 0;
    for (c = 0; c < 4; c++) {
        free[count] = -1;
        for (pRec = g_saveData + 0x1f70; pRec < g_saveData + 0x2270; pRec += 0x30) {
            if ((*(unsigned int *)pRec >> 0x12 & 0xf) == (unsigned int)c)
                break;
        }
        if (pRec >= g_saveData + 0x2270) {
            free[count] = c;
            count++;
        }
    }
    minAge = 0xff;
    allSame = 1;
    firstAge = *(unsigned int *)(g_saveData + 0x67c + free[0] * 0x650) >> 7 & 0xff;
    chosen = index;
    for (i = 0; i < count; i++) {
        age = *(unsigned int *)(g_saveData + 0x67c + free[i] * 0x650) >> 7 & 0xff;
        if (firstAge != age)
            allSame = 0;
        if (age <= minAge) {
            minAge = age;
            chosen = free[i];
        }
    }
    now = CMain::GetFrameDelta();
    if (allSame) {
        best = 0;
        for (i = 0; i < count; i++) {
            if (best <= now - g_unk0x00531654[free[i]]) {
                best = now - g_unk0x00531654[free[i]];
                chosen = free[i];
            }
        }
    }
assign:
    *pRecord = (*pRecord & 0xffc3ffff) | (chosen & 0xf) << 0x12;
    g_unk0x00531654[*pRecord >> 0x12 & 0xf] = CMain::GetFrameDelta();
}

// Clears the profile of record `index`'s category back to a new profile.
// FUNCTION: CMR2 0x004ebf20
void FUN_004ebf20(int index)
{
    unsigned int category;

    RallyData_ValidateIndex(index);
    category = *(unsigned int *)(g_saveData + 0x1f70 + index * 0x30) >> 0x12 & 0xf;
    memset(g_saveData + 0x628 + category * 0x650, 0, 0x650);
    *(int *)(g_saveData + 0x680 + category * 0x650) = 4;
    *(unsigned int *)(g_saveData + 0x63c + category * 0x650) =
        (*(unsigned int *)(g_saveData + 0x63c + category * 0x650) & 0xffe1f17e) | 0x1017e;
    *(unsigned int *)(g_saveData + 0x67c + category * 0x650) |= 0x20;
    *(int *)(g_saveData + 0x674 + category * 0x650) = 0xf11;
    g_saveData[0xbb0 + category * 0x650] = (g_saveData[0xbb0 + category * 0x650] & 0xfc) | 0x3c;
    FUN_004ec260(index);
    g_saveData[0x620 + (*(unsigned int *)(g_saveData + 0x1f70 + index * 0x30) >> 0x12 & 0xf)] = 0;
}

// 12-byte block read from the file buffer (at offset 0x10).
struct Unk0x10Block {
    int field_0x0;
    int field_0x4;
    int field_0x8;
};

// Reads the file and copies the 12 bytes at offset 0x10 into *pOut.
// FUNCTION: CMR2 0x004ebee0
BOOL FUN_004ebee0(Unk0x10Block *pOut, char *param2)
{
    void *pBuffer;

    pBuffer = CFileBuffer::GetGenericFileBuffer(param2, 1);
    if (pBuffer == NULL)
        return FALSE;
    *pOut = *(Unk0x10Block *)((char *)pBuffer + 0x10);
    CFileBuffer::FreeGenericFileBuffer(pBuffer);
    return TRUE;
}


// Releases the current generic file buffer and resets the load state.
// FUNCTION: CMR2 0x004eb6d0
char FUN_004eb6d0(void)
{
    if (g_unk0x00531764 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00531764);
        g_unk0x00531764 = NULL;
    }
    g_unk0x00531650 = 0;
    g_unk0x00818cd0 = 0;
    return 1;
}

