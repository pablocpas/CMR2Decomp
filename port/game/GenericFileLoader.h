#ifndef _GENERIC_FILE_LOADER_H
#define _GENERIC_FILE_LOADER_H


struct GenericFile
{
    void *buffer;
    unsigned int fileSize;
    BOOL didFileLoad;
};

class CGenericFileLoader
{
public:
    // GLOBAL: CMR2 0x00818250
    static GenericFile m_genericFile;
    // GLOBAL: CMR2 0x00663fe8
    static DWORD m_fileSize;

    // TODO: should this not be a static class? seems silly to pass this in like this
    static bool LoadIntoFileRecord(GenericFile *file, char *fileName);
    static bool ReadIntoFileRecord(char *fileName, GenericFile *param_2);
    static int GetGenericFileSize(void);
    static GenericFile* GetGenericFile(void);
    static void *FindFileInArchive(GenericFile *pFile, char *name, DWORD *pId);
    static void *FindFile(GenericFile *pFile, char *pPath, BYTE *pFound, DWORD *pId, BYTE bSkipArchive);
    static void GetFileNameFromPath(char *path, char *out);
    static BYTE *StrUpperPolish(BYTE *str);
    static char *StrLowerPolish(char *str);
};

#endif
