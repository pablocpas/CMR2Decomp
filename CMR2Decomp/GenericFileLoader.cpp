#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include <string.h>

GenericFile CGenericFileLoader::m_genericFile;
DWORD CGenericFileLoader::m_fileSize;

// FUNCTION: CMR2 0x004a9d70
bool CGenericFileLoader::FUN_004a9d70(GenericFile *file, char *fileName)
{
    file->didFileLoad = FALSE;
    if (FUN_004a9c30(fileName, file) != FALSE)
        return file->didFileLoad = TRUE;

    return false;
}

// FUNCTION: CMR2 0x004a9c30
bool CGenericFileLoader::FUN_004a9c30(char *fileName, GenericFile *param_2)
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
        memcpy(entryName, pEntry + 12, nameLen);
        entryName[nameLen] = '\0';
        if (strcmp(entryName, name) == 0)
            return (BYTE *)pFile->buffer + offset;
        pad = (int)nameLen % 4;
        if (pad == 0)
            pEntry += nameLen + 12;
        else
            pEntry += nameLen - pad + 16;
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
        pName = path;
        if (pSlash != NULL)
            pName = pSlash + 1;
    }
    strcpy(out, pName);
}

// FUNCTION: CMR2 0x004a9f90
BYTE *CGenericFileLoader::StrUpperPolish(BYTE *str)
{
    BYTE *p;
    BYTE c;

    c = *str;
    p = str;
    while (c != 0) {
        if (((char)c > '`' && (char)c < '{') || ((char)c > -33 && c > 0x7f))
            *p = c - 0x20;
        switch (c) {
        case 0xa3: *p = 0x7e; break;
        case 0xa9: *p = 0x24; break;
        case 0xae: *p = 0x2a; break;
        case 0xb1: *p = 0x7c; break;
        case 0xbc: *p = 0xf7; break;
        case 0xbd: *p = 0x7b; break;
        case 0xbe: *p = 0x7d; break;
        case 0xd7: *p = 0x99; break;
        case 0xf3: *p = 0xd3; break;
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
        if ((c > '@' && c < '[') || (c > -65 && c < -32))
            *p = c + ' ';
        switch (c) {
        case '$': *p = -0x57; break;
        case '*': *p = -0x52; break;
        case '{': *p = -0x43; break;
        case '|': *p = -0x4f; break;
        case '}': *p = -0x42; break;
        case '~': *p = -0x5d; break;
        case -0x67: *p = -0x29; break;
        case -0x2d: *p = -0xd; break;
        case -9: *p = -0x44; break;
        }
        p++;
        c = *p;
    }
    return str;
}
