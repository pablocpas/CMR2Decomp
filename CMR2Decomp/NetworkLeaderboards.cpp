#include "NetworkLeaderboards.h"
#include "RegKey.h"
#include "InstallInfo.h"
#include "FileBuffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"

int CNetworkLeaderboards::m_leaderboardId = -1;
int CNetworkLeaderboards::m_totalLeaderboards;
NetworkLeaderboard CNetworkLeaderboards::m_leaderboards[32];
char CNetworkLeaderboards::m_strNetworkLeaderboardsDir[40] = "%s\\NetworkLeaderboards\\leaderboards.nlb";

// FUNCTION: CMR2 0x0040e3c0
void CNetworkLeaderboards::Reset() {
    m_leaderboardId = -1;
    m_totalLeaderboards = 0;

    int addr = (int)&m_leaderboards;
    int endAddr = addr + sizeof(m_leaderboards);
    
    while (addr < endAddr) {
        *(BYTE*)addr = 0;
        addr += 0x104;
    }
}

// FUNCTION: CMR2 0x0040e3f0
bool CNetworkLeaderboards::LoadLeaderboards(void) {
    char fileLocation[MAX_PATH];
    GenericFile * fileBuffer;
    bool hasLoaded = false;
    NetworkLeaderboardsFile* leaderboards;

    sprintf(fileLocation, m_strNetworkLeaderboardsDir, CRegKey::GetValueFromKey(CRegKey::m_rkv_gameHDPath));
    leaderboards = (NetworkLeaderboardsFile*)CFileBuffer::GetGenericFileBuffer(fileLocation, TRUE);
    if (leaderboards != NULL) {
        if (CGenericFileLoader::GetGenericFileSize() == 0x2088) {
            m_leaderboardId = leaderboards->leaderboardID;
            m_totalLeaderboards = leaderboards->totalLeaderboards;
            
            memcpy(&m_leaderboards, leaderboards->leaderboards, sizeof(leaderboards->leaderboards));
            hasLoaded = TRUE;
            
        }

        CFileBuffer::FreeGenericFileBuffer(leaderboards);
    }

    return hasLoaded;
}

// Writes the leaderboards back to NetworkLeaderboards\leaderboards.nlb.
// FUNCTION: CMR2 0x0040e470
void CNetworkLeaderboards::SaveLeaderboards(void)
{
    NetworkLeaderboardsFile *pFile;
    char fileLocation[MAX_PATH];

    pFile = (NetworkLeaderboardsFile *)CFileBuffer::AllocateLockedBuffer(0x2088);
    if (pFile != NULL) {
        pFile->leaderboardID = m_leaderboardId;
        pFile->totalLeaderboards = m_totalLeaderboards;
        memcpy(pFile->leaderboards, m_leaderboards, sizeof(pFile->leaderboards));
        sprintf(fileLocation, m_strNetworkLeaderboardsDir, CRegKey::GetValueFromKey(CRegKey::m_rkv_gameHDPath));
        CInstallInfo::WriteFileToDisk(fileLocation, 0, pFile, 0x2088);
        CFileBuffer::FreeGenericFileBuffer(pFile);
    }
}

// FUNCTION: CMR2 0x0040e4f0
int CNetworkLeaderboards::GetLeaderboardId(void)
{
    return m_leaderboardId;
}

// FUNCTION: CMR2 0x0040e500
void CNetworkLeaderboards::SetLeaderboardId(int id)
{
    m_leaderboardId = id;
}

// FUNCTION: CMR2 0x0040e540
int CNetworkLeaderboards::GetTotalLeaderboards(void)
{
    return m_totalLeaderboards;
}

// FUNCTION: CMR2 0x0040e510
NetworkLeaderboard *CNetworkLeaderboards::GetLoadedLeaderboard(int index)
{
    NetworkLeaderboard *p = &m_leaderboards[index];
    return *(BYTE *)p ? p : NULL;
}

void FUN_0040e8a0(BYTE *p);

// Leaderboard order: most wins first, then names in reverse order.
// TODO: CMR2 0x0040e790 (implemented, match 93%)
int __cdecl FUN_0040e790(const void *a, const void *b)
{
    const NetworkLeaderboardEntry *e1 = (const NetworkLeaderboardEntry *)a;
    const NetworkLeaderboardEntry *e2 = (const NetworkLeaderboardEntry *)b;

    if (e1->wins > e2->wins)
        return -1;
    if (e1->wins < e2->wins)
        return 1;
    if (strcmp(e1->name, e2->name) > 0)
        return -1;
    return strcmp(e1->name, e2->name) >= 0 ? -1 : 1;
}

// FUNCTION: CMR2 0x0040e850
void FUN_0040e850(int index)
{
    qsort(CNetworkLeaderboards::m_leaderboards[index].entries, 0x20, sizeof(NetworkLeaderboardEntry), FUN_0040e790);
    FUN_0040e8a0((BYTE *)&CNetworkLeaderboards::m_leaderboards[index]);
}

// Adds wins to the entry with the given name, creating it in the first
// empty slot when it is not listed yet, and re-sorts the leaderboard.
// TODO: CMR2 0x0040e660 (implemented, match 97%)
void FUN_0040e660(int index, char *name, int wins)
{
    int i;

    for (i = 0; i < 0x20; i++) {
        if (strcmp(CNetworkLeaderboards::m_leaderboards[index].entries[i].name, name) == 0) {
            CNetworkLeaderboards::m_leaderboards[index].entries[i].wins += wins;
            FUN_0040e850(index);
            return;
        }
    }
    for (i = 0; i < 0x20; i++) {
        if (strcmp(CNetworkLeaderboards::m_leaderboards[index].entries[i].name, CMain::m_logFileBlankLine) == 0)
            break;
    }
    if (i < 0x20) {
        strcpy(CNetworkLeaderboards::m_leaderboards[index].entries[i].name, name);
        CNetworkLeaderboards::m_leaderboards[index].entries[i].wins = wins;
    }
    FUN_0040e850(index);
}

// GLOBAL: CMR2 0x00533900
BYTE g_unk0x00533900[0x104];
// GLOBAL: CMR2 0x00535b8c
BYTE g_unk0x00535b8c;

// FUNCTION: CMR2 0x0040e890
void FUN_0040e890(void)
{
    g_unk0x00535b8c = 0;
}

// FUNCTION: CMR2 0x0040e8a0
void FUN_0040e8a0(BYTE *p)
{
    memcpy(g_unk0x00533900, p, sizeof(g_unk0x00533900));
    g_unk0x00535b8c = 1;
}

// FUNCTION: CMR2 0x0040e8c0
BYTE *FUN_0040e8c0(void)
{
    return g_unk0x00535b8c ? g_unk0x00533900 : NULL;
}
