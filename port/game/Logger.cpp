#include "port/sys.h"
#include "Logger.h"

// GLOBAL: CMR2 0x00667344
unsigned int unk0x00667344;

BOOL CLogger::bIsLogFileOpen;
char CLogger::unk0x00667204[320];
unsigned int CLogger::unk0x00667348;
SysFile *CLogger::hLogFileHandle;

// FUNCTION: CMR2 0x004ab620
// PORT: the file goes through Sys (c:\\error.txt lands in the user directory).
void CLogger::OpenLogFile(LPCSTR file)
{
    strcpy(unk0x00667204, file);
    unk0x00667344 = 0;
    hLogFileHandle = Sys_OpenFile(file, SYS_FILE_WRITE);
    if (hLogFileHandle != NULL)
        bIsLogFileOpen = TRUE;
}

// FUNCTION: CMR2 0x004ab670
void CLogger::LogToFile(LPCSTR str)
{
    const char n = 0xd;
    const char r = 0xa;

    if (unk0x00667348 == 0 && bIsLogFileOpen != FALSE)
    {
        Sys_WriteFile(hLogFileHandle, str, strlen(str));
        Sys_WriteFile(hLogFileHandle, &n, 1);
        Sys_WriteFile(hLogFileHandle, &r, 1);
        Sys_FlushFile(hLogFileHandle);
    }
}

// FUNCTION: CMR2 0x004ab6f0
void CLogger::CloseLogFile(void)
{
    Sys_CloseFile(hLogFileHandle);
    hLogFileHandle = NULL;
    bIsLogFileOpen = FALSE;
}
