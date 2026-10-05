#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include <string.h>

GenericFile CGenericFileLoader::m_genericFile;
DWORD CGenericFileLoader::m_fileSize;

// FUNCTION: CMR2 0x004a9d70
bool CGenericFileLoader::LoadIntoFileRecord(GenericFile *file, char *fileName)
{
    file->didFileLoad = FALSE;
    if (ReadIntoFileRecord(fileName, file) != FALSE)
        return file->didFileLoad = TRUE;

    return false;
}

// FUNCTION: CMR2 0x004a9c30
bool CGenericFileLoader::ReadIntoFileRecord(char *fileName, GenericFile *param_2)
{
    param_2->buffer = CFileBuffer::GetGenericFileBuffer(fileName, 0);
    if (param_2->buffer != NULL)
    {
        param_2->fileSize = GetGenericFileSize();
        return true;
    }

    param_2->fileSize = 0;
    return false;
}

// FUNCTION: CMR2 0x004aa590
int CGenericFileLoader::GetGenericFileSize(void)
{
    return m_fileSize;
}

// FUNCTION: CMR2 0x004d21a0
GenericFile* CGenericFileLoader::GetGenericFile(void) {
    return &m_genericFile;
}

// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a9c70
void *CGenericFileLoader::FindFileInArchive(GenericFile *pFile, char *name, DWORD *pId)
{
    BYTE *pEntry;
    int offset;
    unsigned int nameLen;
    unsigned int pad;
    char entryName[260];

    _strlwr(name);
    pEntry = (BYTE *)pFile->buffer + *(int *)((BYTE *)pFile->buffer + pFile->fileSize - 4);
    while (pEntry < (BYTE *)pFile->buffer + pFile->fileSize - 4) {
        if (pId != NULL)
            *pId = *(DWORD *)pEntry;
        nameLen = *(DWORD *)(pEntry + 8);
        offset = *(int *)(pEntry + 4);
        pEntry += 12;
        memcpy(entryName, pEntry, nameLen);
        entryName[nameLen] = '\0';
        if (strcmp(entryName, name) == 0)
            return (BYTE *)pFile->buffer + offset;
        pad = (int)nameLen % 4;
        if (pad != 0)
            pEntry += nameLen - pad + 4;
        else
            pEntry += nameLen;
    }
    return NULL;
}

// FUNCTION: CMR2 0x004a9da0
void CGenericFileLoader::GetFileNameFromPath(char *path, char *out)
{
    char *pName;
    char *pSlash;

    pName = strrchr(path, '/');
    if (pName == NULL) {
        pSlash = strrchr(path, '\\');
        if (pSlash == NULL)
            pSlash = path;
        else
            pSlash++;
        pName = pSlash;
    }
    strcpy(out, pName);
}

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004a9f90
BYTE *CGenericFileLoader::StrUpperPolish(BYTE *str)
{
    BYTE *p;
    char c;

    c = *str;
    p = str;
    while (c != 0) {
        if ((c >= 'a' && c <= 'z') || (c >= (char)0xE0 && c <= (char)0xFF))
            *p = c - 0x20;
        switch (c) {
        case -0x5d: *p = 0x7e; break;
        case -0x57: *p = 0x24; break;
        case -0x52: *p = 0x2a; break;
        case -0x4f: *p = 0x7c; break;
        case -0x44: *p = 0xf7; break;
        case -0xd: *p = 0xd3; break;
        case -0x43: *p = 0x7b; break;
        case -0x42: *p = 0x7d; break;
        case -0x29: *p = 0x99; break;
        }
        p++;
        c = *p;
    }
    return str;
}

// FUNCTION: CMR2 0x004aa090
char *CGenericFileLoader::StrLowerPolish(char *str)
{
    char *p;
    char c;

    c = *str;
    p = str;
    while (c != '\0') {
        if ((c >= 'A' && c <= 'Z') || (c >= (char)0xC0 && c <= (char)0xDF))
            *p = c + ' ';
        switch (c) {
        case '~': *p = -0x5d; break;
        case '$': *p = -0x57; break;
        case '*': *p = -0x52; break;
        case '|': *p = -0x4f; break;
        case -9: *p = -0x44; break;
        case -0x2d: *p = -0xd; break;
        case '{': *p = -0x43; break;
        case '}': *p = -0x42; break;
        case -0x67: *p = -0x29; break;
        }
        p++;
        c = *p;
    }
    return str;
}

// Looks a path up inside the archive held by pFile; *pFound is set to 1
// when it was found there. bSkipArchive forces a miss.
// FUNCTION: CMR2 0x004a9df0
void *CGenericFileLoader::FindFile(GenericFile *pFile, char *pPath, BYTE *pFound, DWORD *pId, BYTE bSkipArchive)
{
    void *pResult;
    char name[64];

    if (pFound != NULL)
        *pFound = 0;
    if (pFile != NULL && pFile->didFileLoad != 0 && bSkipArchive == 0) {
        GetFileNameFromPath(pPath, name);
        pResult = FindFileInArchive(pFile, name, pId);
        if (pResult != NULL) {
            if (pFound != NULL)
                *pFound = 1;
            return pResult;
        }
    }
    if (pId != NULL)
        *pId = 0;
    return NULL;
}
