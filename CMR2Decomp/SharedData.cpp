#include <windows.h>

// Globals defined apart from the code that reads them. The original reads
// these bytes with byte loads, as code that only sees an extern declaration
// does; a BYTE defined in the same object is known to be aligned and MSVC6
// reads it with a dword load.

// GLOBAL: CMR2 0x00591944
BYTE g_collisionNegativeCandidateCount;

// GLOBAL: CMR2 0x00591945
BYTE g_collisionPositiveCandidateCount;
