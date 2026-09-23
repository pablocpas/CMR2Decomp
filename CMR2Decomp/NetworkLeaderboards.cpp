#include "NetworkLeaderboards.h"
#include "RegKey.h"
#include "InstallInfo.h"
#include "FileBuffer.h"
#include <stdio.h>
#include <string.h>

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
