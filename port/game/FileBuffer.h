#ifndef _FILE_BUFFER_H
#define _FILE_BUFFER_H

#include "BFL.h"

class CFileBuffer
{
public:
    static void *AllocateLockedBuffer(size_t size);
    static void *ReallocateLockedBuffer(void *buffer, size_t iSize);
    static void *GetGenericFileBuffer(char *fileName, BOOL param2);
    static void FreeGenericFileBuffer(void *buffer);

    // GLOBAL: CMR2 0x00520f1c
    static char m_unk0x00520f1c[4];
    // GLOBAL: CMR2 0x0066461c
    static int m_unk0x0066461c;
    // GLOBAL: CMR2 0x00664620
    static int m_unk0x00664620;
};

#endif
