#include "Game.h"

// Defined apart from the code that reads it: the original reads this byte
// with byte loads (Game.cpp and GameNetwork.cpp only see the declaration),
// where a definition in the same object lets MSVC6 use an aligned dword load.
BYTE CGame::m_connectionCount;
