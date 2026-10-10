#ifndef _LOGGER_H_
#define _LOGGER_H


class CLogger
{
private:
    // GLOBAL: CMR2 0x00667200
    static BOOL bIsLogFileOpen;
    // GLOBAL: CMR2 0x00667204
    static char logFileName[320];
    // GLOBAL: CMR2 0x00667348
    static unsigned int loggingDisabled;
    // GLOBAL: CMR2 0x0066734c
    static struct SysFile *hLogFileHandle;

public:
    static void OpenLogFile(LPCSTR file);
    static void LogToFile(LPCSTR str);
    static void CloseLogFile(void);
};

#endif
