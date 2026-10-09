#include "Game.h"

// FUNCTION: CMR2 0x004aacf0
unsigned int CGame::GetConnectionCount(void)
{
    unsigned int count = 0;

    count = m_connectionCount;
    return count;
}
